#include "LanguageService.h"

namespace glsld
{
    class InlayHintVisitor : public AstVisitor<InlayHintVisitor>
    {
    public:
        InlayHintVisitor(CompiledModule& compiler, TextRange range) : compiler(compiler), range(range)
        {
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            auto nodeRange = compiler.GetLexContext().LookupTextRange(node.GetRange());
            if (range.Contains(nodeRange.start) || range.Contains(nodeRange.end)) {
                return AstVisitPolicy::Traverse;
            }

            return AstVisitPolicy::Leave;
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto funcExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (funcExpr->GetAccessType() == NameAccessType::Function && funcExpr->GetAccessedDecl()) {
                    // We can resolve the function
                    const auto paramDeclList =
                        funcExpr->GetAccessedDecl().GetDecl()->As<AstFunctionDecl>()->GetParams();

                    for (size_t i = 0; i < paramDeclList.size(); ++i) {
                        GLSLD_ASSERT(i < expr.GetArguments().size());
                        const auto& lexContext = compiler.GetLexContext();

                        auto paramDecl = paramDeclList[i];
                        auto argExpr   = expr.GetArguments()[i];

                        auto locBegin       = lexContext.LookupFirstTextPosition(argExpr->GetRange());
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

        auto Export() -> std::vector<lsp::InlayHint>
        {
            return std::move(result);
        }

    private:
        std::vector<lsp::InlayHint> result;

        CompiledModule& compiler;
        TextRange range;
    };

    auto ComputeInlayHint(CompiledModule& compiler, lsp::Range range) -> std::vector<lsp::InlayHint>
    {
        InlayHintVisitor visitor{compiler, FromLspRange(range)};
        visitor.TraverseAst(compiler.GetAstContext());
        return visitor.Export();
    }

} // namespace glsld