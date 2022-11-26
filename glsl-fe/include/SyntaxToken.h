#pragma once

#include "Common.h"
#include <string>

namespace glsld
{
    enum class TokenKlass
    {
        Error,
        Eof,
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
            return ptr;
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

    struct SyntaxLocationInfo
    {
        int file;
        int offset;
        int line;
        int column;
    };

    struct SyntaxLocation
    {
    public:
        SyntaxLocation()
        {
        }
        SyntaxLocation(int index) : index(index)
        {
        }

        int GetIndex()
        {
            return index;
        }

    private:
        int index = -1;
    };

    struct SyntaxRange
    {
        SyntaxLocation begin = -1;
        SyntaxLocation end   = -1;
    };

    struct SyntaxToken
    {
        TokenKlass klass  = TokenKlass::Error;
        LexString text    = {};
        SyntaxRange range = {};
    };

    class SyntaxNode
    {
    public:
        virtual ~SyntaxNode()
        {
        }

        auto GetRange() -> SyntaxRange
        {
            return range;
        }

        auto UpdateRange(SyntaxRange range) -> void
        {
            this->range = range;
        }

    private:
        SyntaxRange range;
    };
} // namespace glsld