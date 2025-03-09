#include "Compiler/SyntaxToken.h"
#include "LexingTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(LexingTestFixture, "Lexing")
{
    SECTION("Keyword")
    {
#define DECL_KEYWORD(KEYWORD)                                                                                          \
    GLSLD_CHECK_TOKENS(#KEYWORD, TokenMatcher{"Keyword", TokenKlass::K_##KEYWORD, std::nullopt}, EofTok());
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
    }

    SECTION("Punctuation")
    {
#define DECL_PUNCT(PUNCT_NAME, PUNCT)                                                                                  \
    GLSLD_CHECK_TOKENS(PUNCT, TokenMatcher{"Punctuation", TokenKlass::PUNCT_NAME, std::nullopt}, EofTok());
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
    }

    SECTION("Integer constant")
    {
        const char* tests[] = {
            "0", "1", "123", "1234567890", "0x123abc", "0X123ABC", "0123", "0u", "123U", "0xabcu", "0XABcU", "0123u",
        };

        for (StringView test : tests) {
            GLSLD_CHECK_TOKENS(test, IntTok(test), EofTok());
        }
    }

    SECTION("Float constant")
    {
        const char* tests[] = {
            "0.0",     "1.0",      "123.0",    "1234567890.0", "0.123",        "0.123e1",
            "0.123E1", "0.123e+1", "0.123e-1", "0.123e+00001", "0.123e-00001",
        };

        for (StringView test : tests) {
            GLSLD_CHECK_TOKENS(test, FloatTok(test), EofTok());
        }
    }

    SECTION("Identifier")
    {
        const char* tests[] = {
            "abc",
            "ABC",
            "a0_",
            "___",
        };

        for (StringView test : tests) {
            GLSLD_CHECK_TOKENS(test, IdTok(test), EofTok());
        }
    }
}
