#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "AstDeclTest")
{
    SetTestTemplate("{}", [this](AstMatcher* matcher) { return TranslationUnit({matcher}); });

    SECTION("Empty Decl")
    {
        GLSLD_CHECK_AST(";", EmptyDecl());
    }

    SECTION("Precision Decl")
    {
        GLSLD_CHECK_AST(
            "precision highp int;",
            PrecisionDecl(QualType(NamedQual({TokenKlass::K_highp}), KeywordTok(glsld::TokenKlass::K_int), NullAst())));
        GLSLD_CHECK_AST("precision mediump float;",
                        PrecisionDecl(QualType(NamedQual({TokenKlass::K_mediump}),
                                               KeywordTok(glsld::TokenKlass::K_float), NullAst())));
        GLSLD_CHECK_AST(
            "precision lowp vec3;",
            PrecisionDecl(QualType(NamedQual({TokenKlass::K_lowp}), KeywordTok(glsld::TokenKlass::K_vec3), NullAst())));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("precision highp int",
                            PrecisionDecl(QualType(NamedQual({TokenKlass::K_highp}),
                                                   KeywordTok(glsld::TokenKlass::K_int), NullAst())));

            // TODO: check error?
            // GLSLD_CHECK_AST("precision highp mediump lowp int;",
            //                 PrecisionDecl(QualType(NamedQual({TokenKlass::K_highp}),
            //                                        KeywordTok(glsld::TokenKlass::K_int), NullAst())));
        }
    }

    SECTION("Global Qualifier Decl")
    {
        GLSLD_CHECK_AST("layout(local_size_x = 1, local_size_y = 2, local_size_z = 3) in;",
                        GlobalQualifierDecl(NamedQual({TokenKlass::K_in}, {{IdTok("local_size_x"), LiteralExpr(1)},
                                                                           {IdTok("local_size_y"), LiteralExpr(2)},
                                                                           {IdTok("local_size_z"), LiteralExpr(3)}})));

        GLSLD_CHECK_AST("layout(triangle) out;",
                        GlobalQualifierDecl(NamedQual({TokenKlass::K_out}, {{IdTok("triangle"), NullAst()}})));

        SECTION("Permissive")
        {

            // TODO: could this be inferred?
            // GLSLD_CHECK_AST("layout(triangle) out",
            //                 GlobalQualifierDecl(NamedQual({TokenKlass::K_out}, {{"triangle", NullAst()}})));
        }
    }

    SECTION("Type Qualifier Override Decl")
    {
        GLSLD_CHECK_AST("invariant gl_Position;",
                        TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_invariant}), {IdTok("gl_Position")}));

        GLSLD_CHECK_AST("precise gl_FragColor, gl_FragData;",
                        TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_precise}),
                                                  {IdTok("gl_FragColor"), IdTok("gl_FragData")}));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("invariant gl_Position",
                            TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_invariant}), {IdTok("gl_Position")}));

            GLSLD_CHECK_AST("invariant gl_Position,",
                            TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_invariant}), {IdTok("gl_Position")}));
        }
    }

    SECTION("Interface Decl")
    {
        GLSLD_CHECK_AST("uniform BLOCK {};", BlockDecl(NamedQual({TokenKlass::K_uniform}), IdTok("BLOCK"), {}));
        GLSLD_CHECK_AST("buffer BLOCK { int a; };",
                        BlockDecl(NamedQual({TokenKlass::K_buffer}), IdTok("BLOCK"),
                                  {BlockFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("uniform BLOCK {}", BlockDecl(NamedQual({TokenKlass::K_uniform}), IdTok("BLOCK"), {}));
            GLSLD_CHECK_AST("uniform {};", BlockDecl(NamedQual({TokenKlass::K_uniform}), InvalidTok(), {}));
            GLSLD_CHECK_AST("uniform {}", BlockDecl(NamedQual({TokenKlass::K_uniform}), InvalidTok(), {}));

            // This is invalid GLSL, but we parse it as interface block
            GLSLD_CHECK_AST("layout() BLOCK {};", BlockDecl(NamedQual({}), IdTok("BLOCK"), {}));
        }
    }

    SECTION("Struct Decl")
    {
        SetTestTemplate(
            "{}", [this](AstMatcher* matcher) { return TranslationUnit({VariableDecl(StructType(matcher), {})}); });

        GLSLD_CHECK_AST("struct A {};", StructDecl(IdTok("A"), {}));
        GLSLD_CHECK_AST("struct A { int a; };",
                        StructDecl(IdTok("A"), {StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}));
        GLSLD_CHECK_AST(
            "struct A { int a; float b; };",
            StructDecl(IdTok("A"), {
                                       StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
                                       StructFieldDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
                                   }));
        GLSLD_CHECK_AST(
            "struct A { int a1, a2[2]; };",
            StructDecl(IdTok("A"),
                       {StructFieldDecl(NamedType(TokenKlass::K_int),
                                        {
                                            StructFieldDeclaratorDecl(IdTok("a1"), NullAst()),
                                            StructFieldDeclaratorDecl(IdTok("a2"), ArraySpec({LiteralExpr(2)})),
                                        })}));
        GLSLD_CHECK_AST(
            "struct A { mat4[2] m; vec3 pos; };",
            StructDecl(IdTok("A"), {
                                       StructFieldDecl(QualType(NullAst(), KeywordTok(TokenKlass::K_mat4),
                                                                ArraySpec({LiteralExpr(2)})),
                                                       IdTok("m"), NullAst()),
                                       StructFieldDecl(NamedType(TokenKlass::K_vec3), IdTok("pos"), NullAst()),
                                   }));
        GLSLD_CHECK_AST("struct B { highp float value; };",
                        StructDecl(IdTok("B"), {StructFieldDecl(QualType(NamedQual({TokenKlass::K_highp}),
                                                                         KeywordTok(TokenKlass::K_float), NullAst()),
                                                                IdTok("value"), NullAst())}));

        SECTION("Permissive")
        {
            // FIXME: add the following tests
            // GLSLD_CHECK_AST("struct;", StructDecl(InvalidTok(), {}));
            // GLSLD_CHECK_AST("struct A;", StructDecl(IdTok("A"), {}));

            // ';' after the struct should be inferred
            GLSLD_CHECK_AST("struct {}", StructDecl(InvalidTok(), {}));

            // '}' to close the struct should be inferred
            GLSLD_CHECK_AST("struct {;", StructDecl(InvalidTok(), {}));

            // ';' after the struct field should be inferred
            GLSLD_CHECK_AST(
                "struct A { int a float b };",
                StructDecl(IdTok("A"), {
                                           StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
                                           StructFieldDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
                                       }));

            // Notably, the struct field cannot have initializer, so it shouldn't be parsed into the AST
            GLSLD_CHECK_AST(
                "struct A { int a = 41; }",
                StructDecl(IdTok("A"), {StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}));

            // FIXME: parser should be able to handle this
            // GLSLD_CHECK_AST(
            //     "struct A { int a +++ ---; float b; };",
            //     StructDecl(IdTok("A"), {
            //                                StructFieldDeclaratorDecl(NamedType(TokenKlass::K_int), IdTok("a"),
            //                                NullAst()), StructFieldDeclaratorDecl(NamedType(TokenKlass::K_float),
            //                                IdTok("b"), NullAst()),
            //                            }));
        }
    }

    SECTION("Function Decl")
    {
        GLSLD_CHECK_AST("void foo();", FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"), {}, NullAst()));
        GLSLD_CHECK_AST("void foo() {}",
                        FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"), {}, CompoundStmt({})));

        GLSLD_CHECK_AST("void foo(int a);",
                        FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                                     {ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}, NullAst()));
        GLSLD_CHECK_AST("void foo(int a[2]);",
                        FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                                     {ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}))},
                                     NullAst()));
        GLSLD_CHECK_AST("void foo(int);",
                        FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                                     {ParamDecl(NamedType(TokenKlass::K_int), InvalidTok(), NullAst())}, NullAst()));

        GLSLD_CHECK_AST("void foo(int a, float b);",
                        FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                                     {
                                         ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
                                         ParamDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
                                     },
                                     NullAst()));

        GLSLD_CHECK_AST(
            "void foo(in int a, out float b, inout double c, const vec3 d);",
            FunctionDecl(
                NamedType(TokenKlass::K_void), IdTok("foo"),
                {
                    ParamDecl(QualType(NamedQual({TokenKlass::K_in}), KeywordTok(TokenKlass::K_int), NullAst()),
                              IdTok("a"), NullAst()),
                    ParamDecl(QualType(NamedQual({TokenKlass::K_out}), KeywordTok(TokenKlass::K_float), NullAst()),
                              IdTok("b"), NullAst()),
                    ParamDecl(QualType(NamedQual({TokenKlass::K_inout}), KeywordTok(TokenKlass::K_double), NullAst()),
                              IdTok("c"), NullAst()),
                    ParamDecl(QualType(NamedQual({TokenKlass::K_const}), KeywordTok(TokenKlass::K_vec3), NullAst()),
                              IdTok("d"), NullAst()),
                },
                NullAst()));
        GLSLD_CHECK_AST("highp float foo();", FunctionDecl(QualType(NamedQual({TokenKlass::K_highp}),
                                                                    KeywordTok(TokenKlass::K_float), NullAst()),
                                                           IdTok("foo"), {}, NullAst()));
        GLSLD_CHECK_AST("void foo(int a[2][3], vec3 b);",
                        FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                                     {
                                         ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"),
                                                   ArraySpec({LiteralExpr(2), LiteralExpr(3)})),
                                         ParamDecl(NamedType(TokenKlass::K_vec3), IdTok("b"), NullAst()),
                                     },
                                     NullAst()));
    }

    SECTION("Global Variable Decl")
    {
        GLSLD_CHECK_AST("int a;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
        GLSLD_CHECK_AST("int a[2];",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}), NullAst()));
        GLSLD_CHECK_AST("int a[2u];",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"),
                                     ArraySpec({ImplicitCastExpr(LiteralExpr(2u))->CheckType(GlslBuiltinType::Ty_int)}),
                                     NullAst()));
        GLSLD_CHECK_AST("int a = 1;",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), LiteralExpr(1)));
        GLSLD_CHECK_AST("int a[2] = {1, 2};",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}),
                                     InitializerList({
                                         LiteralExpr(1),
                                         LiteralExpr(2),
                                     })));
        GLSLD_CHECK_AST("const int a = 1;", VariableDecl(QualType(NamedQual({TokenKlass::K_const}),
                                                                  KeywordTok(TokenKlass::K_int), NullAst()),
                                                         IdTok("a"), NullAst(), LiteralExpr(1)));
        GLSLD_CHECK_AST("int a[2][3] = {{1, 2, 3}, {4, 5, 6}};",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"),
                                     ArraySpec({LiteralExpr(2), LiteralExpr(3)}),
                                     InitializerList({
                                         InitializerList({LiteralExpr(1), LiteralExpr(2), LiteralExpr(3)}),
                                         InitializerList({LiteralExpr(4), LiteralExpr(5), LiteralExpr(6)}),
                                     })));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("int a", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
            GLSLD_CHECK_AST("int;", VariableDecl(NamedType(TokenKlass::K_int), {}));
        }
    }

    SECTION("Local Variable Decl")
    {
        SetTestTemplate("struct StructType {{ int x; }}; void main() {{ {} }}", [this](AstMatcher* matcher) {
            return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                             FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({DeclStmt(matcher)})));
        });

        GLSLD_CHECK_AST("int a;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
        GLSLD_CHECK_AST("int a = 42;",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), LiteralExpr(42)));
        GLSLD_CHECK_AST("int a[2];",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}), NullAst()));
        GLSLD_CHECK_AST("int a[2] = {1, 2};",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}),
                                     InitializerList({
                                         LiteralExpr(1),
                                         LiteralExpr(2),
                                     })));
        GLSLD_CHECK_AST("int[2] a;",
                        VariableDecl(QualType(NullAst(), KeywordTok(TokenKlass::K_int), ArraySpec({LiteralExpr(2)})),
                                     IdTok("a"), NullAst(), NullAst()));
        GLSLD_CHECK_AST("int[2] a = {1, 2};",
                        VariableDecl(QualType(NullAst(), KeywordTok(TokenKlass::K_int), ArraySpec({LiteralExpr(2)})),
                                     IdTok("a"), NullAst(),
                                     InitializerList({
                                         LiteralExpr(1),
                                         LiteralExpr(2),
                                     })));
        GLSLD_CHECK_AST("const int a = 41;", VariableDecl(QualType(NamedQual({TokenKlass::K_const}),
                                                                   KeywordTok(TokenKlass::K_int), NullAst()),
                                                          IdTok("a"), NullAst(), LiteralExpr(41)));
        GLSLD_CHECK_AST("int a[2][3];", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"),
                                                     ArraySpec({LiteralExpr(2), LiteralExpr(3)}), NullAst()));

        GLSLD_CHECK_AST("StructType a;", VariableDecl(NamedType("StructType"), IdTok("a"), NullAst(), NullAst()));
        GLSLD_CHECK_AST("StructType[2] a;",
                        VariableDecl(QualType(NullAst(), IdTok("StructType"), ArraySpec({LiteralExpr(2)})), IdTok("a"),
                                     NullAst(), NullAst()));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("int a", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
            GLSLD_CHECK_AST("int;", VariableDecl(NamedType(TokenKlass::K_int), {}));
        }
    }
}
