#include "AstTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Global Decl")
{
    SetTestTemplate("{}", [this](AstMatcher* matcher) { return TranslationUnit({matcher}); });

    GLSLD_CHECK_AST(";", EmptyDecl());
    // GLSLD_CHECK_AST("int a;", VariableDecl1(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), nullptr));
    GLSLD_CHECK_AST("int a = 1;",
                    VariableDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), NullAst(), LiteralExpr(1)));
}

TEST_CASE_METHOD(AstTestFixture, "Simple Local Decl")
{
    SetTestTemplate("void main() {{ {} }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({DeclStmt(matcher)})));
    });
}