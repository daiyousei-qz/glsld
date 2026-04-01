#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::ConstEvalTest")
{
    SetTestTemplate("void main() {{ {}; }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), AnyTok(), {}, FindMatch(ExprStmt(AnyExpr()), ExprStmt(matcher))));
    });

    auto checkError = [this]() {
        return AnyExpr()->CheckValue([](const ConstValue& value) { return value.IsError(); });
    };
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
        CheckAst("true", checkScalar(true));
        CheckAst("false", checkScalar(false));

        CheckAst("1", checkScalar(1));
        CheckAst("1u", checkScalar(1u));
        CheckAst("1.0", checkScalar(1.0f));
        CheckAst("1.0lf", checkScalar(1.0));
    }

    SECTION("Scalar Ctor")
    {
        CheckAst("bool(0)", checkScalar(false));
        CheckAst("bool(1)", checkScalar(true));
        CheckAst("int(0.0)", checkScalar(0));
        CheckAst("uint(0)", checkScalar(0u));
        CheckAst("float(0)", checkScalar(0.0f));

        CheckAst("int(vec4(1))", checkScalar(1));
        CheckAst("int(vec4(4, 3, 2, 1))", checkScalar(4));
        CheckAst("int(mat4(1))", checkScalar(1));
        CheckAst("int(mat4(2))", checkScalar(2));
    }

    SECTION("Vector Ctor")
    {
        CheckAst("vec4(1)", checkVector(1.f, 1.f, 1.f, 1.f));
        CheckAst("vec4(1, 2, 3, 4)", checkVector(1.f, 2.f, 3.f, 4.f));
        CheckAst("vec4(1.)", checkVector(1.f, 1.f, 1.f, 1.f));
        CheckAst("vec4(1., 2., 3., 4.)", checkVector(1.f, 2.f, 3.f, 4.f));

        CheckAst("vec4(1, vec3(2, 3, 4))", checkVector(1.f, 2.f, 3.f, 4.f));
        CheckAst("vec4(1, vec2(2, 3), 4)", checkVector(1.f, 2.f, 3.f, 4.f));
        CheckAst("vec4(mat2(1, 2, 3, 4))", checkVector(1.f, 2.f, 3.f, 4.f));
    }

    SECTION("Matrix Ctor")
    {
        // TODO: matrix ctor
    }

    SECTION("UnaryExpr")
    {
        CheckAst("-1", checkScalar(-1));
        CheckAst("!true", checkScalar(false));

        CheckAst("(1).length()", checkError());
        CheckAst("vec4(1).length()", checkScalar(4));
        CheckAst("mat2(1).length()", checkScalar(2));
        CheckAst("mat2x3(1).length()", checkScalar(3));
        CheckAst("mat3x2(1).length()", checkScalar(2));
        CheckAst("int[2u](1, 2).length()", checkScalar(2));
        CheckAst("int[2](unknown1, unknown2).length()", checkScalar(2));
    }

    SECTION("BinaryExpr")
    {
        CheckAst("1 + 2", checkScalar(3));
        CheckAst("1 + 2u", checkScalar(3u));
        CheckAst("1 + 2.0", checkScalar(3.0f));
        CheckAst("vec2(1) + vec2(2)", checkVector(3.f, 3.f));
    }

    SECTION("SelectExpr")
    {
        CheckAst("true ? 1 : 2", checkScalar(1));
        CheckAst("false ? 1 : 2", checkScalar(2));
    }

    SECTION("NameAccessExpr")
    {
        CheckAst("const int x = 1; x", checkScalar(1));
        CheckAst("const vec4 x = vec4(1); x", checkVector(1.f, 1.f, 1.f, 1.f));
    }

    SECTION("FieldAccessExpr")
    {
        // FIXME: field access
    }

    SECTION("SwizzleAccessExpr")
    {
        CheckAst("vec4(1, 2, 3, 4).xy", checkVector(1.f, 2.f));
        CheckAst("vec4(1, 2, 3, 4).xxx", checkVector(1.f, 1.f, 1.f));
        CheckAst("1.0.xxx", checkVector(1.f, 1.f, 1.f));
    }
}