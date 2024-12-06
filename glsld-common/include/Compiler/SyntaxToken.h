#pragma once

#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Language/Typing.h"

#include <compare>
#include <cstdint>
#include <optional>

namespace glsld
{
    enum class TokenKlass
    {
        Invalid,
        Unknown,
        // End of main file
        Eof,
        // Line and block comments
        Comment,
        // Preprocessing: #
        Hash,
        // Preprocessing: ##
        HashHash,
        // Preprocessing: <xxx>
        SystemHeaderName,
        // Preprocessing: "xxx"
        UserHeaderName,
        // Preprocessing: produced by `defined(X)` where X is a defined macro.
        DefinedYes,
        // Preprocessing: produced by `defined(X)` where X is not a defined macro.
        DefinedNo,

        IntegerConstant,
        FloatConstant,
        Identifier,

#define DECL_KEYWORD(KEYWORD) K_##KEYWORD,
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

#define DECL_PUNCT(PUNCT_NAME, ...) PUNCT_NAME,
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
    };

    inline auto TokenKlassToString(TokenKlass klass) -> std::string_view
    {
#define STRINGIZE(X) #X

        switch (klass) {
        case TokenKlass::Invalid:
            return "Invalid";
        case TokenKlass::Unknown:
            return "Unknown";
        case TokenKlass::Eof:
            return "Eof";
        case TokenKlass::Comment:
            return "Comment";
        case TokenKlass::Hash:
            return "#";
        case TokenKlass::HashHash:
            return "##";
        case TokenKlass::SystemHeaderName:
            return "SystemHeaderName";
        case TokenKlass::UserHeaderName:
            return "UserHeaderName";
        case TokenKlass::DefinedYes:
            return "DefinedYes";
        case TokenKlass::DefinedNo:
            return "DefinedNo";
        case TokenKlass::IntegerConstant:
            return "IntegerConstant";
        case TokenKlass::FloatConstant:
            return "FloatConstant";
        case TokenKlass::Identifier:
            return "Identifier";

#define DECL_KEYWORD(KEYWORD)                                                                                          \
    case TokenKlass::K_##KEYWORD:                                                                                      \
        return STRINGIZE(K_##KEYWORD);
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

#define DECL_PUNCT(PUNCT_NAME, ...)                                                                                    \
    case TokenKlass::PUNCT_NAME:                                                                                       \
        return STRINGIZE(PUNCT_NAME);
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
        }

#undef STRINGIZE
        GLSLD_UNREACHABLE();
    }

    inline auto GetAllKeywords() -> ArrayView<std::pair<TokenKlass, StringView>>
    {
        static constexpr std::pair<TokenKlass, StringView> allKeywords[] = {
#define DECL_KEYWORD(KEYWORD) {TokenKlass::K_##KEYWORD, #KEYWORD},
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
        };

        return allKeywords;
    }

    inline auto GetAllPunctuations() -> ArrayView<std::pair<TokenKlass, StringView>>
    {
        static constexpr std::pair<TokenKlass, StringView> allPuncts[] = {
#define DECL_PUNCT(PUNCT_NAME, PUNCT) {TokenKlass::PUNCT_NAME, PUNCT},
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
        };

        return allPuncts;
    }

    inline auto IsKeywordToken(TokenKlass klass) -> bool
    {
        switch (klass) {
#define DECL_KEYWORD(KEYWORD)                                                                                          \
    case TokenKlass::K_##KEYWORD:                                                                                      \
        return true;
#include "GlslKeywords.inc"
#undef DECL_KEYWORD

        default:
            return false;
        }
    }

    inline auto GetGlslBuiltinType(TokenKlass klass) -> std::optional<GlslBuiltinType>
    {
        switch (klass) {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, ...)                                                                              \
    case TokenKlass::K_##GLSL_TYPE:                                                                                    \
        return GlslBuiltinType::Ty_##GLSL_TYPE;
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
        default:
            return std::nullopt;
        }
    }

    inline auto IsIdentifierLikeToken(TokenKlass klass) -> bool
    {
        return klass == TokenKlass::Identifier || IsKeywordToken(klass);
    }

    struct PPToken final
    {
        // The kind of the token. Note in preprocessing stage, all keywords are treated as identifiers.
        TokenKlass klass;

        // The source file in which the token is spelled.
        FileID spelledFile;

        // The text range where the token is actually spelled.
        TextRange spelledRange;

        // The text of the token.
        AtomString text;

        // If the token is the first token of a line, this flag is set to true.
        // NOTE if the last line is terminated by a line continuation, this flag should be set to false.
        bool isFirstTokenOfLine;

        // If the token has leading whitespace that's separating it from the previous token, this flag is set to true.
        bool hasLeadingWhitespace;
    };

    enum class TranslationUnitID
    {
        SystemPreamble = 0,
        UserPreamble   = 1,
        UserFile       = 2,
    };

    class SyntaxTokenID
    {
    private:
        static constexpr uint32_t InvalidTokenID = static_cast<uint32_t>(-1);
        // this must be a power of 2 for best performance
        static constexpr uint32_t MaxTranslationUnitTokenCount = 1u << 28;
        static constexpr uint32_t MaxTranslationUnitCount      = 15;

        uint32_t value;

    public:
        constexpr SyntaxTokenID()
        {
        }
        constexpr explicit SyntaxTokenID(uint32_t tuIndex, uint32_t tokIndex)
        {
            GLSLD_ASSERT(tuIndex < MaxTranslationUnitCount && tokIndex < MaxTranslationUnitTokenCount);
            value = tuIndex * MaxTranslationUnitTokenCount + tokIndex;
        }

        auto IsValid() const noexcept -> bool
        {
            return value != InvalidTokenID;
        }

        auto GetTUIndex() const noexcept -> uint32_t
        {
            return value / MaxTranslationUnitTokenCount;
        }

        auto GetTokenIndex() const noexcept -> uint32_t
        {
            return value % MaxTranslationUnitTokenCount;
        }

        auto operator==(const SyntaxTokenID& other) const noexcept -> bool = default;

        auto operator++() noexcept -> SyntaxTokenID&
        {
            ++value;
            return *this;
        }
        auto operator--() noexcept -> SyntaxTokenID&
        {
            --value;
            return *this;
        }
        auto operator+=(uint32_t diff) noexcept -> SyntaxTokenID&
        {
            value += diff;
            return *this;
        }
        auto operator-=(uint32_t diff) noexcept -> SyntaxTokenID&
        {
            value -= diff;
            return *this;
        }
        auto operator+(uint32_t diff) const noexcept -> SyntaxTokenID
        {
            SyntaxTokenID result = *this;
            result += diff;
            return result;
        }
        auto operator-(uint32_t diff) const noexcept -> SyntaxTokenID
        {
            SyntaxTokenID result = *this;
            result += diff;
            return result;
        }
        auto operator-(SyntaxTokenID other) const noexcept -> int32_t
        {
            uint32_t diff = std::max(value, other.value) - std::min(value, other.value);
            int32_t sign  = value > other.value ? 1 : -1;
            return sign * static_cast<int32_t>(diff);
        }
    };

    class AstSyntaxRange final
    {
    private:
        // The range that spells the first token for this AST node.
        SyntaxTokenID beginID = {};

        // The range that spells the first token after this AST node.
        SyntaxTokenID endID = {};

        class Iterator
        {
        private:
            SyntaxTokenID id;

        public:
            Iterator(SyntaxTokenID id) : id(id)
            {
            }

            auto operator==(const Iterator& other) const noexcept -> bool = default;
            auto operator*() const noexcept -> const SyntaxTokenID&
            {
                return id;
            }
            auto operator->() const noexcept -> const SyntaxTokenID&
            {
                return id;
            }
            auto operator++() noexcept -> Iterator&
            {
                ++id;
                return *this;
            }
            auto operator--() noexcept -> Iterator&
            {
                --id;
                return *this;
            }
        };

    public:
        AstSyntaxRange() = default;
        AstSyntaxRange(SyntaxTokenID tokIndex) : beginID(tokIndex), endID(tokIndex + 1)
        {
        }
        AstSyntaxRange(SyntaxTokenID beginTokID, SyntaxTokenID endTokID) : beginID(beginTokID), endID(endTokID)
        {
            GLSLD_ASSERT(beginTokID.GetTUIndex() == endID.GetTUIndex() &&
                         beginTokID.GetTokenIndex() <= endTokID.GetTokenIndex());
        }

        auto Empty() const noexcept -> bool
        {
            return beginID == endID;
        }

        auto GetTokenCount() const noexcept -> size_t
        {
            return endID - beginID;
        }

        auto GetTranslationUnit() const noexcept -> TranslationUnitID
        {
            return static_cast<TranslationUnitID>(endID.GetTUIndex());
        }

        auto GetBeginID() const noexcept -> SyntaxTokenID
        {
            return beginID;
        }

        auto GetEndID() const noexcept -> SyntaxTokenID
        {
            return endID;
        }

        auto GetBackID() const noexcept -> SyntaxTokenID
        {
            GLSLD_ASSERT(!Empty());
            return endID - 1;
        }

        auto begin() const noexcept -> Iterator
        {
            return Iterator{beginID};
        }
        auto end() const noexcept -> Iterator
        {
            return Iterator{endID};
        }
    };

    // A syntax token that is part of the source token stream.
    // It is pointing to a raw token that is managed by the LexContext.
    struct SyntaxToken final
    {
        SyntaxTokenID index = {};
        TokenKlass klass    = TokenKlass::Invalid;
        AtomString text     = {};

        auto IsValid() const noexcept -> bool
        {
            return klass != TokenKlass::Invalid;
        }
        auto IsUnknown() const noexcept -> bool
        {
            return klass == TokenKlass::Unknown;
        }
        auto IsIdentifier() const noexcept -> bool
        {
            return klass == TokenKlass::Identifier;
        }
        auto IsKeyword() const noexcept -> bool
        {
            return IsKeywordToken(klass);
        }

        auto GetSyntaxRange() const noexcept -> AstSyntaxRange
        {
            return AstSyntaxRange(index);
        }
    };

    struct RawSyntaxTokenEntry final
    {
        // The token class.
        TokenKlass klass;

        // The source file where the token is spelled.
        FileID spelledFile;

        // The text range in which the token is spelled. If the token isn't directly spelled but created by macro
        // expansion, this is an empty range before the macro name token.
        TextRange spelledRange;

        // The text range in which the token expanded into the main file.
        // - If the token is spelled in an included file, this is an empty range right before "#include".
        // - If the token is created from a macro expansion in the main file, this is an empty range before macro name.
        // - Otherwise, this is the same as `spelledRange`.
        // NOTE this is always a range with regard to the main file. Also, this range may not be able to hold the text.
        TextRange expandedRange;

        // The token text after preprocessing.
        AtomString text;
    };

    struct RawCommentTokenEntry final
    {
        // The source file where the token is spelled.
        FileID spelledFile;

        // The text range in which the token is spelled.
        TextRange spelledRange;

        // Comment text including control characters
        AtomString text;

        // Index of the next non-comment token in this translation unit
        uint32_t nextTokenIndex;
    };
} // namespace glsld
