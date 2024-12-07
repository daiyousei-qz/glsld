#include "Basic/StringView.h"
#include "Compiler/SyntaxToken.h"
#include "LexingTestFixture.h"
#include <catch2/catch_all.hpp>

using namespace glsld;

TEST_CASE("Lex single token", "")
{
    SECTION("Keyword")
    {
#define DECL_KEYWORD(KEYWORD) REQUIRE(LexSingleTokenTest<TokenKlass::K_##KEYWORD>{#KEYWORD}.Positive());
#include "GlslKeywords.inc"
#undef DECL_KEYWORD
    }

    SECTION("Punctuation")
    {
#define DECL_PUNCT(PUNCT_NAME, PUNCT) REQUIRE(LexSingleTokenTest<TokenKlass::PUNCT_NAME>{PUNCT}.Positive());
#include "GlslPunctuation.inc"
#undef DECL_PUNCT
    }

    SECTION("Integer constant")
    {
        using Test = LexSingleTokenTest<TokenKlass::IntegerConstant>;
        REQUIRE(Test{"0"}.Positive());
        REQUIRE(Test{"1"}.Positive());
        REQUIRE(Test{"123"}.Positive());
        REQUIRE(Test{"1234567890"}.Positive());
        REQUIRE(Test{"0x123abc"}.Positive());
        REQUIRE(Test{"0X123ABC"}.Positive());
        REQUIRE(Test{"0123"}.Positive());
        REQUIRE(Test{"0u"}.Positive());
        REQUIRE(Test{"123U"}.Positive());
        REQUIRE(Test{"0xabcu"}.Positive());
        REQUIRE(Test{"0XABcU"}.Positive());
        REQUIRE(Test{"0123u"}.Positive());
    }

    SECTION("Float constant")
    {
        using Test = LexSingleTokenTest<TokenKlass::FloatConstant>;
        REQUIRE(Test{"0.0"}.Positive());
        REQUIRE(Test{"1.0"}.Positive());
        REQUIRE(Test{"123.0"}.Positive());
        REQUIRE(Test{"1234567890.0"}.Positive());
        REQUIRE(Test{"0.123"}.Positive());
        REQUIRE(Test{"0.123e1"}.Positive());
        REQUIRE(Test{"0.123E1"}.Positive());
        REQUIRE(Test{"0.123e+1"}.Positive());
        REQUIRE(Test{"0.123e-1"}.Positive());
        REQUIRE(Test{"0.123e+00001"}.Positive());
        REQUIRE(Test{"0.123e-00001"}.Positive());
    }

    SECTION("Identifier")
    {
        using Test = LexSingleTokenTest<TokenKlass::Identifier>;
        REQUIRE(Test{"abc"}.Positive());
        REQUIRE(Test{"ABC"}.Positive());
        REQUIRE(Test{"a0_"}.Positive());
        REQUIRE(Test{"___"}.Positive());
        REQUIRE(Test{"测试"}.Negative());
        REQUIRE(Test{"テスト"}.Negative());
    }
}
