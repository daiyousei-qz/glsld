#pragma once

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

    class LexString
    {
    public:
        LexString() = default;
        explicit LexString(const char* p) : ptr(p)
        {
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
        TokenKlass klass  = TokenKlass::Eof;
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