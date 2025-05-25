#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "AstExprTest")
{
    SetTestTemplate("unknown test__ = {};", [this](AstMatcher* matcher) {
        return FindMatch(VariableDecl(AnyAst(), IdTok("test__"), AnyAst(), AnyAst()),
                         VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher));
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
            GLSLD_CHECK_AST(
                "**", BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Mul, ErrorExpr(), ErrorExpr()), ErrorExpr()));
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
            GLSLD_CHECK_AST("true ? : ? :", SelectExpr(LiteralExpr(true), ErrorExpr(),
                                                       SelectExpr(ErrorExpr(), ErrorExpr(), ErrorExpr())));
        }
    }

    SECTION("SwizzleExpr")
    {
        SetTestTemplate("vec4 x; int y; unknown test__ = {};", [this](AstMatcher* matcher) {
            return FindMatch(VariableDecl(AnyAst(), IdTok("test__"), AnyAst(), AnyAst()),
                             VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher));
        });

        GLSLD_CHECK_AST("x.x", SwizzleAccessExpr(NameAccessExpr("x"), "x"));
        GLSLD_CHECK_AST("x.xy", SwizzleAccessExpr(NameAccessExpr("x"), "xy"));
        GLSLD_CHECK_AST("x.xyz", SwizzleAccessExpr(NameAccessExpr("x"), "xyz"));
        GLSLD_CHECK_AST("x.xyzw", SwizzleAccessExpr(NameAccessExpr("x"), "xyzw"));

        GLSLD_CHECK_AST("y.xxxx", SwizzleAccessExpr(NameAccessExpr("y"), "xxxx"));
        GLSLD_CHECK_AST("y.wzyx", SwizzleAccessExpr(NameAccessExpr("y"), "wzyx"));

        // Swizzle expression has no permissive parsing since it would be classified as other expressions instead.
    }

    SECTION("IndexAccessExpr")
    {
        GLSLD_CHECK_AST("a[1]", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)));
        GLSLD_CHECK_AST("a[1][2]",
                        IndexAccessExpr(IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)), LiteralExpr(2)));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("a[]", IndexAccessExpr(NameAccessExpr("a"), ErrorExpr()));
            GLSLD_CHECK_AST("a[", IndexAccessExpr(NameAccessExpr("a"), ErrorExpr()));
            GLSLD_CHECK_AST("a[1", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)));
            // FIXME: this is seen as a constructor call???
            // GLSLD_CHECK_AST("a[int bool]", IndexAccessExpr(NameAccessExpr("a"), ErrorExpr()));
        }
    }

    SECTION("FunctionCallExpr")
    {
        GLSLD_CHECK_AST("foo()", FunctionCallExpr("foo", {}));
        GLSLD_CHECK_AST("foo(1)", FunctionCallExpr("foo", {LiteralExpr(1)}));
        GLSLD_CHECK_AST("foo((1))", FunctionCallExpr("foo", {LiteralExpr(1)}));
        GLSLD_CHECK_AST("foo(1, 2)", FunctionCallExpr("foo", {LiteralExpr(1), LiteralExpr(2)}));

        SECTION("Permissive")
        {
            // FIXME: shouldn't we parse `foo(` as `foo()`?
            GLSLD_CHECK_AST("foo(", FunctionCallExpr("foo", {ErrorExpr()}));
            GLSLD_CHECK_AST("foo(()", FunctionCallExpr("foo", {ErrorExpr()}));
            GLSLD_CHECK_AST("foo(())", FunctionCallExpr("foo", {ErrorExpr()}));
            GLSLD_CHECK_AST("foo(1,)", FunctionCallExpr("foo", {LiteralExpr(1), ErrorExpr()}));
        }
    }

    SECTION("ConstructorCallExpr")
    {
        SetTestTemplate("struct S{{}}; unknown test__ = {};", [this](AstMatcher* matcher) {
            return FindMatch(VariableDecl(AnyAst(), IdTok("test__"), AnyAst(), AnyAst()),
                             VariableDecl(AnyAst(), AnyTok(), AnyAst(), matcher));
        });

        GLSLD_CHECK_AST("int()", ConstructorCallExpr(NamedType(TokenKlass::K_int), {}));
        GLSLD_CHECK_AST("int[2](1, 2)", ConstructorCallExpr(QualType(NullAst(), KeywordTok(TokenKlass::K_int),
                                                                     ArraySpec({LiteralExpr(2)})),
                                                            {
                                                                LiteralExpr(1),
                                                                LiteralExpr(2),
                                                            }));

        GLSLD_CHECK_AST("vec3()", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {}));
        GLSLD_CHECK_AST("vec3(1)", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                                          LiteralExpr(1),
                                                                                      }));
        GLSLD_CHECK_AST("vec3(1, 2, 3)", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                                                LiteralExpr(1),
                                                                                                LiteralExpr(2),
                                                                                                LiteralExpr(3),
                                                                                            }));

        GLSLD_CHECK_AST("S()", ConstructorCallExpr(NamedType("S"), {}));
        GLSLD_CHECK_AST("S[2]()",
                        ConstructorCallExpr(QualType(NullAst(), IdTok("S"), ArraySpec({LiteralExpr(2)})), {}));

        GLSLD_CHECK_AST("struct {}()", ConstructorCallExpr(StructType(StructDecl(InvalidTok(), {})), {}));
        GLSLD_CHECK_AST("struct X { int x; }(0)",
                        ConstructorCallExpr(
                            StructType(StructDecl(
                                IdTok("X"), {StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("x"), NullAst())})),
                            {
                                LiteralExpr(0),
                            }));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("vec3(", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                                            ErrorExpr(),
                                                                                        }));
            GLSLD_CHECK_AST("vec3(1,)", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                                               LiteralExpr(1),
                                                                                               ErrorExpr(),
                                                                                           }));
        }
    }

    SECTION("Paren Wrapped Expr")
    {
        GLSLD_CHECK_AST("(1)", LiteralExpr(1));
        GLSLD_CHECK_AST("((1))", LiteralExpr(1));
        GLSLD_CHECK_AST("(1 + 2)", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
        GLSLD_CHECK_AST(
            "(1 + 2) * 3",
            BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3)));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("(", ErrorExpr());
            GLSLD_CHECK_AST("()", ErrorExpr());
            GLSLD_CHECK_AST("(())", ErrorExpr());
            GLSLD_CHECK_AST("(1", LiteralExpr(1));
            GLSLD_CHECK_AST("(1 +", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), ErrorExpr()));
        }
    }
}

TEST_CASE_METHOD(CompilerTestFixture, "AstInitializerList")
{
    SetTestTemplate("unknown test__ = {};", [this](AstMatcher* matcher) {
        return FindMatch(VariableDecl(AnyQualType(), IdTok("test__"), AnyAst(), AnyInitializer()),
                         VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher));
    });

    SECTION("Simple")
    {
        SetTestTemplate("int[] test__ = {};", [this](AstMatcher* matcher) {
            return FindMatch(VariableDecl(AnyQualType(), IdTok("test__"), AnyAst(), AnyInitializer()),
                             VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher));
        });

        GLSLD_CHECK_AST("{}", InitializerList({}));
        GLSLD_CHECK_AST("{1}", InitializerList({LiteralExpr(1)}));
        GLSLD_CHECK_AST("{1,}", InitializerList({LiteralExpr(1)}));
        GLSLD_CHECK_AST("{1,2}", InitializerList({LiteralExpr(1), LiteralExpr(2)}));
        GLSLD_CHECK_AST("{1,2,}", InitializerList({LiteralExpr(1), LiteralExpr(2)}));
    }

    SECTION("Nested")
    {
        GLSLD_CHECK_AST("{{}}", InitializerList({
                                    InitializerList({}),
                                }));
        GLSLD_CHECK_AST("{{},}", InitializerList({
                                     InitializerList({}),
                                 }));
        GLSLD_CHECK_AST("{{},{}}", InitializerList({
                                       InitializerList({}),
                                       InitializerList({}),
                                   }));

        // We have implicit cast here because the target type is unknown
        GLSLD_CHECK_AST("{{1}}", InitializerList({
                                     InitializerList({
                                         ImplicitCastExpr(LiteralExpr(1)),
                                     }),
                                 }));
        GLSLD_CHECK_AST("{{1}, {2, {3}}}", InitializerList({
                                               InitializerList({
                                                   ImplicitCastExpr(LiteralExpr(1)),
                                               }),
                                               InitializerList({
                                                   ImplicitCastExpr(LiteralExpr(2)),
                                                   InitializerList({
                                                       ImplicitCastExpr(LiteralExpr(3)),
                                                   }),
                                               }),
                                           }));
    }

    SECTION("Permissive")
    {
        GLSLD_CHECK_AST("{", InitializerList({ErrorExpr()}));
        GLSLD_CHECK_AST("{,", InitializerList({ErrorExpr(), ErrorExpr()}));
        // This is considered as a single-element list like `{1,}`
        GLSLD_CHECK_AST("{,}", InitializerList({ErrorExpr()}));
        GLSLD_CHECK_AST("{,,", InitializerList({ErrorExpr(), ErrorExpr(), ErrorExpr()}));
        // This is considered as a two-element list like `{1,2,}`
        GLSLD_CHECK_AST("{,,}", InitializerList({ErrorExpr(), ErrorExpr()}));
    }
}

TEST_CASE_METHOD(CompilerTestFixture, "AstImplicitCast")
{
    SetTestTemplate("void main() {{ {}; }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({ExprStmt(matcher)})));
    });

    SECTION("BinaryExpr")
    {
        // int + uint -> uint + uint
        GLSLD_CHECK_AST("1 + 2u",
                        BinaryExpr(BinaryOp::Plus, ImplicitCastExpr(LiteralExpr(1)), LiteralExpr(2u))
                            ->CheckType([](const Type& type) { return type.IsSameWith(GlslBuiltinType::Ty_uint); }));
        // int + float -> float + float
        GLSLD_CHECK_AST("1 + 2.0f",
                        BinaryExpr(BinaryOp::Plus, ImplicitCastExpr(LiteralExpr(1)), LiteralExpr(2.0f))
                            ->CheckType([](const Type& type) { return type.IsSameWith(GlslBuiltinType::Ty_float); }));
        // double + float -> double + double
        GLSLD_CHECK_AST("1.0lf + 2.0f",
                        BinaryExpr(BinaryOp::Plus, LiteralExpr(1.0), ImplicitCastExpr(LiteralExpr(2.0f)))
                            ->CheckType([](const Type& type) { return type.IsSameWith(GlslBuiltinType::Ty_double); }));
        // ivecn + vecn -> vecn + vecn
        GLSLD_CHECK_AST(
            "ivec2(1) + vec2(2.0f)",
            BinaryExpr(BinaryOp::Plus,
                       ImplicitCastExpr(ConstructorCallExpr(NamedType(TokenKlass::K_ivec2), {LiteralExpr(1)}))
                           ->CheckType([](const Type& type) { return type.IsSameWith(GlslBuiltinType::Ty_vec2); }),
                       AnyExpr()));
    }

    SECTION("SelectExpr")
    {
        // bool ? int : uint -> bool ? uint : uint
        GLSLD_CHECK_AST("true ? 1 : 2u",
                        SelectExpr(AnyExpr(), ImplicitCastExpr(LiteralExpr(1))->CheckType([](const Type& type) {
                            return type.IsSameWith(GlslBuiltinType::Ty_uint);
                        }),
                                   AnyExpr()));
        // bool ? int : float -> bool ? float : float
        GLSLD_CHECK_AST("true ? 1 : 2.0f",
                        SelectExpr(AnyExpr(), ImplicitCastExpr(LiteralExpr(1))->CheckType([](const Type& type) {
                            return type.IsSameWith(GlslBuiltinType::Ty_float);
                        }),
                                   AnyExpr()));
        // bool ? double : float -> bool ? double : double
        GLSLD_CHECK_AST(
            "true ? 1.0lf : 2.0f",
            SelectExpr(AnyExpr(), AnyExpr(), ImplicitCastExpr(LiteralExpr(2.0f))->CheckType([](const Type& type) {
                return type.IsSameWith(GlslBuiltinType::Ty_double);
            })));
    }

    SECTION("Function Call")
    {
        StringView testTemplate = R"(
            void foo(uint x);
            void main() {{
                {};
            }}
        )";
        SetTestTemplate(testTemplate, [this](AstMatcher* matcher) {
            return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                             FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({ExprStmt(matcher)})));
        });

        GLSLD_CHECK_AST("foo(1)",
                        FunctionCallExpr("foo", {
                                                    ImplicitCastExpr(LiteralExpr(1))->CheckType([](const Type& type) {
                                                        return type.IsSameWith(GlslBuiltinType::Ty_uint);
                                                    }),
                                                }));
    }

    SECTION("Initializer")
    {
        SetTestTemplate("void main() {{ {} test__ = {} }}", [this](AstMatcher* matcher) {
            return FindMatch(
                FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                FunctionDecl(AnyQualType(), AnyTok(), {},
                             CompoundStmt({DeclStmt(VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher))})));
        });

        auto checkFloatType  = [](const Type& type) { return type.IsSameWith(GlslBuiltinType::Ty_float); };
        auto checkDoubleType = [](const Type& type) { return type.IsSameWith(GlslBuiltinType::Ty_double); };

        GLSLD_CHECK_AST(SOURCE_PIECES("float", "1"), ImplicitCastExpr(LiteralExpr(1))->CheckType(checkFloatType));

        GLSLD_CHECK_AST(SOURCE_PIECES("vec2", "{1, 2}"),
                        InitializerList({
                            ImplicitCastExpr(LiteralExpr(1))->CheckType(checkFloatType),
                            ImplicitCastExpr(LiteralExpr(2))->CheckType(checkFloatType),
                        }));

        GLSLD_CHECK_AST(SOURCE_PIECES("struct { float x; double y; }", "{1, 2}"),
                        InitializerList({
                            ImplicitCastExpr(LiteralExpr(1))->CheckType(checkFloatType),
                            ImplicitCastExpr(LiteralExpr(2))->CheckType(checkDoubleType),
                        }));
    }
}