#include "LanguageService.h"
#include "StandardDocumentation.h"
#include "ModuleVisitor.h"

namespace glsld
{
    class SignatureHelpVisitor : public ModuleVisitor<SignatureHelpVisitor>
    {
    private:
        TextPosition position;

        AstInvokeExpr* invokeExpr = nullptr;

    public:
        SignatureHelpVisitor(const LanguageQueryProvider& provider, TextPosition position)
            : ModuleVisitor(provider), position(position)
        {
        }

        auto Execute() -> AstInvokeExpr*
        {
            TraverseAllGlobalDecl();
            return invokeExpr;
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            if (GetProvider().ContainsPosition(node, position)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            invokeExpr = &expr;
        }
    };

    // FIXME: implement correctly. Currently this is a placeholder implmentation.
    auto ComputeSignatureHelp(const LanguageQueryProvider& provider, lsp::Position position)
        -> std::optional<lsp::SignatureHelp>
    {
        auto expr = SignatureHelpVisitor{provider, FromLspPosition(position)}.Execute();

        if (expr) {
            auto funcExpr = expr->GetInvokedExpr()->As<AstNameAccessExpr>();
            if (funcExpr && funcExpr->GetAccessType() == NameAccessType::Function) {
                auto funcName = funcExpr->GetAccessName().text.StrView();

                std::vector<lsp::SignatureInformation> result;

                // For function in the default library
                for (auto funcDecl : GetStandardLibraryModule()->GetAstContext().functionDecls) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetName().text.StrView() == funcName) {
                        std::string label;
                        ReconstructSourceText(label, *funcDecl);
                        std::string documentation = QueryFunctionDocumentation(funcName).Str();

                        result.push_back(lsp::SignatureInformation{
                            .label         = std::move(label),
                            .documentation = std::move(documentation),
                        });
                    }
                }

                // For function in this module
                for (auto funcDecl : provider.GetAstContext().functionDecls) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetName().text.StrView() == funcName) {
                        std::string label;
                        ReconstructSourceText(label, *funcDecl);

                        result.push_back(lsp::SignatureInformation{
                            .label         = std::string{label},
                            .documentation = fmt::format("```glsl\n{}\n```", label),
                        });
                    }
                }

                return lsp::SignatureHelp{
                    .signatures = std::move(result),
                };
            }
        }

        return std::nullopt;
    }

} // namespace glsld