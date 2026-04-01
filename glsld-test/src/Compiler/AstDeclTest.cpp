#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::AstDeclTest")
{
    SetTestTemplate("{}", [this](AstMatcher* matcher) { return TranslationUnit({matcher}); });

    SECTION("Empty Decl")
    {
        CheckAst(";", EmptyDecl());
    }

    SECTION("Precision Decl")
    {
        CheckAst("precision highp int;", PrecisionDecl(QualType(NamedQual({TokenKlass::K_highp}),
                                                                KeywordTok(glsld::TokenKlass::K_int), NullAst())));
        CheckAst("precision mediump float;",
                 PrecisionDecl(
                     QualType(NamedQual({TokenKlass::K_mediump}), KeywordTok(glsld::TokenKlass::K_float), NullAst())));
        CheckAst("precision lowp vec3;", PrecisionDecl(QualType(NamedQual({TokenKlass::K_lowp}),
                                                                KeywordTok(glsld::TokenKlass::K_vec3), NullAst())));

        SECTION("Permissive")
        {
            CheckAst("precision highp int", PrecisionDecl(QualType(NamedQual({TokenKlass::K_highp}),
                                                                   KeywordTok(glsld::TokenKlass::K_int), NullAst())));

            // TODO: check error?
            // CheckAst("precision highp mediump lowp int;",
            //          PrecisionDecl(
            //              QualType(NamedQual({TokenKlass::K_highp}), KeywordTok(glsld::TokenKlass::K_int),
            //              NullAst())));
        }
    }

    SECTION("Global Qualifier Decl")
    {
        CheckAst("layout(local_size_x = 1, local_size_y = 2, local_size_z = 3) in;",
                 GlobalQualifierDecl(NamedQual({TokenKlass::K_in}, {{IdTok("local_size_x"), LiteralExpr(1)},
                                                                    {IdTok("local_size_y"), LiteralExpr(2)},
                                                                    {IdTok("local_size_z"), LiteralExpr(3)}})));

        CheckAst("layout(triangle) out;",
                 GlobalQualifierDecl(NamedQual({TokenKlass::K_out}, {{IdTok("triangle"), NullAst()}})));

        SECTION("Permissive")
        {

            // TODO: could this be inferred?
            // CheckAst("layout(triangle) out",
            //          GlobalQualifierDecl(NamedQual({TokenKlass::K_out}, {{"triangle", NullAst()}})));
        }
    }

    SECTION("Type Qualifier Override Decl")
    {
        CheckAst("invariant gl_Position;",
                 TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_invariant}), {IdTok("gl_Position")}));

        CheckAst("precise gl_FragColor, gl_FragData;",
                 TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_precise}),
                                           {IdTok("gl_FragColor"), IdTok("gl_FragData")}));

        SECTION("Permissive")
        {
            CheckAst("invariant gl_Position",
                     TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_invariant}), {IdTok("gl_Position")}));

            CheckAst("invariant gl_Position,",
                     TypeQualifierOverrideDecl(NamedQual({TokenKlass::K_invariant}), {IdTok("gl_Position")}));
        }
    }

    SECTION("Interface Decl")
    {
        CheckAst("uniform BLOCK {};", BlockDecl(NamedQual({TokenKlass::K_uniform}), IdTok("BLOCK"), {}));
        CheckAst("buffer BLOCK { int a; };",
                 BlockDecl(NamedQual({TokenKlass::K_buffer}), IdTok("BLOCK"),
                           {BlockFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}));

        SECTION("Permissive")
        {
            CheckAst("uniform BLOCK {}", BlockDecl(NamedQual({TokenKlass::K_uniform}), IdTok("BLOCK"), {}));
            CheckAst("uniform {};", BlockDecl(NamedQual({TokenKlass::K_uniform}), InvalidTok(), {}));
            CheckAst("uniform {}", BlockDecl(NamedQual({TokenKlass::K_uniform}), InvalidTok(), {}));

            // This is invalid GLSL, but we parse it as interface block
            CheckAst("layout() BLOCK {};", BlockDecl(NamedQual({}), IdTok("BLOCK"), {}));
        }
    }

    SECTION("Struct Decl")
    {
        SetTestTemplate(
            "{}", [this](AstMatcher* matcher) { return TranslationUnit({VariableDecl(StructType(matcher), {})}); });

        CheckAst("struct A {};", StructDecl(IdTok("A"), {}));
        CheckAst("struct A { int a; };",
                 StructDecl(IdTok("A"), {StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}));
        CheckAst("struct A { int a; float b; };",
                 StructDecl(IdTok("A"), {
                                            StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
                                            StructFieldDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
                                        }));
        CheckAst("struct A { int a1, a2[2]; };",
                 StructDecl(IdTok("A"),
                            {StructFieldDecl(NamedType(TokenKlass::K_int),
                                             {
                                                 StructFieldDeclaratorDecl(IdTok("a1"), NullAst()),
                                                 StructFieldDeclaratorDecl(IdTok("a2"), ArraySpec({LiteralExpr(2)})),
                                             })}));
        CheckAst("struct A { mat4[2] m; vec3 pos; };",
                 StructDecl(IdTok("A"), {
                                            StructFieldDecl(QualType(NullAst(), KeywordTok(TokenKlass::K_mat4),
                                                                     ArraySpec({LiteralExpr(2)})),
                                                            IdTok("m"), NullAst()),
                                            StructFieldDecl(NamedType(TokenKlass::K_vec3), IdTok("pos"), NullAst()),
                                        }));
        CheckAst("struct B { highp float value; };",
                 StructDecl(IdTok("B"), {StructFieldDecl(QualType(NamedQual({TokenKlass::K_highp}),
                                                                  KeywordTok(TokenKlass::K_float), NullAst()),
                                                         IdTok("value"), NullAst())}));

        SECTION("Permissive")
        {
            // FIXME: add the following tests
            // CheckAst("struct;", StructDecl(InvalidTok(), {}));
            // CheckAst("struct A;", StructDecl(IdTok("A"), {}));

            // ';' after the struct should be inferred
            CheckAst("struct {}", StructDecl(InvalidTok(), {}));

            // '}' to close the struct should be inferred
            CheckAst("struct {;", StructDecl(InvalidTok(), {}));

            // ';' after the struct field should be inferred
            CheckAst("struct A { int a float b };",
                     StructDecl(IdTok("A"), {
                                                StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
                                                StructFieldDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
                                            }));

            // Notably, the struct field cannot have initializer, so it shouldn't be parsed into the AST
            CheckAst("struct A { int a = 41; }",
                     StructDecl(IdTok("A"), {StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}));

            // FIXME: parser should be able to handle this
            // CheckAst("struct A { int a +++ ---; float b; };",
            //          StructDecl(IdTok("A"),
            //                     {
            //                         StructFieldDeclaratorDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
            //                         StructFieldDeclaratorDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
            //                     }));
        }
    }

    SECTION("Function Decl")
    {
        CheckAst("void foo();", FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"), {}, NullAst()));
        CheckAst("void foo() {}", FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"), {}, CompoundStmt({})));

        CheckAst("void foo(int a);",
                 FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                              {ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst())}, NullAst()));
        CheckAst("void foo(int a[2]);",
                 FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                              {ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}))},
                              NullAst()));
        CheckAst("void foo(int);",
                 FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                              {ParamDecl(NamedType(TokenKlass::K_int), InvalidTok(), NullAst())}, NullAst()));

        CheckAst("void foo(int a, float b);",
                 FunctionDecl(NamedType(TokenKlass::K_void), IdTok("foo"),
                              {
                                  ParamDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
                                  ParamDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
                              },
                              NullAst()));

        CheckAst(
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
        CheckAst("highp float foo();",
                 FunctionDecl(QualType(NamedQual({TokenKlass::K_highp}), KeywordTok(TokenKlass::K_float), NullAst()),
                              IdTok("foo"), {}, NullAst()));
        CheckAst("void foo(int a[2][3], vec3 b);",
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
        CheckAst("int a;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
        CheckAst("int a[2];",
                 VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}), NullAst()));
        CheckAst("int a[2u];",
                 VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"),
                              ArraySpec({ImplicitCastExpr(LiteralExpr(2u))->CheckType(GlslBuiltinType::Ty_int)}),
                              NullAst()));
        CheckAst("int a = 1;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), LiteralExpr(1)));
        CheckAst("int a[2] = {1, 2};",
                 VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}),
                              InitializerList({
                                  LiteralExpr(1),
                                  LiteralExpr(2),
                              })));
        CheckAst("const int a = 1;",
                 VariableDecl(QualType(NamedQual({TokenKlass::K_const}), KeywordTok(TokenKlass::K_int), NullAst()),
                              IdTok("a"), NullAst(), LiteralExpr(1)));
        CheckAst("int a[2][3] = {{1, 2, 3}, {4, 5, 6}};",
                 VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2), LiteralExpr(3)}),
                              InitializerList({
                                  InitializerList({LiteralExpr(1), LiteralExpr(2), LiteralExpr(3)}),
                                  InitializerList({LiteralExpr(4), LiteralExpr(5), LiteralExpr(6)}),
                              })));

        SECTION("Permissive")
        {
            CheckAst("int a", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
            CheckAst("int;", VariableDecl(NamedType(TokenKlass::K_int), {}));
        }
    }

    SECTION("Local Variable Decl")
    {
        SetTestTemplate("struct StructType {{ int x; }}; void main() {{ {} }}", [this](AstMatcher* matcher) {
            return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                             FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({DeclStmt(matcher)})));
        });

        CheckAst("int a;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
        CheckAst("int a = 42;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), LiteralExpr(42)));
        CheckAst("int a[2];",
                 VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}), NullAst()));
        CheckAst("int a[2] = {1, 2};",
                 VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}),
                              InitializerList({
                                  LiteralExpr(1),
                                  LiteralExpr(2),
                              })));
        CheckAst("int[2] a;",
                 VariableDecl(QualType(NullAst(), KeywordTok(TokenKlass::K_int), ArraySpec({LiteralExpr(2)})),
                              IdTok("a"), NullAst(), NullAst()));
        CheckAst("int[2] a = {1, 2};",
                 VariableDecl(QualType(NullAst(), KeywordTok(TokenKlass::K_int), ArraySpec({LiteralExpr(2)})),
                              IdTok("a"), NullAst(),
                              InitializerList({
                                  LiteralExpr(1),
                                  LiteralExpr(2),
                              })));
        CheckAst("const int a = 41;",
                 VariableDecl(QualType(NamedQual({TokenKlass::K_const}), KeywordTok(TokenKlass::K_int), NullAst()),
                              IdTok("a"), NullAst(), LiteralExpr(41)));
        CheckAst("int a[2][3];", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"),
                                              ArraySpec({LiteralExpr(2), LiteralExpr(3)}), NullAst()));

        CheckAst("StructType a;", VariableDecl(NamedType("StructType"), IdTok("a"), NullAst(), NullAst()));
        CheckAst("StructType[2] a;", VariableDecl(QualType(NullAst(), IdTok("StructType"), ArraySpec({LiteralExpr(2)})),
                                                  IdTok("a"), NullAst(), NullAst()));

        SECTION("Permissive")
        {
            CheckAst("int a", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
            CheckAst("int;", VariableDecl(NamedType(TokenKlass::K_int), {}));
        }
    }
}
