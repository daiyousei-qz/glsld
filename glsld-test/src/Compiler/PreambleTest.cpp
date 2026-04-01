#include "CompilerTestFixture.h"

using namespace glsld;

TEST_CASE_METHOD(CompilerTestFixture, "Compiler::PreambleTest")
{
    SECTION("Macro")
    {
        SourceTextView preambleText = R"(
            #define FOO 42
        )";
        SourceTextView mainFileText = R"(
            FOO
        )";

        //  Macros in the preamble should be visible in the main file
        auto result = CompileWithUserPreamble(preambleText, mainFileText, CompileMode::PreprocessOnly);
        CheckTokens(result->GetUserFileArtifacts().GetTokens(), {NumTok("42"), EofTok()});
    }

    SECTION("Variable")
    {
        SourceTextView preambleText = R"(
            const int foo = 42;
        )";
        SourceTextView mainFileText = R"(
            void main() {
                foo;
            }
        )";

        // Variables in the preamble should be visible in the main file
        auto result = CompileWithUserPreamble(preambleText, mainFileText, CompileMode::ParseOnly);
        CheckAst(result->GetUserPreambleArtifacts().GetAst(),
                 TranslationUnit({
                     VariableDecl(QualType(AnyAst(), KeywordTok(glsld::TokenKlass::K_int), NullAst()),
                                  {VariableDeclaratorDecl(IdTok("foo"), NullAst(), LiteralExpr(42))}),
                 }));
        CheckAst(result->GetUserFileArtifacts().GetAst(),
                 TranslationUnit({
                     FunctionDecl(AnyAst(), IdTok("main"), {},
                                  CompoundStmt({
                                      ExprStmt(NameAccessExpr("foo")
                                                   ->CheckType(GlslBuiltinType::Ty_int)
                                                   ->CheckValue([](const ConstValue& v) {
                                                       return v == ConstValue::CreateScalar(42);
                                                   })),
                                  })),
                 }));
    }

    SECTION("Function")
    {
        SourceTextView preambleText = R"(
            int foo() { return 42; }
        )";
        SourceTextView mainFileText = R"(
            void main() {
                foo();
            }
        )";

        // Functions in the preamble should be visible in the main file
        auto result = CompileWithUserPreamble(preambleText, mainFileText, CompileMode::ParseOnly);
        CheckAst(result->GetUserPreambleArtifacts().GetAst(),
                 TranslationUnit({
                     FunctionDecl(NamedType(TokenKlass::K_int), IdTok("foo"), {},
                                  CompoundStmt({ReturnStmt(LiteralExpr(42))})),
                 }));
        CheckAst(result->GetUserFileArtifacts().GetAst(),
                 TranslationUnit({
                     FunctionDecl(AnyAst(), IdTok("main"), {},
                                  CompoundStmt({
                                      ExprStmt(FunctionCallExpr("foo", {})->CheckType(GlslBuiltinType::Ty_int)),
                                  })),
                 }));
    }
}