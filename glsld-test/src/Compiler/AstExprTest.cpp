#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::AstExprTest")
{
    SetTestTemplate("unknown test__ = {};", [this](AstMatcher* matcher) {
        return FindMatch(VariableDecl(AnyAst(), IdTok("test__"), AnyAst(), AnyAst()),
                         VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher));
    });

    SECTION("ErrorExpr")
    {
        CheckAst("", ErrorExpr());
    }

    SECTION("LiteralExpr")
    {
        // Integer literal
        CheckAst("1", LiteralExpr(1));
        CheckAst("0xe1", LiteralExpr(0xe1));
        CheckAst("0xf", LiteralExpr(0xf));

        // Float literal
        CheckAst("1.0", LiteralExpr(1.0f));
        CheckAst("1e1", LiteralExpr(1e1f));

        // Double literal
        CheckAst("1.0lf", LiteralExpr(1.0));
        CheckAst("1e1lf", LiteralExpr(1e1));

        // Bool literal
        CheckAst("true", LiteralExpr(true));
    }

    SECTION("NameAccessExpr")
    {
        CheckAst("a", NameAccessExpr("a"));
    }

    SECTION("FieldAccessExpr")
    {
        CheckAst("a.b", FieldAccessExpr(NameAccessExpr("a"), "b"));
        CheckAst("a.b.c", FieldAccessExpr(FieldAccessExpr(NameAccessExpr("a"), "b"), "c"));
        CheckAst("foo().bar", FieldAccessExpr(FunctionCallExpr("foo", {}), "bar"));

        SECTION("Permissive")
        {
            // FIXME: match invalid syntax token with better syntax
            CheckAst("a.", FieldAccessExpr(NameAccessExpr("a"), ""));
            CheckAst("a..b", FieldAccessExpr(FieldAccessExpr(NameAccessExpr("a"), ""), "b"));
        }
    }

    SECTION("UnaryExpr")
    {
        CheckAst("+1", UnaryExpr(UnaryOp::Identity, LiteralExpr(1)));
        CheckAst("-1", UnaryExpr(UnaryOp::Negate, LiteralExpr(1)));
        CheckAst("~1", UnaryExpr(UnaryOp::BitwiseNot, LiteralExpr(1)));
        CheckAst("!true", UnaryExpr(UnaryOp::LogicalNot, LiteralExpr(true)));
        CheckAst("++a", UnaryExpr(UnaryOp::PrefixInc, NameAccessExpr("a")));
        CheckAst("--a", UnaryExpr(UnaryOp::PrefixDec, NameAccessExpr("a")));
        CheckAst("a++", UnaryExpr(UnaryOp::PostfixInc, NameAccessExpr("a")));
        CheckAst("a--", UnaryExpr(UnaryOp::PostfixDec, NameAccessExpr("a")));
        CheckAst("vec3().length()", UnaryExpr(UnaryOp::Length, ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {})));
        CheckAst("--(--a)", UnaryExpr(UnaryOp::PrefixDec, UnaryExpr(UnaryOp::PrefixDec, NameAccessExpr("a"))));
        CheckAst("-~a", UnaryExpr(UnaryOp::Negate, UnaryExpr(UnaryOp::BitwiseNot, NameAccessExpr("a"))));
        CheckAst("-(a++)", UnaryExpr(UnaryOp::Negate, UnaryExpr(UnaryOp::PostfixInc, NameAccessExpr("a"))));

        SECTION("Permissive")
        {
            CheckAst("+", UnaryExpr(UnaryOp::Identity, ErrorExpr()));
            CheckAst("++", UnaryExpr(UnaryOp::PrefixInc, ErrorExpr()));
        }
    }

    SECTION("CommaExpr")
    {
        SetTestTemplate("void main() {{ {}; }};", [this](AstMatcher* matcher) {
            return FindMatch(FunctionDecl(AnyAst(), IdTok("main"), {}, AnyAst()),
                             FunctionDecl(AnyAst(), AnyTok(), {}, CompoundStmt({ExprStmt(matcher)})));
        });

        CheckAst("1, 2", BinaryExpr(BinaryOp::Comma, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1, 2, 3", BinaryExpr(BinaryOp::Comma, BinaryExpr(BinaryOp::Comma, LiteralExpr(1), LiteralExpr(2)),
                                       LiteralExpr(3)));
        CheckAst("(a = 1, b)",
                 BinaryExpr(BinaryOp::Comma, BinaryExpr(BinaryOp::Assign, NameAccessExpr("a"), LiteralExpr(1)),
                            NameAccessExpr("b")));
    }

    SECTION("AssignmentExpr")
    {
        SetTestTemplate("void main() {{ {}; }};", [this](AstMatcher* matcher) {
            return FindMatch(FunctionDecl(AnyAst(), IdTok("main"), {}, AnyAst()),
                             FunctionDecl(AnyAst(), AnyTok(), {}, CompoundStmt({ExprStmt(matcher)})));
        });

        CheckAst("a = 1", BinaryExpr(BinaryOp::Assign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a += 1", BinaryExpr(BinaryOp::AddAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a -= 1", BinaryExpr(BinaryOp::SubAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a *= 1", BinaryExpr(BinaryOp::MulAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a /= 1", BinaryExpr(BinaryOp::DivAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a %= 1", BinaryExpr(BinaryOp::ModAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a <<= 1", BinaryExpr(BinaryOp::LShiftAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a >>= 1", BinaryExpr(BinaryOp::RShiftAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a &= 1", BinaryExpr(BinaryOp::AndAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a |= 1", BinaryExpr(BinaryOp::OrAssign, NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a ^= 1", BinaryExpr(BinaryOp::XorAssign, NameAccessExpr("a"), LiteralExpr(1)));

        CheckAst("a = b = 1", BinaryExpr(BinaryOp::Assign, NameAccessExpr("a"),
                                         BinaryExpr(BinaryOp::Assign, NameAccessExpr("b"), LiteralExpr(1))));
        CheckAst("a += b = 1", BinaryExpr(BinaryOp::AddAssign, NameAccessExpr("a"),
                                          BinaryExpr(BinaryOp::Assign, NameAccessExpr("b"), LiteralExpr(1))));
        CheckAst("a = b += 1", BinaryExpr(BinaryOp::Assign, NameAccessExpr("a"),
                                          BinaryExpr(BinaryOp::AddAssign, NameAccessExpr("b"), LiteralExpr(1))));
        CheckAst("a *= b += c", BinaryExpr(BinaryOp::MulAssign, NameAccessExpr("a"),
                                           BinaryExpr(BinaryOp::AddAssign, NameAccessExpr("b"), NameAccessExpr("c"))));

        SECTION("Permissive")
        {
            CheckAst("1 = 2", BinaryExpr(BinaryOp::Assign, LiteralExpr(1), LiteralExpr(2)));
            CheckAst("=", BinaryExpr(BinaryOp::Assign, ErrorExpr(), ErrorExpr()));
            CheckAst("a =", BinaryExpr(BinaryOp::Assign, NameAccessExpr("a"), ErrorExpr()));
            CheckAst("= b", BinaryExpr(BinaryOp::Assign, ErrorExpr(), NameAccessExpr("b")));
        }
    }

    SECTION("BinaryExpr")
    {
        CheckAst("1 + 2", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 - 2", BinaryExpr(BinaryOp::Minus, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 * 2", BinaryExpr(BinaryOp::Mul, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 / 2", BinaryExpr(BinaryOp::Div, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 % 2", BinaryExpr(BinaryOp::Modulo, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 == 2", BinaryExpr(BinaryOp::Equal, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 != 2", BinaryExpr(BinaryOp::NotEqual, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 < 2", BinaryExpr(BinaryOp::Less, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 <= 2", BinaryExpr(BinaryOp::LessEq, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 > 2", BinaryExpr(BinaryOp::Greater, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 >= 2", BinaryExpr(BinaryOp::GreaterEq, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 & 2", BinaryExpr(BinaryOp::BitwiseAnd, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 | 2", BinaryExpr(BinaryOp::BitwiseOr, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 ^ 2", BinaryExpr(BinaryOp::BitwiseXor, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 && 2", BinaryExpr(BinaryOp::LogicalAnd, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 || 2", BinaryExpr(BinaryOp::LogicalOr, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 << 2", BinaryExpr(BinaryOp::ShiftLeft, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("1 >> 2", BinaryExpr(BinaryOp::ShiftRight, LiteralExpr(1), LiteralExpr(2)));

        CheckAst("1 + 2 + 3", BinaryExpr(BinaryOp::Plus, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)),
                                         LiteralExpr(3)));
        CheckAst("1 + 2 * 3",
                 BinaryExpr(BinaryOp::Plus, LiteralExpr(1), BinaryExpr(BinaryOp::Mul, LiteralExpr(2), LiteralExpr(3))));
        CheckAst("1 * 2 + 3",
                 BinaryExpr(BinaryOp::Plus, BinaryExpr(BinaryOp::Mul, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3)));
        CheckAst("1 - 2 - 3", BinaryExpr(BinaryOp::Minus, BinaryExpr(BinaryOp::Minus, LiteralExpr(1), LiteralExpr(2)),
                                         LiteralExpr(3)));
        CheckAst("1 << 2 + 3", BinaryExpr(BinaryOp::ShiftLeft, LiteralExpr(1),
                                          BinaryExpr(BinaryOp::Plus, LiteralExpr(2), LiteralExpr(3))));
        CheckAst("1 + (2 << 3)", BinaryExpr(BinaryOp::Plus, LiteralExpr(1),
                                            BinaryExpr(BinaryOp::ShiftLeft, LiteralExpr(2), LiteralExpr(3))));

        SECTION("Permissive")
        {
            CheckAst("* 1", BinaryExpr(BinaryOp::Mul, ErrorExpr(), LiteralExpr(1)));
            CheckAst("1 *", BinaryExpr(BinaryOp::Mul, LiteralExpr(1), ErrorExpr()));
            CheckAst("*", BinaryExpr(BinaryOp::Mul, ErrorExpr(), ErrorExpr()));
            CheckAst("**", BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Mul, ErrorExpr(), ErrorExpr()), ErrorExpr()));
        }
    }

    SECTION("SelectExpr")
    {
        CheckAst("true ? 1 : 2", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2)));
        CheckAst("true ? 1 : false ? 2 : 3",
                 SelectExpr(LiteralExpr(true), LiteralExpr(1),
                            SelectExpr(LiteralExpr(false), LiteralExpr(2), LiteralExpr(3))));
        CheckAst("true ? false ? 1 : 2 : 3",
                 SelectExpr(LiteralExpr(true), SelectExpr(LiteralExpr(false), LiteralExpr(1), LiteralExpr(2)),
                            LiteralExpr(3)));
        CheckAst("a ? b : c + d", SelectExpr(NameAccessExpr("a"), NameAccessExpr("b"),
                                             BinaryExpr(BinaryOp::Plus, NameAccessExpr("c"), NameAccessExpr("d"))));

        SECTION("Permissive")
        {
            CheckAst("true ? 1", SelectExpr(LiteralExpr(true), LiteralExpr(1), ErrorExpr()));
            CheckAst("true ? 1 :", SelectExpr(LiteralExpr(true), LiteralExpr(1), ErrorExpr()));
            CheckAst("true ? 1 : 2 :", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2)));
            CheckAst("true ? : ? :",
                     SelectExpr(LiteralExpr(true), ErrorExpr(), SelectExpr(ErrorExpr(), ErrorExpr(), ErrorExpr())));
        }
    }

    SECTION("SwizzleExpr")
    {
        SetTestTemplate("vec4 x; int y; unknown test__ = {};", [this](AstMatcher* matcher) {
            return FindMatch(VariableDecl(AnyAst(), IdTok("test__"), AnyAst(), AnyAst()),
                             VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher));
        });

        CheckAst("x.x", SwizzleAccessExpr(NameAccessExpr("x"), "x"));
        CheckAst("x.xy", SwizzleAccessExpr(NameAccessExpr("x"), "xy"));
        CheckAst("x.xyz", SwizzleAccessExpr(NameAccessExpr("x"), "xyz"));
        CheckAst("x.xyzw", SwizzleAccessExpr(NameAccessExpr("x"), "xyzw"));

        CheckAst("y.xxxx", SwizzleAccessExpr(NameAccessExpr("y"), "xxxx"));
        CheckAst("y.wzyx", SwizzleAccessExpr(NameAccessExpr("y"), "wzyx"));

        // Swizzle expression has no permissive parsing since it would be classified as other expressions instead.
    }

    SECTION("IndexAccessExpr")
    {
        CheckAst("a[1]", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)));
        CheckAst("a[1u]", IndexAccessExpr(NameAccessExpr("a"),
                                          ImplicitCastExpr(LiteralExpr(1u))->CheckType(GlslBuiltinType::Ty_int)));
        CheckAst("a[1][2]", IndexAccessExpr(IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)), LiteralExpr(2)));

        CheckAst("foo()[1]", IndexAccessExpr(FunctionCallExpr("foo", {}), LiteralExpr(1)));
        CheckAst("foo()[bar]",
                 IndexAccessExpr(FunctionCallExpr("foo", {}),
                                 ImplicitCastExpr(NameAccessExpr("bar"))->CheckType(GlslBuiltinType::Ty_int)));

        SECTION("Permissive")
        {
            CheckAst("a[]", IndexAccessExpr(NameAccessExpr("a"),
                                            ImplicitCastExpr(ErrorExpr())->CheckType(GlslBuiltinType::Ty_int)));
            CheckAst("a[", IndexAccessExpr(NameAccessExpr("a"),
                                           ImplicitCastExpr(ErrorExpr())->CheckType(GlslBuiltinType::Ty_int)));
            CheckAst("a[1", IndexAccessExpr(NameAccessExpr("a"), LiteralExpr(1)));
            // FIXME: this is seen as a constructor call???
            // CheckAst("a[int bool]", IndexAccessExpr(NameAccessExpr("a"), ErrorExpr()));
        }
    }

    SECTION("MixedAccessExpr")
    {
        CheckAst("foo().bar[1]", IndexAccessExpr(FieldAccessExpr(FunctionCallExpr("foo", {}), "bar"), LiteralExpr(1)));
        CheckAst("foo()[1].bar", FieldAccessExpr(IndexAccessExpr(FunctionCallExpr("foo", {}), LiteralExpr(1)), "bar"));
        CheckAst("foo().bar.baz", FieldAccessExpr(FieldAccessExpr(FunctionCallExpr("foo", {}), "bar"), "baz"));
    }

    SECTION("FunctionCallExpr")
    {
        CheckAst("foo()", FunctionCallExpr("foo", {}));
        CheckAst("foo(1)", FunctionCallExpr("foo", {LiteralExpr(1)}));
        CheckAst("foo((1))", FunctionCallExpr("foo", {LiteralExpr(1)}));
        CheckAst("foo(1, 2)", FunctionCallExpr("foo", {LiteralExpr(1), LiteralExpr(2)}));
        CheckAst("foo(bar(1))", FunctionCallExpr("foo", {FunctionCallExpr("bar", {LiteralExpr(1)})}));
        CheckAst("foo(bar(1), baz(2, 3))",
                 FunctionCallExpr("foo", {FunctionCallExpr("bar", {LiteralExpr(1)}),
                                          FunctionCallExpr("baz", {LiteralExpr(2), LiteralExpr(3)})}));

        SECTION("Permissive")
        {
            // FIXME: shouldn't we parse `foo(` as `foo()`?
            CheckAst("foo(", FunctionCallExpr("foo", {ErrorExpr()}));
            CheckAst("foo(()", FunctionCallExpr("foo", {ErrorExpr()}));
            CheckAst("foo(())", FunctionCallExpr("foo", {ErrorExpr()}));
            CheckAst("foo(1,)", FunctionCallExpr("foo", {LiteralExpr(1), ErrorExpr()}));
        }
    }

    SECTION("ConstructorCallExpr")
    {
        SetTestTemplate("struct S{{}}; unknown test__ = {};", [this](AstMatcher* matcher) {
            return FindMatch(VariableDecl(AnyAst(), IdTok("test__"), AnyAst(), AnyAst()),
                             VariableDecl(AnyAst(), AnyTok(), AnyAst(), matcher));
        });

        CheckAst("int()", ConstructorCallExpr(NamedType(TokenKlass::K_int), {}));
        CheckAst("int[2](1, 2)",
                 ConstructorCallExpr(QualType(NullAst(), KeywordTok(TokenKlass::K_int), ArraySpec({LiteralExpr(2)})),
                                     {
                                         LiteralExpr(1),
                                         LiteralExpr(2),
                                     }));
        CheckAst("float(1)", ConstructorCallExpr(NamedType(TokenKlass::K_float), {LiteralExpr(1)}));

        CheckAst("vec3()", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {}));
        CheckAst("vec3(1)", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                                   ImplicitCastExpr(LiteralExpr(1)),
                                                                               }));
        CheckAst("mat3(vec3(1.0), vec3(2.0), vec3(3.0))",
                 ConstructorCallExpr(NamedType(TokenKlass::K_mat3),
                                     {
                                         ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {LiteralExpr(1.f)}),
                                         ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {LiteralExpr(2.f)}),
                                         ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {LiteralExpr(3.f)}),
                                     }));

        CheckAst("S()", ConstructorCallExpr(NamedType("S"), {}));
        CheckAst("S[2]()", ConstructorCallExpr(QualType(NullAst(), IdTok("S"), ArraySpec({LiteralExpr(2)})), {}));

        CheckAst("struct {}()", ConstructorCallExpr(StructType(StructDecl(InvalidTok(), {})), {}));
        CheckAst("struct X { int x; }(0)",
                 ConstructorCallExpr(StructType(StructDecl(IdTok("X"), {StructFieldDecl(NamedType(TokenKlass::K_int),
                                                                                        IdTok("x"), NullAst())})),
                                     {
                                         LiteralExpr(0),
                                     }));

        SECTION("Permissive")
        {
            CheckAst("vec3(", ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                                     ErrorExpr(),
                                                                                 }));
            CheckAst("vec3(1,)",
                     ConstructorCallExpr(NamedType(TokenKlass::K_vec3), {
                                                                            ImplicitCastExpr(LiteralExpr(1)),
                                                                            ErrorExpr(),
                                                                        }));
        }
    }

    SECTION("Paren Wrapped Expr")
    {
        CheckAst("(1)", LiteralExpr(1));
        CheckAst("((1))", LiteralExpr(1));
        CheckAst("(1 + 2)", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)));
        CheckAst("(1 + 2) * 3",
                 BinaryExpr(BinaryOp::Mul, BinaryExpr(BinaryOp::Plus, LiteralExpr(1), LiteralExpr(2)), LiteralExpr(3)));
        CheckAst("(true ? 1 : 2)", SelectExpr(LiteralExpr(true), LiteralExpr(1), LiteralExpr(2)));

        SECTION("Permissive")
        {
            CheckAst("(", ErrorExpr());
            CheckAst("()", ErrorExpr());
            CheckAst("(())", ErrorExpr());
            CheckAst("(1", LiteralExpr(1));
            CheckAst("(1 +", BinaryExpr(BinaryOp::Plus, LiteralExpr(1), ErrorExpr()));
        }
    }
}

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::AstInitializerListTest")
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

        CheckAst("{}", InitializerList({}));
        CheckAst("{1}", InitializerList({LiteralExpr(1)}));
        CheckAst("{1,}", InitializerList({LiteralExpr(1)}));
        CheckAst("{1,2}", InitializerList({LiteralExpr(1), LiteralExpr(2)}));
        CheckAst("{1,2,}", InitializerList({LiteralExpr(1), LiteralExpr(2)}));
    }

    SECTION("Nested")
    {
        CheckAst("{{}}", InitializerList({
                             InitializerList({}),
                         }));
        CheckAst("{{},}", InitializerList({
                              InitializerList({}),
                          }));
        CheckAst("{{},{}}", InitializerList({
                                InitializerList({}),
                                InitializerList({}),
                            }));

        CheckAst("{{1}}", InitializerList({
                              InitializerList({
                                  LiteralExpr(1),
                              }),
                          }));
        CheckAst("{{1}, {2, {3}}}", InitializerList({
                                        InitializerList({
                                            LiteralExpr(1),
                                        }),
                                        InitializerList({
                                            LiteralExpr(2),
                                            InitializerList({
                                                LiteralExpr(3),
                                            }),
                                        }),
                                    }));
    }

    SECTION("Permissive")
    {
        CheckAst("{", InitializerList({ErrorExpr()}));
        CheckAst("{,", InitializerList({ErrorExpr(), ErrorExpr()}));
        // This is considered as a single-element list like `{1,}`
        CheckAst("{,}", InitializerList({ErrorExpr()}));
        CheckAst("{,,", InitializerList({ErrorExpr(), ErrorExpr(), ErrorExpr()}));
        // This is considered as a two-element list like `{1,2,}`
        CheckAst("{,,}", InitializerList({ErrorExpr(), ErrorExpr()}));
    }
}

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::AstImplicitCastTest")
{
    SetTestTemplate("void main() {{ {}; }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({ExprStmt(matcher)})));
    });

    SECTION("BinaryExpr")
    {
        // int + uint -> uint + uint
        CheckAst("1 + 2u", BinaryExpr(BinaryOp::Plus, ImplicitCastExpr(LiteralExpr(1)), LiteralExpr(2u))
                               ->CheckType(GlslBuiltinType::Ty_uint));
        // int + float -> float + float
        CheckAst("1 + 2.0f", BinaryExpr(BinaryOp::Plus, ImplicitCastExpr(LiteralExpr(1)), LiteralExpr(2.0f))
                                 ->CheckType(GlslBuiltinType::Ty_float));
        // double + float -> double + double
        CheckAst("1.0lf + 2.0f", BinaryExpr(BinaryOp::Plus, LiteralExpr(1.0), ImplicitCastExpr(LiteralExpr(2.0f)))
                                     ->CheckType(GlslBuiltinType::Ty_double));
        // ivecn + vecn -> vecn + vecn
        CheckAst("ivec2(1) + vec2(2.0f)",
                 BinaryExpr(BinaryOp::Plus,
                            ImplicitCastExpr(ConstructorCallExpr(NamedType(TokenKlass::K_ivec2), {LiteralExpr(1)}))
                                ->CheckType(GlslBuiltinType::Ty_vec2),
                            AnyExpr()));
    }

    SECTION("SelectExpr")
    {
        // bool ? int : uint -> bool ? uint : uint
        CheckAst(
            "true ? 1 : 2u",
            SelectExpr(AnyExpr(), ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_uint), AnyExpr()));
        // bool ? int : float -> bool ? float : float
        CheckAst(
            "true ? 1 : 2.0f",
            SelectExpr(AnyExpr(), ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float), AnyExpr()));
        // bool ? double : float -> bool ? double : double
        CheckAst("true ? 1.0lf : 2.0f",
                 SelectExpr(AnyExpr(), AnyExpr(),
                            ImplicitCastExpr(LiteralExpr(2.0f))->CheckType(GlslBuiltinType::Ty_double)));
    }

    SECTION("Function Call")
    {
        StringView testTemplate = R"(
            void foo(uint x);
            void bar(double x, double y);
            void main() {{
                {};
            }}
        )";
        SetTestTemplate(testTemplate, [this](AstMatcher* matcher) {
            return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                             FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({ExprStmt(matcher)})));
        });

        CheckAst("foo(1)",
                 FunctionCallExpr("foo", {
                                             ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_uint),
                                         }));
        CheckAst("bar(1, 2.0f)",
                 FunctionCallExpr("bar", {
                                             ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_double),
                                             ImplicitCastExpr(LiteralExpr(2.0f))->CheckType(GlslBuiltinType::Ty_double),
                                         }));
    }

    SECTION("Constructor")
    {
        SetTestTemplate("void main() {{ {} test__ = {} }}", [this](AstMatcher* matcher) {
            return FindMatch(
                FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                FunctionDecl(AnyQualType(), AnyTok(), {},
                             CompoundStmt({DeclStmt(VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher))})));
        });

        // Conversion constructor doesn't introduce an implicit cast
        CheckAst(MakeSourcePieces("float", "float(1)"),
                 ConstructorCallExpr(NamedType(TokenKlass::K_float), {LiteralExpr(1)}));

        // Broadcast constructor
        CheckAst(MakeSourcePieces("vec4", "vec4(1)"),
                 ConstructorCallExpr(NamedType(TokenKlass::K_vec4),
                                     {
                                         ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float),
                                     }));

        // Swizzle constructor
        CheckAst(MakeSourcePieces("vec4", "vec4(1, ivec3(1))"),
                 ConstructorCallExpr(
                     NamedType(TokenKlass::K_vec4),
                     {
                         ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float),
                         ImplicitCastExpr(ConstructorCallExpr(NamedType(TokenKlass::K_ivec3), {LiteralExpr(1)}))
                             ->CheckType(GlslBuiltinType::Ty_vec3),
                     }));

        // CheckAst(MakeSourcePieces("vec4[]", "vec4[](ivec4(1))"),
        //                 ConstructorCallExpr(
        //                     NamedType(TokenKlass::K_vec4),
        //                     {
        //                         ImplicitCastExpr(LiteralExpr(1))->CheckType(checkFloatType),
        //                         ImplicitCastExpr(ConstructorCallExpr(NamedType(TokenKlass::K_ivec4),
        //                         {LiteralExpr(1)}))
        //                             ->CheckType(checkVec4Type),
        //                     }));
    }

    SECTION("Initializer")
    {
        SetTestTemplate("void main() {{ {} test__ = {} }}", [this](AstMatcher* matcher) {
            return FindMatch(
                FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                FunctionDecl(AnyQualType(), AnyTok(), {},
                             CompoundStmt({DeclStmt(VariableDecl(AnyQualType(), AnyTok(), AnyAst(), matcher))})));
        });

        CheckAst(MakeSourcePieces("float", "1"),
                 ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float));

        CheckAst(MakeSourcePieces("vec2", "{1, 2}"),
                 InitializerList({
                     ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float),
                     ImplicitCastExpr(LiteralExpr(2))->CheckType(GlslBuiltinType::Ty_float),
                 }));

        CheckAst(MakeSourcePieces("struct { float x; double y; }", "{1, 2}"),
                 InitializerList({
                     ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float),
                     ImplicitCastExpr(LiteralExpr(2))->CheckType(GlslBuiltinType::Ty_double),
                 }));

        CheckAst(MakeSourcePieces("vec3", "{1, true, 3}"),
                 InitializerList({
                     ImplicitCastExpr(LiteralExpr(1))->CheckType(GlslBuiltinType::Ty_float),
                     ImplicitCastExpr(LiteralExpr(true))->CheckType(GlslBuiltinType::Ty_float),
                     ImplicitCastExpr(LiteralExpr(3))->CheckType(GlslBuiltinType::Ty_float),
                 }));
    }
}