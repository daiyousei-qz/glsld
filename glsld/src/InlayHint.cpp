#include "LanguageService.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    class InlayHintCollector : public LanguageQueryVisitor<InlayHintCollector>
    {
    private:
        TextRange displayRange;

        std::vector<lsp::InlayHint> result;

    public:
        InlayHintCollector(const LanguageQueryProvider& provider, TextRange range)
            : LanguageQueryVisitor(provider), displayRange(range)
        {
        }

        auto Execute() -> std::vector<lsp::InlayHint>
        {
            TraverseGlobalDeclOverlaps(displayRange);
            return std::move(result);
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto funcExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (!GetProvider().InMainFile(funcExpr->GetAccessName())) {
                    return;
                }

                if (funcExpr->GetAccessType() == NameAccessType::Function && funcExpr->GetAccessedDecl()) {
                    // We can resolve this function. So inlay hints could/should be computed.

                    const auto funcDecl = funcExpr->GetAccessedDecl().GetDecl()->As<AstFunctionDecl>();
                    GLSLD_ASSERT(funcDecl);

                    const auto paramDeclList = funcDecl->GetParams();

                    for (size_t i = 0; i < paramDeclList.size(); ++i) {
                        GLSLD_ASSERT(i < expr.GetArguments().size());
                        const auto& lexContext = GetProvider().GetLexContext();

                        auto paramDecl = paramDeclList[i];
                        auto argExpr   = expr.GetArguments()[i];

                        auto locBegin =
                            lexContext.LookupExpandedTextRange(argExpr->GetSyntaxRange().startTokenIndex).start;
                        StringView hintText = "";
                        if (paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->declTok.IsIdentifier()) {
                            hintText = paramDecl->GetDeclarator()->declTok.text.StrView();
                        }

                        if (!hintText.Empty()) {
                            result.push_back(lsp::InlayHint{
                                .position     = ToLspPosition({locBegin.line, locBegin.character}),
                                .label        = fmt::format("{}:", hintText),
                                .paddingLeft  = false,
                                .paddingRight = true,
                            });
                        }
                    }
                }
            }
        }
    };

    auto ComputeInlayHint(const LanguageQueryProvider& provider, lsp::Range range) -> std::vector<lsp::InlayHint>
    {
        return InlayHintCollector{provider, FromLspRange(range)}.Execute();
    }

} // namespace glsld