#pragma once

#include "Common.h"
#include "SourceView.h"
#include "SourceContext.h"

#include <functional>
#include <string>
#include <vector>

namespace glsld
{
    enum class TokenKlass
    {
        Error,
        Eof,
        // Line and block comments
        Comment,
        // #
        Hash,
        // ##
        HashHash,
        // <xxx>
        AngleString,
        // "xxx"
        QuotedString,
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
        case TokenKlass::Error:
            return "Error";
        case TokenKlass::Eof:
            return "Eof";
        case TokenKlass::Comment:
            return "Comment";
        case TokenKlass::Hash:
            return "#";
        case TokenKlass::HashHash:
            return "##";
        case TokenKlass::AngleString:
            return "AngleString";
        case TokenKlass::QuotedString:
            return "QuotedString";
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

    inline auto GetAllKeywords() -> ArrayView<std::pair<TokenKlass, std::string_view>>
    {
        static constexpr std::pair<TokenKlass, std::string_view> allKeywords[] = {
#define DECL_KEYWORD(KEYWORD) {TokenKlass::K_##KEYWORD, #KEYWORD},
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
        };

        return allKeywords;
    }

    inline auto GetAllPunctuations() -> ArrayView<std::pair<TokenKlass, std::string_view>>
    {
        static constexpr std::pair<TokenKlass, std::string_view> allPuncts[] = {
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

    inline auto IsIdentifierLikeToken(TokenKlass klass) -> bool
    {
        return klass == TokenKlass::Identifier || IsKeywordToken(klass);
    }

    class LexString
    {
    public:
        LexString() = default;
        explicit LexString(const char* p) : ptr(p)
        {
        }

        auto Get() const noexcept -> const char*
        {
            return ptr;
        }

        auto Equals(const char* other) const noexcept -> bool
        {
            return ptr ? strcmp(ptr, other) == 0 : false;
        }

        auto Str() const noexcept -> std::string
        {
            return ptr ? ptr : "";
        }
        auto StrView() const noexcept -> StringView
        {
            return ptr ? ptr : "";
        }

        operator const char*() noexcept
        {
            return ptr;
        }

    private:
        // A pointer of C-style string that's hosted by the LexContext
        const char* ptr = nullptr;
    };

    inline auto operator==(const LexString& lhs, const char* rhs) noexcept -> bool
    {
        return lhs.Equals(rhs);
    }
    inline auto operator==(const char* lhs, const LexString& rhs) noexcept -> bool
    {
        return rhs.Equals(lhs);
    }

    // NOTE we assume two LexString are managed by the same LexContext.
    // It is illegal to compare two LexString from different contexts.
    inline auto operator==(const LexString& lhs, const LexString& rhs) noexcept -> bool
    {
        return lhs.Get() == rhs.Get();
    }

    using SyntaxTokenIndex                              = uint32_t;
    inline constexpr SyntaxTokenIndex InvalidTokenIndex = static_cast<uint32_t>(-1);

    struct SyntaxTokenRange
    {
        SyntaxTokenIndex startTokenIndex = 0;
        SyntaxTokenIndex endTokenIndex   = 0;

        SyntaxTokenRange() = default;
        SyntaxTokenRange(SyntaxTokenIndex tokIndex) : startTokenIndex(tokIndex), endTokenIndex(tokIndex + 1)
        {
        }
        SyntaxTokenRange(SyntaxTokenIndex beginTokIndex, SyntaxTokenIndex endTokIndex)
            : startTokenIndex(beginTokIndex), endTokenIndex(endTokIndex)
        {
            GLSLD_ASSERT(beginTokIndex <= endTokIndex);
        }
    };

    // A syntax token that is part of the source token stream.
    // It is pointing to a raw token which is managed by the LexContext.
    struct SyntaxToken
    {
        SyntaxTokenIndex index = InvalidTokenIndex;
        TokenKlass klass       = TokenKlass::Error;
        LexString text         = {};

        auto IsValid() const -> bool
        {
            return index != InvalidTokenIndex;
        }
        auto IsError() const -> bool
        {
            return klass == TokenKlass::Error;
        }
        auto IsIdentifier() const -> bool
        {
            return klass == TokenKlass::Identifier;
        }
        auto IsKeyword() const -> bool
        {
            return IsKeywordToken(klass);
        }
    };

    struct RawSyntaxToken final
    {
        // File id of which the token is from
        FileID file;

        // The token class
        TokenKlass klass;

        // The token text
        LexString text;

        // The token range in the source file
        TextRange range;
    };

    struct PPTokenData final
    {
        // The kind of the token. Note in preprocessing stage, all keywords are treated as identifiers.
        TokenKlass klass;

        // If the token is the first token of a line, this flag is set to true.
        // NOTE if the last line is terminated by a line continuation, this flag is always set to false.
        bool isFirstTokenOfLine;

        // If the token has leading whitespace that's separating it from the previous token, this flag is set to true.
        bool hasLeadingWhitespace;

        // The text of the token.
        LexString text;

        // The spelling range of the token in the source.
        TextRange range;
    };

} // namespace glsld

namespace std
{
    template <>
    struct hash<glsld::LexString>
    {
        [[nodiscard]] auto operator()(const glsld::LexString& key) const noexcept -> size_t
        {
            return hash<glsld::StringView>{}(key.StrView());
        }
    };
} // namespace std
