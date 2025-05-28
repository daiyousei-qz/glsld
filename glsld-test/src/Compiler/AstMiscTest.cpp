#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "AstMiscTest")
{
    SetTestTemplate("{}");

    SECTION("Error Recovery")
    {
        GLSLD_CHECK_AST(R"(
                void foo((((((
                ;
                void bar();
            )",
                        TranslationUnit({
                            FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"), {}, NullAst()),
                            FunctionDecl(NamedType(TokenKlass::K_void), IdTok("bar"), {}, NullAst()),
                        }));
    }
}