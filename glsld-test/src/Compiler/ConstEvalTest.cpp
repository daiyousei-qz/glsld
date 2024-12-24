#include "AstTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Const Eval")
{
    // const AstNode* captureExpr = nullptr;
    // SetTestTemplate("int x = {}; int y = x;", [](AstMatcher matcher) {
    //     return TranslationUnit(VariableDecl1(AnyQualType(), AnyTok(), AnyInitializer()));
    // });

    // SECTION("Simple")
    // {
    //     GLSLD_CHECK_AST("int x = 1 + 2;", VariableDecl1(BuiltinType(GlslBuiltinType::Ty_int), "x",
    //                                                     BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
    //     GLSLD_CHECK_AST("int x = 1 + 2 * 3;", VariableDecl1(BuiltinType(GlslBuiltinType::Ty_int), "x",
    //                                                         BinaryExpr(BinaryOp::Plus, LiteralExpr(1),
    //                                                                     BinaryExpr(BinaryOp::Mul, LiteralExpr(2),
    //                                                                     LiteralExpr(3))));
    // }
}