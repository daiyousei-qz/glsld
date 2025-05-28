#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "TypeCheckTest")
{
    SetTestTemplate("void main() {{ {} }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), IdTok("main"), {}, CompoundStmt({ExprStmt(matcher)})));
    });

    SECTION("Swizzle")
    {
        GLSLD_CHECK_AST("(1).x;", SwizzleAccessExpr(LiteralExpr(1), "x")->CheckType(GlslBuiltinType::Ty_int));

        GLSLD_CHECK_AST("vec3().x;", SwizzleAccessExpr(ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {}), "x")
                                         ->CheckType(GlslBuiltinType::Ty_float));

        GLSLD_CHECK_AST("vec3().xy;", SwizzleAccessExpr(ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {}), "xy")
                                          ->CheckType(GlslBuiltinType::Ty_vec2));

        GLSLD_CHECK_AST("vec3().xyz;", SwizzleAccessExpr(ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {}), "xyz")
                                           ->CheckType(GlslBuiltinType::Ty_vec3));

        GLSLD_CHECK_AST("vec3().xyzw;",
                        SwizzleAccessExpr(ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {}), "xyzw")
                            ->CheckType(GlslBuiltinType::Ty_vec4));
    }
}