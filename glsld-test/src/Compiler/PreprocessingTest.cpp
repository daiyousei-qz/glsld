#include "Compiler/PPEval.h"
#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::PreprocessingTest")
{
    SetTestTemplate("{}");

    SECTION("Version")
    {
        const SourceTextView sourceText = R"(
            #version 450 core
        )";

        struct TestPPCallback : public PPCallback
        {
            auto OnVersionDirective(ArrayView<PPToken> tokens, GlslVersion version, GlslProfile profile)
                -> void override
            {
                CHECK(version == GlslVersion::Ver450);
                CHECK(profile == GlslProfile::Core);
            }
        };

        TestPPCallback testCallback;
        Compile(sourceText, CompileMode::PreprocessOnly, &testCallback);
    }

    SECTION("Extension")
    {
        const SourceTextView sourceText = R"(
            #extension GL_EXT_ray_tracing : enable
        )";

        struct TestPPCallback : public PPCallback
        {
            auto OnExtensionDirective(ArrayView<PPToken> tokens, ExtensionId extension, ExtensionBehavior behavior)
                -> void override
            {
                CHECK(extension == ExtensionId::GL_EXT_ray_tracing);
                CHECK(behavior == ExtensionBehavior::Enable);
            }
        };

        TestPPCallback testCallback;
        Compile(sourceText, CompileMode::PreprocessOnly, &testCallback);
    }

    SECTION("Pragma")
    {
        const SourceTextView sourceText = R"(
            #pragma some_pragma argument1 argument2
        )";

        struct TestPPCallback : public PPCallback
        {
            auto OnUnknownPragmaDirective(ArrayView<PPToken> argTokens) -> void override
            {
                CHECK(argTokens.size() == 3);
                CHECK(argTokens[0].text.StrView() == "some_pragma");
                CHECK(argTokens[1].text.StrView() == "argument1");
                CHECK(argTokens[2].text.StrView() == "argument2");
            }
        };

        TestPPCallback testCallback;
        Compile(sourceText, CompileMode::PreprocessOnly, &testCallback);
    }

    SECTION("Macro")
    {
        CheckTokens("#define MACRO test\nMACRO", {IdTok("test"), EofTok()});

        CheckTokens("#define MACRO test##2\nMACRO", {IdTok("test2"), EofTok()});

        CheckTokens("#define MACRO(A) A\nMACRO(test)", {IdTok("test"), EofTok()});

        CheckTokens("#define MACRO(A) A##2\nMACRO(test)", {IdTok("test2"), EofTok()});

        CheckTokens("#define MACRO(A, B) A##B\nMACRO(test, 2)", {IdTok("test2"), EofTok()});

        {
            const SourceTextView sourceText = R"(
                #define MACRO test
                #define MACRO2 MACRO
                MACRO2
            )";
            CheckTokens(sourceText, {IdTok("test"), EofTok()});
        }

        SECTION("Permissive")
        {
            const SourceTextView sourceText = R"(
                #define FOO(X) X
                FOO()
            )";
            CheckTokens(sourceText, {EofTok()});
        }
    }

    SECTION("Conditional")
    {
        CheckTokens("#if 1\na\n#endif", {IdTok("a"), EofTok()});
        CheckTokens("#if 0\na\n#endif", {EofTok()});
        CheckTokens("#if 1\na\n#elif 1\nb\n#endif", {IdTok("a"), EofTok()});
        CheckTokens("#if 0\na\n#elif 1\nb\n#endif", {IdTok("b"), EofTok()});
        CheckTokens("#if 0\na\n#elif 0\nb\n#endif", {EofTok()});
        CheckTokens("#if 0\na\n#elif 0\nb\n#else\nc\n#endif", {IdTok("c"), EofTok()});

        CheckTokens("#define MACRO 0\n#if MACRO\na\n#else\nb\n#endif", {IdTok("b"), EofTok()});
        CheckTokens("#define MACRO 1\n#if MACRO\na\n#else\nb\n#endif", {IdTok("a"), EofTok()});
        CheckTokens("#if UNDEFINED_MACRO\na\n#else\nb\n#endif", {IdTok("b"), EofTok()});

        CheckTokens(R"(
            #if 0
            #if 1
            a
            #endif
            #endif
        )",
                    {EofTok()});
        CheckTokens(R"(
            #if 0
            #if 1
            a
            #else
            b
            #endif
            #endif
        )",
                    {EofTok()});
    }

    SECTION("PPEval")
    {
        AtomTable atomTable{nullptr};
        auto ppInt = [&atomTable](int value) {
            return PPToken{
                .klass                = TokenKlass::NumberLiteral,
                .spelledFile          = FileID{},
                .spelledRange         = TextRange{},
                .text                 = atomTable.GetAtom(fmt::to_string(value)),
                .isFirstTokenOfLine   = false,
                .hasLeadingWhitespace = false,
            };
        };
        auto ppOp = [&atomTable](TokenKlass klass) {
            return PPToken{
                .klass                = klass,
                .spelledFile          = FileID{},
                .spelledRange         = TextRange{},
                .text                 = atomTable.GetAtom(""),
                .isFirstTokenOfLine   = false,
                .hasLeadingWhitespace = false,
            };
        };

        // Test unary operators
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppOp(TokenKlass::Plus), ppInt(1)}) == 1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppOp(TokenKlass::Dash), ppInt(1)}) == -1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppOp(TokenKlass::Tilde), ppInt(1)}) == ~1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppOp(TokenKlass::Bang), ppInt(1)}) == 0);

        // Test binary operators
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Plus), ppInt(3)}) == 5);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Dash), ppInt(3)}) == -1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Star), ppInt(3)}) == 6);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Slash), ppInt(3)}) == 0);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Percent), ppInt(3)}) == 2);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::LShift), ppInt(3)}) == 16);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::RShift), ppInt(3)}) == 0);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::LAngle), ppInt(3)}) == 1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::LessEq), ppInt(3)}) == 1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::RAngle), ppInt(3)}) == 0);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::GreaterEq), ppInt(3)}) == 0);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Equal), ppInt(3)}) == 0);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::NotEqual), ppInt(3)}) == 1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Ampersand), ppInt(3)}) == 2);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Caret), ppInt(3)}) == 1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::VerticalBar), ppInt(3)}) == 3);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::And), ppInt(3)}) == 1);
        REQUIRE(PPExprEvaluator{}.Evaluate(std::vector{ppInt(2), ppOp(TokenKlass::Or), ppInt(3)}) == 1);

        // Test operator precedence and parentheses
        REQUIRE(PPExprEvaluator{}.Evaluate(
                    std::vector{ppInt(1), ppOp(TokenKlass::Plus), ppInt(1), ppOp(TokenKlass::Plus), ppInt(1)}) == 3);
        REQUIRE(PPExprEvaluator{}.Evaluate(
                    std::vector{ppInt(1), ppOp(TokenKlass::Plus), ppOp(TokenKlass::Dash), ppInt(1)}) == 0);
        REQUIRE(PPExprEvaluator{}.Evaluate(
                    std::vector{ppInt(1), ppOp(TokenKlass::Plus), ppInt(2), ppOp(TokenKlass::Star), ppInt(3)}) == 7);
        REQUIRE(
            PPExprEvaluator{}.Evaluate(std::vector{ppOp(TokenKlass::LParen), ppInt(1), ppOp(TokenKlass::Plus), ppInt(2),
                                                   ppOp(TokenKlass::RParen), ppOp(TokenKlass::Star), ppInt(3)}) == 9);
    }
}