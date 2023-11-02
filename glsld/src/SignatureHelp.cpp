#include "LanguageService.h"
#include "StandardDocumentation.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    class SignatureHelpVisitor : public LanguageQueryVisitor<SignatureHelpVisitor>
    {
    private:
        TextPosition position;

        const AstFunctionCallExpr* functionCallExpr = nullptr;

    public:
        SignatureHelpVisitor(const LanguageQueryProvider& provider, TextPosition position)
            : LanguageQueryVisitor(provider), position(position)
        {
        }

        auto Execute() -> const AstFunctionCallExpr*
        {
            TraverseTranslationUnit();
            return functionCallExpr;
        }

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy
        {
            return TraverseNodeContains(node, position);
        }

        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
        {
            functionCallExpr = &expr;
        }
    };

    // FIXME: implement correctly. Currently this is a placeholder implmentation.
    auto ComputeSignatureHelp(const LanguageQueryProvider& provider, lsp::Position position)
        -> std::optional<lsp::SignatureHelp>
    {
        auto expr = SignatureHelpVisitor{provider, FromLspPosition(position)}.Execute();

        if (expr) {
            auto funcName = expr->GetFunctionName().text.StrView();

            std::vector<lsp::SignatureInformation> result;

            // For function in the default library
            for (auto decl : GetStandardLibraryModule()->GetAstContext().GetTranslationUnit()->GetGlobalDecls()) {
                if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetDeclTok().text.StrView() == funcName) {
                        std::string label;
                        ReconstructSourceText(label, *funcDecl);
                        std::string documentation = QueryFunctionDocumentation(funcName).Str();

                        result.push_back(lsp::SignatureInformation{
                            .label         = std::move(label),
                            .documentation = std::move(documentation),
                        });
                    }
                }
            }

            // For function in this module
            for (auto decl : provider.GetAstContext().GetTranslationUnit()->GetGlobalDecls()) {
                if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetDeclTok().text.StrView() == funcName) {
                        std::string label;
                        ReconstructSourceText(label, *funcDecl);

                        result.push_back(lsp::SignatureInformation{
                            .label         = std::string{label},
                            .documentation = fmt::format("```glsl\n{}\n```", label),
                        });
                    }
                }
            }

            return lsp::SignatureHelp{
                .signatures = std::move(result),
            };
        }

        return std::nullopt;
    }

} // namespace glsld