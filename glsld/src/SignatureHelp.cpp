#include "LanguageService.h"
#include "StandardDocumentation.h"
#include "ModuleVisitor.h"

namespace glsld
{
    class SignatureHelpVisitor : public AstVisitor<SignatureHelpVisitor>
    {
    public:
        SignatureHelpVisitor(const LexContext& lexContext, TextPosition position)
            : lexContext(lexContext), position(position)
        {
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            auto nodeRange = lexContext.LookupTextRange(node.GetRange());
            if (nodeRange.Contains(position)) {
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

        auto Export() -> AstInvokeExpr*
        {
            return invokeExpr;
        }

    private:
        const LexContext& lexContext;
        TextPosition position;

        AstInvokeExpr* invokeExpr = nullptr;
    };

    // FIXME: implement correctly. Currently this is a placeholder implmentation.
    auto ComputeSignatureHelp(const CompilerObject& compileResult, lsp::Position position)
        -> std::optional<lsp::SignatureHelp>
    {
        SignatureHelpVisitor visitor{compileResult.GetLexContext(), FromLspPosition(position)};
        visitor.TraverseAst(compileResult.GetAstContext());
        auto expr = visitor.Export();

        if (expr) {
            auto funcExpr = expr->GetInvokedExpr()->As<AstNameAccessExpr>();
            if (funcExpr && funcExpr->GetAccessType() == NameAccessType::Function) {
                auto funcName = funcExpr->GetAccessName().text.StrView();

                std::vector<lsp::SignatureInformation> result;

                // For function in the default library
                for (auto funcDecl : GetDefaultLibraryModule()->GetAstContext().functionDecls) {
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
                for (auto funcDecl : compileResult.GetAstContext().functionDecls) {
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