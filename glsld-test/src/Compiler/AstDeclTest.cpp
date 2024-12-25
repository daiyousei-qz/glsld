#include "AstTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Global Decl")
{
    SetTestTemplate("{}", [](AstMatcher matcher) { return TranslationUnit(std::move(matcher)); });

    GLSLD_CHECK_AST(";", EmptyDecl());
    // GLSLD_CHECK_AST("int a;", VariableDecl1(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), nullptr));
    GLSLD_CHECK_AST("int a = 1;", VariableDecl1(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), LiteralExpr(1)));
}

TEST_CASE_METHOD(AstTestFixture, "Simple Local Decl")
{
    SetTestTemplate("void main() {{ {} }}", [](AstMatcher matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), AnyStmt()),
                         FunctionDecl(AnyAst(), AnyTok(), CompoundStmt(DeclStmt(std::move(matcher)))));
    });
}