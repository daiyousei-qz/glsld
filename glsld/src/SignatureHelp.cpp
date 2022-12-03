#include "LanguageService.h"

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
            auto locBegin = lexContext.LookupSyntaxLocation(node.GetRange().begin);
            auto locEnd   = lexContext.LookupSyntaxLocation(node.GetRange().end);
            auto range    = TextRange{
                   .start = {.line = locBegin.line, .character = locBegin.column},
                   .end   = {.line = locEnd.line, .character = locEnd.column},
            };

            if (range.ContainPosition(position)) {
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
    auto ComputeSignatureHelp(CompiledModule& compiler, lsp::Position position) -> std::optional<lsp::SignatureHelp>
    {
        SignatureHelpVisitor visitor{compiler.GetLexContext(), TextPosition::FromLspPosition(position)};
        visitor.TraverseAst(compiler.GetAstContext());
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
                        result.push_back(lsp::SignatureInformation{
                            .label = std::string{funcName},
                            .documentation =
                                fmt::format("```glsl\n{} {}{}\n```", ReconstructSourceText(funcDecl->GetReturnType()),
                                            funcName, ReconstructSourceText(funcDecl->GetParams())),
                        });
                    }
                }

                // For function in this module
                for (auto funcDecl : compiler.GetAstContext().functionDecls) {
                    // NOTE we cannot compare lex string here since they are compiled from different compiler instance
                    if (funcDecl->GetName().text.StrView() == funcName) {
                        result.push_back(lsp::SignatureInformation{
                            .label = std::string{funcName},
                            .documentation =
                                fmt::format("```glsl\n{} {}{}\n```", ReconstructSourceText(funcDecl->GetReturnType()),
                                            funcName, ReconstructSourceText(funcDecl->GetParams())),
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