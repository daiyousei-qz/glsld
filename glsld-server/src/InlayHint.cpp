#include "InlayHints.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

namespace glsld
{
    // FIXME: handle display range better
    class InlayHintCollector : public LanguageQueryVisitor<InlayHintCollector>
    {
    private:
        const InlayHintConfig& config;
        TextRange displayRange;

        std::vector<lsp::InlayHint> result;

    public:
        InlayHintCollector(const LanguageQueryProvider& provider, const InlayHintConfig& config, TextRange range)
            : LanguageQueryVisitor(provider), config(config), displayRange(range)
        {
        }

        auto Execute() -> std::vector<lsp::InlayHint>
        {
            TraverseTranslationUnit();
            return std::move(result);
        }

        auto VisitAstImplicitCastExpr(const AstImplicitCastExpr& expr) -> void
        {
            if (!config.enableImplicitCastHint) {
                return;
            }
            if (expr.GetDeducedType()->IsError() || expr.GetSyntaxRange().GetTokenCount() > 1) {
                // FIXME: handle multi-token case. For example float(1+2)?
                return;
            }

            auto exprTextRange = GetProvider().LookupExpandedTextRange(expr);
            if (exprTextRange.IsEmpty()) {
                return;
            }

            TryAddInlayHint(exprTextRange.start, fmt::format("({})", expr.GetDeducedType()->GetDebugName()), false,
                            false);
        }

        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
        {
            if (!config.enableArgumentNameHint) {
                return;
            }
            if (!expr.GetResolvedFunction()) {
                return;
            }

            const auto paramDeclList = expr.GetResolvedFunction()->GetParams();
            for (size_t i = 0; i < paramDeclList.size(); ++i) {
                GLSLD_ASSERT(i < expr.GetArgs().size());

                auto paramDecl = paramDeclList[i];
                auto argExpr   = expr.GetArgs()[i];

                auto argTextRange = GetProvider().LookupExpandedTextRange(*argExpr);
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
                    TryAddInlayHint(argTextRange.start, fmt::format("{}{}:", outputHint, paramNameHint), false, true);
                }
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
        {
            if (!config.enableBlockEndHint) {
                return;
            }
            if (!decl.GetDeclTok().IsIdentifier()) {
                return;
            }

            auto declTextRange = GetProvider().LookupExpandedTextRange(decl);
            if (declTextRange.GetNumLines() < config.blockEndHintLineThreshold) {
                return;
            }

            // FIXME: don't show hints if any additional tokens are present in the line
            TryAddInlayHint(declTextRange.end, fmt::format("// {}", decl.GetDeclTok().text.StrView()), true, false);
        }

    private:
        auto TryAddInlayHint(TextPosition position, std::string label, bool paddingLeft, bool paddingRight) -> void
        {
            if (displayRange.Contains(position)) {
                result.push_back(lsp::InlayHint{
                    .position     = ToLspPosition(position),
                    .label        = std::move(label),
                    .paddingLeft  = paddingLeft,
                    .paddingRight = paddingRight,
                });
            }
        }
    };

    auto ComputeInlayHint(const LanguageQueryProvider& provider, const InlayHintConfig& config, lsp::Range displayRange)
        -> std::vector<lsp::InlayHint>
    {
        return InlayHintCollector{provider, config, FromLspRange(displayRange)}.Execute();
    }

} // namespace glsld