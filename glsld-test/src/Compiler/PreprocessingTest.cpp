#include "Compiler/SyntaxToken.h"
#include "LexingTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(LexingTestFixture, "Preprocessing")
{
    SECTION("Macro")
    {
        GLSLD_CHECK_TOKENS("#define MACRO test\nMACRO", TokenMatcher{"Identifier", TokenKlass::Identifier, "test"},
                           EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO test##2\nMACRO", TokenMatcher{"Identifier", TokenKlass::Identifier, "test2"},
                           EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO(A) A\nMACRO(test)",
                           TokenMatcher{"Identifier", TokenKlass::Identifier, "test"}, EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO(A) A##2\nMACRO(test)",
                           TokenMatcher{"Identifier", TokenKlass::Identifier, "test2"}, EofTok());

        GLSLD_CHECK_TOKENS("#define MACRO(A, B) A##B\nMACRO(test, 2)",
                           TokenMatcher{"Identifier", TokenKlass::Identifier, "test2"}, EofTok());

        {
            auto sourceText = R"(
#define MACRO test
#define MACRO2 MACRO
MACRO2
)";
            GLSLD_CHECK_TOKENS(sourceText, TokenMatcher{"Identifier", TokenKlass::Identifier, "test"}, EofTok());
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