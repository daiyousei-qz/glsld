#pragma once

#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Basic/AtomTable.h"
#include "Basic/SourceInfo.h"
#include "Language/Typing.h"

#include <optional>

namespace glsld
{
    enum class TokenKlass
    {
        Invalid,
        Unknown,
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
        // NOTE if the last line is terminated by a line continuation, this flag is always set to false.
        bool isFirstTokenOfLine;

        // If the token has leading whitespace that's separating it from the previous token, this flag is set to true.
        bool hasLeadingWhitespace;
    };

    using SyntaxTokenIndex                              = uint32_t;
    inline constexpr SyntaxTokenIndex InvalidTokenIndex = static_cast<uint32_t>(-1);

    struct AstSyntaxRange final
    {
        // The range that spells the first token for this AST node.
        SyntaxTokenIndex startTokenIndex = 0;

        // The range that spells the first token after this AST node.
        SyntaxTokenIndex endTokenIndex = 0;

        AstSyntaxRange() = default;
        AstSyntaxRange(SyntaxTokenIndex tokIndex) : startTokenIndex(tokIndex), endTokenIndex(tokIndex + 1)
        {
        }
        AstSyntaxRange(SyntaxTokenIndex beginTokIndex, SyntaxTokenIndex endTokIndex)
            : startTokenIndex(beginTokIndex), endTokenIndex(endTokIndex)
        {
            GLSLD_ASSERT(beginTokIndex <= endTokIndex);
        }
    };

    // A syntax token that is part of the source token stream.
    // It is pointing to a raw token that is managed by the LexContext.
    struct SyntaxToken final
    {
        SyntaxTokenIndex index = InvalidTokenIndex;
        TokenKlass klass       = TokenKlass::Invalid;
        AtomString text        = {};

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
            return AstSyntaxRange(index, index + 1);
        }
    };

    struct RawSyntaxTokenEntry final
    {
        // The token class.
        TokenKlass klass;

        // The source file id that the token is spelled.
        FileID spelledFile;

        // The text range in which the token is spelled. If the token isn't directly spelled but created by macro
        // expansion, this is an empty range.
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

} // namespace glsld
