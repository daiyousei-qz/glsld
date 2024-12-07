#pragma once

#include "AstMatcher.h"

#include "Basic/StringView.h"
#include "Compiler/CompilerInvocation.h"

namespace glsld
{
    class TestFailure
    {
    private:
        std::string reason;

    public:
        TestFailure() = default;
        TestFailure(std::string reason) : reason(std::move(reason))
        {
        }
    };

    class AstExprTest
    {
    private:
        std::string sourceText;
        AstMatcher matchExpr;

    public:
        AstExprTest(StringView sourceText, AstMatcher matchExpr) : matchExpr(std::move(matchExpr))
        {
            this->sourceText = std::format("unknown x = {};", sourceText.StdStrView());
        }

        auto Positive() const -> bool
        {
            std::unique_ptr<CompilerInvocation> compiler = std::make_unique<CompilerInvocation>();
            compiler->SetNoStdlib(true);
            compiler->SetMainFileFromBuffer(sourceText);
            auto compilerResult = compiler->CompileMainFile(nullptr, CompileMode::ParseOnly);

            auto ast = compilerResult->GetUserFileAst();
            if (!ast || ast->GetGlobalDecls().size() != 1) {
                return false;
            }

            auto decl = ast->GetGlobalDecls()[0]->As<AstVariableDecl>();
            if (!decl || decl->GetDeclarators().size() != 1) {
                return false;
            }

            auto init = decl->GetDeclarators()[0].initializer;
            if (!init) {
                return false;
            }

            return matchExpr.Match(*init);
        }
    };
} // namespace glsld