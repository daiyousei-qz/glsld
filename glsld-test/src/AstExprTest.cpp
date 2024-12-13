#include "AstTestFixture.h"

#include <catch2/catch_all.hpp>

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Expr")
{
    SetTestTemplate("unknown x = {};", [](AstMatcher matcher) {
        return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher)));
    });

    SECTION("ErrorExpr")
    {
        GLSLD_CHECK_AST("", ErrorExpr());
    }

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

        SECTION("Permissive")
        {
            // FIXME: match invalid syntax token with better syntax
            GLSLD_CHECK_AST("a.", FieldAccessExpr(NameAccessExpr("a"), ""));
            GLSLD_CHECK_AST("a..b", FieldAccessExpr(FieldAccessExpr(NameAccessExpr("a"), ""), "b"));
        }
    }

    SECTION("UnaryExpr")
    {
        GLSLD_CHECK_AST("-1", UnaryExpr(UnaryOp::Negate, LiteralExpr(1)));
        GLSLD_CHECK_AST("!true", UnaryExpr(UnaryOp::LogicalNot, LiteralExpr(true)));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("+", UnaryExpr(UnaryOp::Identity, ErrorExpr()));
        }
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

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("* 1", BinaryExpr(BinaryOp::Mul, ErrorExpr(), LiteralExpr(1)));
            GLSLD_CHECK_AST("1 *", BinaryExpr(BinaryOp::Mul, LiteralExpr(1), ErrorExpr()));
            GLSLD_CHECK_AST("*", BinaryExpr(BinaryOp::Mul, ErrorExpr(), ErrorExpr()));
        }
    }

    SECTION("SelectExpr")
    {
        GLSLD_CHECK_AST("true ? 1 : 2", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2)));
        GLSLD_CHECK_AST("true ? 1 : false ? 2 : 3",
                        SelectExpr(LiteralExpr(true), LiteralExpr(1),
                                   SelectExpr(LiteralExpr(false), LiteralExpr(2), LiteralExpr(3))));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("true ? 1", SelectExpr(LiteralExpr(true), LiteralExpr(1), ErrorExpr()));
            GLSLD_CHECK_AST("true ? 1 :", SelectExpr(LiteralExpr(true), LiteralExpr(1), ErrorExpr()));
            GLSLD_CHECK_AST("true ? 1 : 2 :", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2)));
        }
    }

    SECTION("SwizzleExpr")
    {
        SetTestTemplate("vec4 x; int y; void main() {{ {} }}", [](AstMatcher matcher) {
            return TranslationUnit(AnyDecl(), AnyDecl(),
                                   FunctionDecl(AnyQualType(), Any(), CompoundStmt(ExprStmt(std::move(matcher)))));
        });

        GLSLD_CHECK_AST("x.x", SwizzleAccessExpr(NameAccessExpr("x"), "x"));
        GLSLD_CHECK_AST("x.xy", SwizzleAccessExpr(NameAccessExpr("x"), "xy"));
        GLSLD_CHECK_AST("x.xyz", SwizzleAccessExpr(NameAccessExpr("x"), "xyz"));
        GLSLD_CHECK_AST("x.xyzw", SwizzleAccessExpr(NameAccessExpr("x"), "xyzw"));

        GLSLD_CHECK_AST("y.xxxx", SwizzleAccessExpr(NameAccessExpr("y"), "xxxx"));
        GLSLD_CHECK_AST("y.wzyx", SwizzleAccessExpr(NameAccessExpr("y"), "wzyx"));
    }

    SECTION("IndexAccessExpr")
    {
        GLSLD_CHECK_AST("a[1]", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)));
        GLSLD_CHECK_AST("a[1][2]", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1), LiteralExpr(2)));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("a[]", IndexAccessExpr(NameAccessExpr("a"), ErrorExpr()));
            GLSLD_CHECK_AST("a[", IndexAccessExpr(NameAccessExpr("a"), ErrorExpr()));
            GLSLD_CHECK_AST("a[1", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)));
        }
    }

    SECTION("FunctionCallExpr")
    {
        GLSLD_CHECK_AST("foo()", FunctionCallExpr("foo"));
        GLSLD_CHECK_AST("foo(1)", FunctionCallExpr("foo", LiteralExpr(1)));
        GLSLD_CHECK_AST("foo(1, 2)", FunctionCallExpr("foo", LiteralExpr(1), LiteralExpr(2)));

        SECTION("Permissive")
        {
            // FIXME: shouldn't we parse `foo(` as `foo()`?
            GLSLD_CHECK_AST("foo(", FunctionCallExpr("foo", ErrorExpr()));
            GLSLD_CHECK_AST("foo(()", FunctionCallExpr("foo", ErrorExpr()));
            GLSLD_CHECK_AST("foo(())", FunctionCallExpr("foo", ErrorExpr()));
            GLSLD_CHECK_AST("foo(1,)", FunctionCallExpr("foo", LiteralExpr(1), ErrorExpr()));
        }
    }

    SECTION("ConstructorCallExpr")
    {
        SetTestTemplate("struct S{{}}; unknown x = {};", [](AstMatcher matcher) {
            return TranslationUnit(AnyDecl(), VariableDecl1(AnyQualType(), std::move(matcher)));
        });

        GLSLD_CHECK_AST("vec3()", ConstructorCallExpr(BuiltinType(glsld::GlslBuiltinType::Ty_vec3)));
        GLSLD_CHECK_AST("vec3(1)", ConstructorCallExpr(BuiltinType(glsld::GlslBuiltinType::Ty_vec3), LiteralExpr(1)));
        GLSLD_CHECK_AST("vec3(1, 2, 3)", ConstructorCallExpr(BuiltinType(glsld::GlslBuiltinType::Ty_vec3),
                                                             LiteralExpr(1), LiteralExpr(2), LiteralExpr(3)));

        GLSLD_CHECK_AST("S()", ConstructorCallExpr(NamedType("S")));

        // FIXME: add `int[2]()`
        // FIXME: add `struct {}()`

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("vec3(", ConstructorCallExpr(BuiltinType(glsld::GlslBuiltinType::Ty_vec3), ErrorExpr()));
            GLSLD_CHECK_AST("vec3(1,)", ConstructorCallExpr(BuiltinType(glsld::GlslBuiltinType::Ty_vec3),
                                                            LiteralExpr(1), ErrorExpr()));
        }
    }
}

TEST_CASE_METHOD(AstTestFixture, "Paren Wrapped Expr")
{
    SetTestTemplate("unknown x = {};", [](AstMatcher matcher) {
        return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher)));
    });

    GLSLD_CHECK_AST("(1)", LiteralExpr(1));
    GLSLD_CHECK_AST("((1))", LiteralExpr(1));
    GLSLD_CHECK_AST("(1 + 2)", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
    GLSLD_CHECK_AST("(1 + 2) * 3", BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)),
                                              LiteralExpr(3)));

    SECTION("Permissive")
    {
        GLSLD_CHECK_AST("(", ErrorExpr());
        GLSLD_CHECK_AST("()", ErrorExpr());
        GLSLD_CHECK_AST("(())", ErrorExpr());
        GLSLD_CHECK_AST("(1", LiteralExpr(1));
        GLSLD_CHECK_AST("(1 +", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), ErrorExpr()));
    }
}

TEST_CASE_METHOD(AstTestFixture, "Initializer List")
{
    SetTestTemplate("unknown x = {};", [](AstMatcher matcher) {
        return TranslationUnit(VariableDecl1(AnyQualType(), std::move(matcher)));
    });

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

    SECTION("Permissive")
    {
        GLSLD_CHECK_AST("{", InitializerList(ErrorExpr()));
        GLSLD_CHECK_AST("{,", InitializerList(ErrorExpr()));
        GLSLD_CHECK_AST("{,}", InitializerList(ErrorExpr(), ErrorExpr()));
        GLSLD_CHECK_AST("{,,}", InitializerList(ErrorExpr(), ErrorExpr()));
    }
}