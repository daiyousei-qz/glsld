#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Lexing")
{
    SetTestTemplate("{}");

    SECTION("Keyword")
    {
#define DECL_KEYWORD(KEYWORD) CheckTokens(#KEYWORD, {KeywordTok(TokenKlass::K_##KEYWORD), EofTok()});
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
    }

    SECTION("Punctuation")
    {
#define DECL_PUNCT(PUNCT_NAME, PUNCT) CheckTokens(PUNCT, {KeywordTok(TokenKlass::PUNCT_NAME), EofTok()});
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
    }

    SECTION("Integer constant")
    {
        const char* tests[] = {
            "0", "1", "123", "1234567890", "0x123abc", "0X123ABC", "0123", "0u", "123U", "0xabcu", "0XABcU", "0123u",
        };

        for (SourceTextView test : tests) {
            CheckTokens(test, {NumTok(test), EofTok()});
        }
    }

    SECTION("Float constant")
    {
        const char* tests[] = {
            "0.0",     "1.0",      "123.0",    "1234567890.0", "0.123",        "0.123e1",
            "0.123E1", "0.123e+1", "0.123e-1", "0.123e+00001", "0.123e-00001",
        };

        for (SourceTextView test : tests) {
            CheckTokens(test, {NumTok(test), EofTok()});
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

        for (SourceTextView test : tests) {
            CheckTokens(test, {IdTok(test), EofTok()});
        }
    }
}
