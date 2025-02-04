#include "AstTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "ConstEval")
{
    SetTestTemplate("void main() {{ {}; }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), AnyTok(), {}, FindMatch(ExprStmt(AnyExpr()), ExprStmt(matcher))));
    });

    auto checkScalar = [this](auto scalarValue) {
        return AnyExpr()->CheckValue(
            [scalarValue](const ConstValue& value) { return value == ConstValue::CreateScalar(scalarValue); });
    };
    auto checkVector = [this](auto... scalarValues) {
        return AnyExpr()->CheckValue([scalarValues...](const ConstValue& value) {
            return value == ConstValue::CreateVector({scalarValues...});
        });
    };

    SECTION("Simple Literal")
    {
        GLSLD_CHECK_AST("true", checkScalar(true));
        GLSLD_CHECK_AST("false", checkScalar(false));

        GLSLD_CHECK_AST("1", checkScalar(1));
        GLSLD_CHECK_AST("1u", checkScalar(1u));
        GLSLD_CHECK_AST("1.0", checkScalar(1.0f));
        GLSLD_CHECK_AST("1.0lf", checkScalar(1.0));
    }

    SECTION("Scalar Ctor")
    {
        GLSLD_CHECK_AST("bool(0)", checkScalar(false));
        GLSLD_CHECK_AST("bool(1)", checkScalar(true));
        GLSLD_CHECK_AST("int(0.0)", checkScalar(0));
        GLSLD_CHECK_AST("uint(0)", checkScalar(0u));
        GLSLD_CHECK_AST("float(0)", checkScalar(0.0f));

        GLSLD_CHECK_AST("int(vec4(1))", checkScalar(1));
        GLSLD_CHECK_AST("int(vec4(4, 3, 2, 1))", checkScalar(4));
        GLSLD_CHECK_AST("int(mat4(1))", checkScalar(1));
        GLSLD_CHECK_AST("int(mat4(2))", checkScalar(2));
    }

    SECTION("Vector Ctor")
    {
        GLSLD_CHECK_AST("vec4(1)", checkVector(1.f, 1.f, 1.f, 1.f));
        GLSLD_CHECK_AST("vec4(1, 2, 3, 4)", checkVector(1.f, 2.f, 3.f, 4.f));
        GLSLD_CHECK_AST("vec4(1.)", checkVector(1.f, 1.f, 1.f, 1.f));
        GLSLD_CHECK_AST("vec4(1., 2., 3., 4.)", checkVector(1.f, 2.f, 3.f, 4.f));

        GLSLD_CHECK_AST("vec4(1, vec3(2, 3, 4))", checkVector(1.f, 2.f, 3.f, 4.f));
        GLSLD_CHECK_AST("vec4(1, vec2(2, 3), 4)", checkVector(1.f, 2.f, 3.f, 4.f));
        GLSLD_CHECK_AST("vec4(mat2(1, 2, 3, 4))", checkVector(1.f, 2.f, 3.f, 4.f));
    }

    SECTION("Matrix Ctor")
    {
        // TODO: matrix ctor
    }

    SECTION("UnaryExpr")
    {
        GLSLD_CHECK_AST("-1", checkScalar(-1));
        GLSLD_CHECK_AST("!true", checkScalar(false));
    }

    SECTION("BinaryExpr")
    {
        GLSLD_CHECK_AST("1 + 2", checkScalar(3));
        GLSLD_CHECK_AST("1 + 2u", checkScalar(3u));
        GLSLD_CHECK_AST("1 + 2.0", checkScalar(3.0f));
        GLSLD_CHECK_AST("vec2(1) + vec2(2)", checkVector(3.f, 3.f));
    }

    SECTION("SelectExpr")
    {
        GLSLD_CHECK_AST("true ? 1 : 2", checkScalar(1));
        GLSLD_CHECK_AST("false ? 1 : 2", checkScalar(2));
    }

    SECTION("NameAccessExpr")
    {
        GLSLD_CHECK_AST("const int x = 1; x", checkScalar(1));
        GLSLD_CHECK_AST("const vec4 x = vec4(1); x", checkVector(1.f, 1.f, 1.f, 1.f));
    }

    SECTION("FieldAccessExpr")
    {
        // FIXME: field access
    }

    SECTION("SwizzleAccessExpr")
    {
        GLSLD_CHECK_AST("vec4(1, 2, 3, 4).xy", checkVector(1.f, 2.f));
        GLSLD_CHECK_AST("vec4(1, 2, 3, 4).xxx", checkVector(1.f, 1.f, 1.f));
        GLSLD_CHECK_AST("1.0.xxx", checkVector(1.f, 1.f, 1.f));
    }
}