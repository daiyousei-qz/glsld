#include "Feature/SignatureHelp.h"
#include "Support/SourceText.h"
#include "Server/StandardDocumentation.h"
#include "Server/LanguageQueryVisitor.h"

namespace glsld
{
    class SignatureHelpVisitor : public LanguageQueryVisitor<SignatureHelpVisitor>
    {
    private:
        TextPosition position;

        const AstFunctionCallExpr* functionCallExpr = nullptr;

    public:
        SignatureHelpVisitor(const LanguageQueryInfo& info, TextPosition position)
            : LanguageQueryVisitor(info), position(position)
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

    auto GetSignatureHelpOptions(const SignatureHelpConfig& config) -> std::optional<lsp::SignatureHelpOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::SignatureHelpOptions{
            .triggerCharacters   = {"("},
            .retriggerCharacters = {","},
        };
    }

    // FIXME: implement correctly. Currently this is a placeholder implementation.
    auto HandleSignatureHelp(const SignatureHelpConfig& config, const LanguageQueryInfo& info,
                             const lsp::SignatureHelpParams& params) -> std::optional<lsp::SignatureHelp>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        auto expr = SignatureHelpVisitor{info, FromLspPosition(params.position)}.Execute();

        if (expr) {
            auto funcName = expr->GetNameToken().text.StrView();

            std::vector<lsp::SignatureInformation> result;

            // For function in the default library
            // FIXME: handle user preamble
            for (auto decl : GetStdlibModule()->GetSystemPreambleArtifacts().GetAst()->GetGlobalDecls()) {
                if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetNameToken().text.StrView() == funcName) {
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
            for (auto decl : info.GetUserFileAst().GetGlobalDecls()) {
                if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetNameToken().text.StrView() == funcName) {
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