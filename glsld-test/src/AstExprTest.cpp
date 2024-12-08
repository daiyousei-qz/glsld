#include "AstTestFixture.h"

#include <catch2/catch_all.hpp>

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Expr")
{
    SetTemplate("unknown x = {};",
                [](AstMatcher matcher) { return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher))); });

    SECTION("LiteralExpr")
    {
        GLSLD_CHECK_AST("1", LiteralExpr(1));
        GLSLD_CHECK_AST("1.0", LiteralExpr(1.0f));
        GLSLD_CHECK_AST("true", LiteralExpr(true));
    }

    SECTION("NameAccessExpr")
    {
        GLSLD_CHECK_AST("a", NameAccessExpr("a"));
    }

    SECTION("FieldAccessExpr")
    {
        GLSLD_CHECK_AST("a.b", FieldAccessExpr(NameAccessExpr("a"), "b"));
        GLSLD_CHECK_AST("a.b.c", FieldAccessExpr(FieldAccessExpr(NameAccessExpr("a"), "b"), "c"));
    }

    SECTION("UnaryExpr")
    {
        GLSLD_CHECK_AST("-1", UnaryExpr(UnaryOp::Negate, LiteralExpr(1)));
        GLSLD_CHECK_AST("!true", UnaryExpr(UnaryOp::LogicalNot, LiteralExpr(true)));
    }

    SECTION("BinaryExpr")
    {
        GLSLD_CHECK_AST("1 + 2", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
        GLSLD_CHECK_AST(
            "1 + 2 + 3",
            BinaryExpr(BinaryOp::Plus, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3)));
        GLSLD_CHECK_AST("1 + 2 * 3", BinaryExpr(BinaryOp::Plus, LiteralExpr(1),
                                                BinaryExpr(BinaryOp::Mul, LiteralExpr(2), LiteralExpr(3))));
        GLSLD_CHECK_AST(
            "1 * 2 + 3",
            BinaryExpr(BinaryOp::Plus, BinaryExpr(BinaryOp::Mul, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3)));
    }

    SECTION("SelectExpr")
    {
        GLSLD_CHECK_AST("true ? 1 : 2", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2)));
        GLSLD_CHECK_AST("true ? 1 : false ? 2 : 3",
                        SelectExpr(LiteralExpr(true), LiteralExpr(1),
                                   SelectExpr(LiteralExpr(false), LiteralExpr(2), LiteralExpr(3))));
    }

    SECTION("SwizzleExpr")
    {
        SetTemplate("vec4 x; void main() {{ {} }}", [](AstMatcher matcher) {
            return TranslationUnit(AnyDecl(),
                                   FunctionDecl(AnyQualType(), Any(), CompoundStmt(ExprStmt(std::move(matcher)))));
        });

        GLSLD_CHECK_AST("x.x", SwizzleAccessExpr(NameAccessExpr("x"), "x"));
        GLSLD_CHECK_AST("x.xy", SwizzleAccessExpr(NameAccessExpr("x"), "xy"));
        GLSLD_CHECK_AST("x.xyz", SwizzleAccessExpr(NameAccessExpr("x"), "xyz"));
        GLSLD_CHECK_AST("x.xyzw", SwizzleAccessExpr(NameAccessExpr("x"), "xyzw"));
        GLSLD_CHECK_AST("x.xxxx", SwizzleAccessExpr(NameAccessExpr("x"), "xxxx"));
        GLSLD_CHECK_AST("x.wzyx", SwizzleAccessExpr(NameAccessExpr("x"), "wzyx"));
    }
    // TODO: IndexAccessExpr
    // TODO: FunctionCallExpr
    // TODO: ConstructorCallExpr
}

TEST_CASE_METHOD(AstTestFixture, "Paren Wrapped Expr")
{
    SetTemplate("unknown x = {};",
                [](AstMatcher matcher) { return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher))); });

    GLSLD_CHECK_AST("(1)", LiteralExpr(1));
    GLSLD_CHECK_AST("((1))", LiteralExpr(1));
    GLSLD_CHECK_AST("(1 + 2)", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
    GLSLD_CHECK_AST("(1 + 2) * 3", BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)),
                                              LiteralExpr(3)));
}

TEST_CASE_METHOD(AstTestFixture, "Permissive Expr")
{
    SetTemplate("unknown x = {};",
                [](AstMatcher matcher) { return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher))); });

    GLSLD_CHECK_AST("", ErrorExpr());
    GLSLD_CHECK_AST("1 +", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), ErrorExpr()));
    GLSLD_CHECK_AST("+", UnaryExpr(UnaryOp::Identity, ErrorExpr()));
    GLSLD_CHECK_AST("*", BinaryExpr(BinaryOp::Mul, ErrorExpr(), ErrorExpr()));

    GLSLD_CHECK_AST("(1", LiteralExpr(1));
}

TEST_CASE_METHOD(AstTestFixture, "Initializer List")
{
    SetTemplate("unknown x = {};",
                [](AstMatcher matcher) { return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher))); });

    GLSLD_CHECK_AST("{}", InitializerList());
    GLSLD_CHECK_AST("{1}", InitializerList(LiteralExpr(1)));
    GLSLD_CHECK_AST("{1,}", InitializerList(LiteralExpr(1)));
    GLSLD_CHECK_AST("{1,2}", InitializerList(LiteralExpr(1), LiteralExpr(2)));
    GLSLD_CHECK_AST("{1,2,}", InitializerList(LiteralExpr(1), LiteralExpr(2)));

    GLSLD_CHECK_AST("{{}}", InitializerList(InitializerList()));
    GLSLD_CHECK_AST("{{},}", InitializerList(InitializerList()));
    GLSLD_CHECK_AST("{{},{}}", InitializerList(InitializerList(), InitializerList()));

    GLSLD_CHECK_AST("{{1}}", InitializerList(InitializerList(LiteralExpr(1))));
    GLSLD_CHECK_AST("{{1}, {2, {3}}}",
                    InitializerList(InitializerList(LiteralExpr(1)),
                                    InitializerList(LiteralExpr(2), InitializerList(LiteralExpr(3)))));
}

TEST_CASE_METHOD(AstTestFixture, "Permissive Initializer List")
{
    SetTemplate("unknown x = {};",
                [](AstMatcher matcher) { return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher))); });

    GLSLD_CHECK_AST("{,}", InitializerList(ErrorExpr()));
    GLSLD_CHECK_AST("{,,}", InitializerList(ErrorExpr(), ErrorExpr()));
}
