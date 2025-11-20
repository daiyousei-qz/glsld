#include "Feature/SignatureHelp.h"
#include "Support/SourceReconstruction.h"
#include "Support/SourceText.h"
#include "Server/StandardDocumentation.h"
#include "Server/LanguageQueryVisitor.h"

#include <unordered_map>

namespace glsld
{
    class SignatureHelpLocatingVisitor
        : public LanguageQueryVisitor<SignatureHelpLocatingVisitor, const AstFunctionCallExpr*>
    {
    private:
        TextPosition position;

        const AstFunctionCallExpr* functionCallExpr = nullptr;

    public:
        SignatureHelpLocatingVisitor(const LanguageQueryInfo& info, TextPosition position)
            : LanguageQueryVisitor(info), position(position)
        {
        }

        auto Finish() -> const AstFunctionCallExpr* GLSLD_AST_VISITOR_OVERRIDE
        {
            return functionCallExpr;
        }

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy GLSLD_AST_VISITOR_OVERRIDE
        {
            return TraverseNodeContains(node, position);
        }

        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
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

    auto ComputeSignatureHelpPreambleInfo(const PrecompiledPreamble& preamble)
        -> std::unique_ptr<SignatureHelpPreambleInfo>
    {
        std::unordered_multimap<AtomString, const AstFunctionDecl*> builtinFunctionDeclMap;
        for (auto decl : preamble.GetSystemPreambleArtifacts().GetAst()->GetGlobalDecls()) {
            if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                builtinFunctionDeclMap.emplace(funcDecl->GetNameToken().text, funcDecl);
            }
        }

        return std::make_unique<SignatureHelpPreambleInfo>(std::move(builtinFunctionDeclMap));
    }

    // FIXME: implement correctly. Currently this is a placeholder implementation.
    auto HandleSignatureHelp(const SignatureHelpConfig& config, const SignatureHelpPreambleInfo& preambleInfo,
                             const LanguageQueryInfo& queryInfo, const lsp::SignatureHelpParams& params)
        -> std::optional<lsp::SignatureHelp>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        auto expr = TraverseAst(SignatureHelpLocatingVisitor{queryInfo, FromLspPosition(params.position)},
                                queryInfo.GetUserFileAst());

        if (expr) {
            auto funcName = expr->GetNameToken().text;

            std::vector<lsp::SignatureInformation> result;

            // For function in the default library
            // FIXME: handle user preamble
            auto [it, end] = preambleInfo.builtinFunctionDeclMap.equal_range(funcName);
            for (auto [_, funcDecl] : std::ranges::subrange(it, end)) {
                std::string label         = SourceReconstructionBuilder{}.Print(*funcDecl);
                std::string documentation = queryInfo.QueryCommentDescription(*funcDecl);

                result.push_back(lsp::SignatureInformation{
                    .label         = std::move(label),
                    .documentation = std::move(documentation),
                });
            }

            // For function in this module
            for (auto decl : queryInfo.GetUserFileAst().GetGlobalDecls()) {
                if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetNameToken().text == funcName) {
                        std::string label         = SourceReconstructionBuilder{}.Print(*funcDecl);
                        std::string documentation = queryInfo.QueryCommentDescription(*funcDecl);

                        result.push_back(lsp::SignatureInformation{
                            .label         = std::move(label),
                            .documentation = std::move(documentation),
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