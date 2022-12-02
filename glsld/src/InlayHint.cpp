#include "LanguageService.h"

namespace glsld
{
    class InlayHintVisitor : public AstVisitor<InlayHintVisitor>
    {
    public:
        InlayHintVisitor(GlsldCompiler& compiler, TextRange range) : compiler(compiler), range(range)
        {
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            const auto& lexContext = compiler.GetLexContext();
            auto locBegin          = lexContext.LookupSyntaxLocation(node.GetRange().begin);
            auto locEnd            = lexContext.LookupSyntaxLocation(node.GetRange().end);

            auto nodeRange = TextRange{
                {locBegin.line, locBegin.column},
                {locEnd.line, locEnd.column},
            };
            if (range.ContainPosition(nodeRange.start) || range.ContainPosition(nodeRange.end)) {
                return AstVisitPolicy::Traverse;
            }

            return AstVisitPolicy::Leave;
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto funcExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (funcExpr->GetAccessType() == NameAccessType::Function && funcExpr->GetAccessedDecl()) {
                    // We can resolve the function
                    const auto paramDeclList = funcExpr->GetAccessedDecl()->As<AstFunctionDecl>()->GetParams();

                    for (size_t i = 0; i < paramDeclList.size(); ++i) {
                        GLSLD_ASSERT(i < expr.GetArguments().size());
                        const auto& lexContext = compiler.GetLexContext();

                        auto paramDecl = paramDeclList[i];
                        auto argExpr   = expr.GetArguments()[i];

                        auto locBegin = lexContext.LookupSyntaxLocation(argExpr->GetRange().begin);
                        auto hintText = paramDecl->GetDeclTok().value_or(SyntaxToken{}).text.StrView();
                        if (!hintText.empty()) {
                            result.push_back(lsp::InlayHint{
                                .position     = TextPosition::ToLspPosition({locBegin.line, locBegin.column}),
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

        GlsldCompiler& compiler;
        TextRange range;
    };

    auto ComputeInlayHint(GlsldCompiler& compiler, lsp::Range range) -> std::vector<lsp::InlayHint>
    {
        InlayHintVisitor visitor{compiler, TextRange::FromLspRange(range)};
        visitor.TraverseAst(compiler.GetAstContext());
        return visitor.Export();
    }

} // namespace glsld