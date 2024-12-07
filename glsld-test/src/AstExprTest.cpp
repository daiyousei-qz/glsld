#include "AstTestFixture.h"

#include <catch2/catch_all.hpp>

using namespace glsld;

TEST_CASE("Simple Expr", "")
{
    SECTION("LiteralExpr")
    {
        REQUIRE(AstExprTest{"1", LiteralExpr(1)}.Positive());
        REQUIRE(AstExprTest{"1.0", LiteralExpr(1.0f)}.Positive());
        REQUIRE(AstExprTest{"true", LiteralExpr(true)}.Positive());
    }

    SECTION("NameAccessExpr")
    {
        REQUIRE(AstExprTest{"a", NameAccessExpr("a")}.Positive());
    }

    SECTION("FieldAccessExpr")
    {
        REQUIRE(AstExprTest{"a.b", FieldAccessExpr(NameAccessExpr("a"), "b")}.Positive());
        REQUIRE(AstExprTest{"a.b.c", FieldAccessExpr(FieldAccessExpr(NameAccessExpr("a"), "b"), "c")}.Positive());
    }

    SECTION("UnaryExpr")
    {
        REQUIRE(AstExprTest{"-1", UnaryExpr(UnaryOp::Negate, LiteralExpr(1))}.Positive());
        REQUIRE(AstExprTest{"!true", UnaryExpr(UnaryOp::LogicalNot, LiteralExpr(true))}.Positive());
    }

    SECTION("BinaryExpr")
    {
        REQUIRE(AstExprTest{"1 + 2", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2))}.Positive());
        REQUIRE(AstExprTest{
            "1 + 2 + 3",
            BinaryExpr(BinaryOp::Plus, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3))}
                    .Positive());
        REQUIRE(AstExprTest{"1 + 2 * 3", BinaryExpr(BinaryOp::Plus, LiteralExpr(1),
                                                    BinaryExpr(BinaryOp::Mul, LiteralExpr(2), LiteralExpr(3)))}
                    .Positive());
        REQUIRE(AstExprTest{
            "1 * 2 + 3",
            BinaryExpr(BinaryOp::Plus, BinaryExpr(BinaryOp::Mul, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3))}
                    .Positive());
    }

    SECTION("SelectExpr")
    {
        REQUIRE(AstExprTest{"true ? 1 : 2", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2))}.Positive());
        REQUIRE(AstExprTest{"true ? 1 : false ? 2 : 3",
                            SelectExpr(LiteralExpr(true), LiteralExpr(1),
                                       SelectExpr(LiteralExpr(false), LiteralExpr(2), LiteralExpr(3)))}
                    .Positive());
    }

    // TODO: SwizzleExpr
    // TODO: IndexAccessExpr
    // TODO: FunctionCallExpr
    // TODO: ConstructorCallExpr
}

TEST_CASE("Paren Wrapped Expr")
{
    REQUIRE(AstExprTest{"(1)", LiteralExpr(1)}.Positive());
    REQUIRE(AstExprTest{"((1))", LiteralExpr(1)}.Positive());
    REQUIRE(AstExprTest{"(1 + 2)", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2))}.Positive());
    REQUIRE(
        AstExprTest{"(1 + 2) * 3", BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)),
                                              LiteralExpr(3))}
            .Positive());
}

TEST_CASE("Permissive Expr")
{
    REQUIRE(AstExprTest{"", ErrorExpr()}.Positive());
    REQUIRE(AstExprTest{"1 +", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), ErrorExpr())}.Positive());
    REQUIRE(AstExprTest{"+", UnaryExpr(UnaryOp::Identity, ErrorExpr())}.Positive());
    REQUIRE(AstExprTest{"*", BinaryExpr(BinaryOp::Mul, ErrorExpr(), ErrorExpr())}.Positive());
}

TEST_CASE("Initializer List")
{
    REQUIRE(AstExprTest{"{}", InitializerList()}.Positive());
    REQUIRE(AstExprTest{"{1}", InitializerList(LiteralExpr(1))}.Positive());
    REQUIRE(AstExprTest{"{1,}", InitializerList(LiteralExpr(1))}.Positive());
    REQUIRE(AstExprTest{"{1,2}", InitializerList(LiteralExpr(1), LiteralExpr(2))}.Positive());
    REQUIRE(AstExprTest{"{1,2,}", InitializerList(LiteralExpr(1), LiteralExpr(2))}.Positive());

    REQUIRE(AstExprTest{"{{}}", InitializerList(InitializerList())}.Positive());
    REQUIRE(AstExprTest{"{{},}", InitializerList(InitializerList())}.Positive());
    REQUIRE(AstExprTest{"{{},{}}", InitializerList(InitializerList(), InitializerList())}.Positive());

    REQUIRE(AstExprTest{"{{1}}", InitializerList(InitializerList(LiteralExpr(1)))}.Positive());
    REQUIRE(AstExprTest{"{{1}, {2, {3}},}",
                        InitializerList(InitializerList(LiteralExpr(1)),
                                        InitializerList(LiteralExpr(2), InitializerList(LiteralExpr(3))))}
                .Positive());
}

TEST_CASE("Permissive Initializer List")
{
    REQUIRE(AstExprTest{"{,}", InitializerList()}.Positive());
    REQUIRE(AstExprTest{"{,,,}", InitializerList()}.Positive());
}
