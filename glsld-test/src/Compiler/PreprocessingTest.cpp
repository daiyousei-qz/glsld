#include "Compiler/PPEval.h"
#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Preprocessing")
{
    SetTestTemplate("{}");

    SECTION("Version")
    {
        const auto sourceText = R"(
            #version 450 core
        )";

        struct TestPPCallback : public PPCallback
        {
            auto OnVersionDirective(FileID file, TextRange range, GlslVersion version, GlslProfile profile)
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
        const auto sourceText = R"(
            #extension GL_EXT_ray_tracing : enable
        )";

        struct TestPPCallback : public PPCallback
        {
            auto OnExtensionDirective(FileID file, TextRange range, ExtensionId extension, ExtensionBehavior behavior)
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
        const auto sourceText = R"(
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
        GLSLD_CHECK_TOKENS("#define MACRO test\nMACRO", IdTok("test"), EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO test##2\nMACRO", IdTok("test2"), EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO(A) A\nMACRO(test)", IdTok("test"), EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO(A) A##2\nMACRO(test)", IdTok("test2"), EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO(A, B) A##B\nMACRO(test, 2)", IdTok("test2"), EofTok());

        {
            auto sourceText = R"(
                #define MACRO test
                #define MACRO2 MACRO
                MACRO2
            )";
            GLSLD_CHECK_TOKENS(sourceText, IdTok("test"), EofTok());
        }

        SECTION("Permissive")
        {
            auto sourceText = R"(
                #define FOO(X) X
                FOO()
            )";
            GLSLD_CHECK_TOKENS(sourceText, EofTok());
        }
    }

    SECTION("Conditional")
    {
        GLSLD_CHECK_TOKENS("#if 1\na\n#endif", IdTok("a"), EofTok());
        GLSLD_CHECK_TOKENS("#if 0\na\n#endif", EofTok());
        GLSLD_CHECK_TOKENS("#if 1\na\n#elif 1\nb\n#endif", IdTok("a"), EofTok());
        GLSLD_CHECK_TOKENS("#if 0\na\n#elif 1\nb\n#endif", IdTok("b"), EofTok());
        GLSLD_CHECK_TOKENS("#if 0\na\n#elif 0\nb\n#endif", EofTok());
        GLSLD_CHECK_TOKENS("#if 0\na\n#elif 0\nb\n#else\nc\n#endif", IdTok("c"), EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO 0\n#if MACRO\na\n#else\nb\n#endif", IdTok("b"), EofTok());
        GLSLD_CHECK_TOKENS("#define MACRO 1\n#if MACRO\na\n#else\nb\n#endif", IdTok("a"), EofTok());
        GLSLD_CHECK_TOKENS("#if UNDEFINED_MACRO\na\n#else\nb\n#endif", IdTok("b"), EofTok());

        GLSLD_CHECK_TOKENS(R"(
            #if 0
            #if 1
            a
            #endif
            #endif
        )",
                           EofTok());
        GLSLD_CHECK_TOKENS(R"(
            #if 0
            #if 1
            a
            #else
            b
            #endif
            #endif
        )",
                           EofTok());
    }

    SECTION("PPEval")
    {
        AtomTable atomTable;
        auto ppInt = [&atomTable](int value) {
            return PPToken{
                .klass                = TokenKlass::IntegerConstant,
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