#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Preprocessing")
{
    SetTestTemplate("{}");

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
    }
}