#include "AstTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(AstTestFixture, "Simple Global Decl")
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

    SECTION("Variable Decl")
    {
        GLSLD_CHECK_AST("int a;", VariableDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), NullAst(), NullAst()));
        GLSLD_CHECK_AST("int a[2];", VariableDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"),
                                                  ArraySpec({LiteralExpr(2)}), NullAst()));
        GLSLD_CHECK_AST("int a = 1;",
                        VariableDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), NullAst(), LiteralExpr(1)));
        GLSLD_CHECK_AST("int a[2] = {1, 2};",
                        VariableDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), ArraySpec({LiteralExpr(2)}),
                                     InitializerList({
                                         LiteralExpr(1),
                                         LiteralExpr(2),
                                     })));

        SECTION("Permissive")
        {
            GLSLD_CHECK_AST("int;", VariableDecl(BuiltinType(GlslBuiltinType::Ty_int), {}));
        }
    }

    SECTION("Type Decl")
    {
        SetTestTemplate("{}", [this](AstMatcher* matcher) { return TranslationUnit({VariableDecl(matcher, {})}); });

        GLSLD_CHECK_AST("struct A {};", StructDecl(IdTok("A"), {}));
        GLSLD_CHECK_AST("struct A { int a; };", StructDecl(IdTok("A"), {FieldDecl(BuiltinType(GlslBuiltinType::Ty_int),
                                                                                  IdTok("a"), NullAst())}));
        GLSLD_CHECK_AST(
            "struct A { int a; float b; };",
            StructDecl(IdTok("A"), {
                                       FieldDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), NullAst()),
                                       FieldDecl(BuiltinType(GlslBuiltinType::Ty_float), IdTok("b"), NullAst()),
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
        GLSLD_CHECK_AST("void foo();",
                        FunctionDecl(BuiltinType(GlslBuiltinType::Ty_void), IdTok("foo"), {}, NullAst()));
        GLSLD_CHECK_AST("void foo() {}",
                        FunctionDecl(BuiltinType(GlslBuiltinType::Ty_void), IdTok("foo"), {}, CompoundStmt({})));

        GLSLD_CHECK_AST("void foo(int a);",
                        FunctionDecl(BuiltinType(GlslBuiltinType::Ty_void), IdTok("foo"),
                                     {ParamDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), NullAst())},
                                     NullAst()));
        GLSLD_CHECK_AST("void foo(int a[2]);", FunctionDecl(BuiltinType(GlslBuiltinType::Ty_void), IdTok("foo"),
                                                            {ParamDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"),
                                                                       ArraySpec({LiteralExpr(2)}))},
                                                            NullAst()));
        GLSLD_CHECK_AST("void foo(int);",
                        FunctionDecl(BuiltinType(GlslBuiltinType::Ty_void), IdTok("foo"),
                                     {ParamDecl(BuiltinType(GlslBuiltinType::Ty_int), InvalidTok(), NullAst())},
                                     NullAst()));

        GLSLD_CHECK_AST("void foo(int a, float b);",
                        FunctionDecl(BuiltinType(GlslBuiltinType::Ty_void), IdTok("foo"),
                                     {
                                         ParamDecl(BuiltinType(GlslBuiltinType::Ty_int), IdTok("a"), NullAst()),
                                         ParamDecl(BuiltinType(GlslBuiltinType::Ty_float), IdTok("b"), NullAst()),
                                     },
                                     NullAst()));
    }
}

TEST_CASE_METHOD(AstTestFixture, "Simple Local Decl")
{
    SetTestTemplate("void main() {{ {} }}", [this](AstMatcher* matcher) {
        return FindMatch(FunctionDecl(AnyQualType(), IdTok("main"), {}, AnyStmt()),
                         FunctionDecl(AnyQualType(), AnyTok(), {}, CompoundStmt({DeclStmt(matcher)})));
    });
}