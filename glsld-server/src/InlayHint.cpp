#include "InlayHints.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

namespace glsld
{
    // FIXME: handle display range
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
            TraverseTranslationUnit();
            return std::move(result);
        }

        auto VisitAstImplicitCastExpr(const AstImplicitCastExpr& expr) -> void
        {
            if (expr.GetDeducedType()->IsError()) {
                return;
            }

            auto exprTextRange = GetProvider().GetExpandedTextRange(expr);
            if (exprTextRange.IsEmpty()) {
                return;
            }

            result.push_back(lsp::InlayHint{
                .position     = ToLspPosition({exprTextRange.start.line, exprTextRange.start.character}),
                .label        = fmt::format("({})", expr.GetDeducedType()->GetDebugName()),
                .paddingLeft  = false,
                .paddingRight = false,
            });
        }

        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
        {
            if (!expr.GetResolvedFunction()) {
                return;
            }

            const auto paramDeclList = expr.GetResolvedFunction()->GetParams();
            for (size_t i = 0; i < paramDeclList.size(); ++i) {
                GLSLD_ASSERT(i < expr.GetArgs().size());

                auto paramDecl = paramDeclList[i];
                auto argExpr   = expr.GetArgs()[i];

                auto argTextRange = GetProvider().GetExpandedTextRange(*argExpr);
                if (argTextRange.IsEmpty()) {
                    continue;
                }

                StringView outputHint = "";
                if (paramDecl->IsOutputParam()) {
                    outputHint = "&";
                }
                StringView paramNameHint = "";
                if (paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->declTok.IsIdentifier()) {
                    paramNameHint = paramDecl->GetDeclarator()->declTok.text.StrView();
                }

                if (!outputHint.Empty() || !paramNameHint.Empty()) {
                    result.push_back(lsp::InlayHint{
                        .position     = ToLspPosition({argTextRange.start.line, argTextRange.start.character}),
                        .label        = fmt::format("{}{}:", outputHint, paramNameHint),
                        .paddingLeft  = false,
                        .paddingRight = true,
                    });
                }
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
        {
            if (!decl.GetDeclTok().IsIdentifier()) {
                return;
            }

            auto declTextRange = GetProvider().GetExpandedTextRange(decl);
            // TODO: make this configurable
            if (declTextRange.GetNumLines() < 4) {
                return;
            }

            // FIXME: don't show hints if any additional tokens are present in the line
            result.push_back(lsp::InlayHint{
                .position     = ToLspPosition({declTextRange.end.line, declTextRange.end.character}),
                .label        = fmt::format("// {}", decl.GetDeclTok().text.StrView()),
                .paddingLeft  = true,
                .paddingRight = false,
            });
        }
    };

    auto ComputeInlayHint(const LanguageQueryProvider& provider, lsp::Range range) -> std::vector<lsp::InlayHint>
    {
        return InlayHintCollector{provider, FromLspRange(range)}.Execute();
    }

} // namespace glsld