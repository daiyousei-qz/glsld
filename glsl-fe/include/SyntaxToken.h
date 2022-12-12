#pragma once

#include "Common.h"
#include "SourceInfo.h"
#include <string>

namespace glsld
{
    enum class TokenKlass
    {
        Error,
        Eof,
        Comment,
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
        default:
            GLSLD_UNREACHABLE();
        }

#undef STRINGIZE
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

    class LexString
    {
    public:
        LexString() = default;
        explicit LexString(const char* p) : ptr(p)
        {
        }

        auto Get() const -> const char*
        {
            return ptr;
        }

        auto Str() const -> std::string
        {
            return ptr ? ptr : "";
        }
        auto StrView() const -> std::string_view
        {
            return ptr ? ptr : "";
        }

        operator const char*()
        {
            return ptr;
        }
        auto operator==(const LexString& other) -> bool
        {
            return ptr == other.ptr;
        }

    private:
        // A pointer of C-style string that's hosted by the LexContext
        const char* ptr = nullptr;
    };

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

    struct SyntaxTokenInfo
    {
        int file;
        TokenKlass klass;
        LexString text;
        TextRange range;
    };

} // namespace glsld