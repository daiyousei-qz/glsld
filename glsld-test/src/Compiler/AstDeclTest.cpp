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
        // FIXME: support this test
        GLSLD_CHECK_AST("precision highp int;", PrecisionDecl(AnyQualType()));
    }

    SECTION("Block Decl")
    {
        GLSLD_CHECK_AST("uniform BLOCK {};", BlockDecl(UniformQual(), IdTok("BLOCK"), {}));
        GLSLD_CHECK_AST(
            "uniform BLOCK { int a; };",
            BlockDecl(UniformQual(), IdTok("BLOCK"), {BlockFieldDecl(NamedType(TokenKlass::K_int), IdTok("a"))}));
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

        SECTION("Permissive")
        {
            // FIXME: add the following tests
            // GLSLD_CHECK_AST("struct;", StructDecl(InvalidTok(), {}));
            // GLSLD_CHECK_AST("struct A;", StructDecl(IdTok("A"), {}));
            GLSLD_CHECK_AST("struct {}", StructDecl(InvalidTok(), {}));
            GLSLD_CHECK_AST("struct {;", StructDecl(InvalidTok(), {}));
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
