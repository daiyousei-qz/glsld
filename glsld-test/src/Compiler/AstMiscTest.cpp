#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::AstMiscTest")
{
    SetTestTemplate("{}");

    SECTION("Error Recovery")
    {
        CheckAst(R"(
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