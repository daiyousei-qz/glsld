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

    SECTION("Number literal")
    {
        const char* tests[] = {
            // Integer literals
            "0",
            "1",
            "123",
            "1234567890",
            "0x123abc",
            "0X123ABC",
            "0123",
            "0u",
            "123U",
            "0xabcu",
            "0XABcU",
            "0123u",
            "42u",
            "42U",
            // Float literals
            "0.0",
            "1.0",
            "123.0",
            "1234567890.0",
            "0.123",
            ".0",
            ".5",
            "1.",
            "1e1",
            "1E+1",
            "1E-1",
            "0.123e1",
            "0.123E1",
            "0.123e+1",
            "0.123e-1",
            "0.123e+00001",
            "0.123e-00001",
            "1.0f",
            "1.0F",
            "1e1f",
            "1E+1F",
            "1.0lf",
            "1.0LF",
            "1e1lf",
            "1E-1LF",
            // Permissive invalid literals that are still lexed as number literals
            "09",     // Invalid octal literal with non-octal digit
            "0x",     // Invalid hex literal with no digits
            "0x123g", // Invalid hex literal with non-hex digit
            "123abc", // Invalid decimal literal with non-digit suffix
            "0.123e", // Invalid float literal with no exponent digits
        };

        for (SourceTextView test : tests) {
            CheckTokens(test, {NumTok(test), EofTok()});
        }
    }

    SECTION("Identifier")
    {
        const char* tests[] = {
            // Valid identifiers
            "abc",
            "ABC",
            "a0_",
            "_member",
            "___",
            // Keyword prefix remains identifier
            "for_",
            "if0",
            "layout123",
            "structName",
            "whileLoop",
        };

        for (SourceTextView test : tests) {
            CheckTokens(test, {IdTok(test), EofTok()});
        }
    }

    SECTION("Mixed token stream")
    {
        CheckTokens("foo+=bar<<2u;",
                    {IdTok("foo"), KeywordTok(TokenKlass::AddAssign), IdTok("bar"), KeywordTok(TokenKlass::LShift),
                     NumTok("2u"), KeywordTok(TokenKlass::Semicolon), EofTok()});

        CheckTokens("vec4(color.rgb, 1.0)",
                    {KeywordTok(TokenKlass::K_vec4), KeywordTok(TokenKlass::LParen), IdTok("color"),
                     KeywordTok(TokenKlass::Dot), IdTok("rgb"), KeywordTok(TokenKlass::Comma), NumTok("1.0"),
                     KeywordTok(TokenKlass::RParen), EofTok()});
    }

    SECTION("Line continuation")
    {
        CheckTokens("ab\\\ncd 12\\\n34", {IdTok("abcd"), NumTok("1234"), EofTok()});
        CheckTokens("foo\\\n_bar", {IdTok("foo_bar"), EofTok()});
    }

    SECTION("Comments")
    {
        const SourceTextView sourceText = "foo/*block*/+//line\nbar";

        CheckTokens(sourceText, {IdTok("foo"), KeywordTok(TokenKlass::Plus), IdTok("bar"), EofTok()});

        auto compilerResult = Compile(sourceText, CompileMode::PreprocessOnly);
        auto comments       = compilerResult->GetUserFileArtifacts().GetComments();

        REQUIRE(comments.size() == 2);
        CHECK(comments[0].text.StrView() == "/*block*/");
        CHECK(comments[0].frontAttachmentLine == 0);
        CHECK(comments[0].backAttachmentLine == 0);
        CHECK(comments[0].nextTokenIndex == 1);

        CHECK(comments[1].text.StrView() == "//line\n");
        CHECK(comments[1].frontAttachmentLine == 0);
        CHECK(comments[1].backAttachmentLine == 1);
        CHECK(comments[1].nextTokenIndex == 2);
    }

    SECTION("Unknown character")
    {
        CheckTokens("@", {UnknownTok("@"), EofTok()});
        CheckTokens("啊", {UnknownTok("啊"), EofTok()});
    }
}
