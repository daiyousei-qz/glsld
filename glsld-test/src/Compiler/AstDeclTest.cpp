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

    SECTION("Interface Decl")
    {
        GLSLD_CHECK_AST("uniform BLOCK {};", BlockDecl(NamedQual({TokenKlass::K_uniform}), IdTok("BLOCK"), {}));
        GLSLD_CHECK_AST("uniform BLOCK { int a; };",
                        BlockDecl(NamedQual({TokenKlass::K_uniform}), IdTok("BLOCK"),
                                  {BlockFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"))}));
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
                       {
                           StructFieldDecl(NamedType(TokenKlass::K_int),
                                           {
                                               DeclaratorMatcher{IdTok("a1"), NullAst(), NullAst()},
                                               DeclaratorMatcher{IdTok("a2"), ArraySpec({LiteralExpr(2)}), NullAst()},
                                           }),
                       }));

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
            //                                StructFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst()),
            //                                StructFieldDecl(NamedType(TokenKlass::K_float), IdTok("b"), NullAst()),
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
    }

    SECTION("Global Variable Decl")
    {
        GLSLD_CHECK_AST("int a;", VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), NullAst()));
        GLSLD_CHECK_AST("int a[2];",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}), NullAst()));
        GLSLD_CHECK_AST("int a = 1;",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), NullAst(), LiteralExpr(1)));
        GLSLD_CHECK_AST("int a[2] = {1, 2};",
                        VariableDecl(NamedType(TokenKlass::K_int), IdTok("a"), ArraySpec({LiteralExpr(2)}),
                                     InitializerList({
                                         LiteralExpr(1),
                                         LiteralExpr(2),
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
