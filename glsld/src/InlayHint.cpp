#include "LanguageService.h"
#include "ModuleVisitor.h"

namespace glsld
{
    class InlayHintVisitor : public ModuleVisitor<InlayHintVisitor>
    {
    public:
        using ModuleVisitor::ModuleVisitor;

        auto Execute(TextRange range) -> std::vector<lsp::InlayHint>
        {
            this->range = range;

            this->Traverse();

            return std::move(result);
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            return this->EnterIfOverlapRange(node, range);
        }

        auto VisitAstInvokeExpr(AstInvokeExpr& expr) -> void
        {
            if (auto funcExpr = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                if (funcExpr->GetAccessType() == NameAccessType::Function && funcExpr->GetAccessedDecl()) {
                    // We can resolve this function. So inlay hints could/should be computed.

                    const auto funcDecl = funcExpr->GetAccessedDecl().GetDecl()->As<AstFunctionDecl>();
                    GLSLD_ASSERT(funcDecl);

                    const auto paramDeclList = funcDecl->GetParams();

                    for (size_t i = 0; i < paramDeclList.size(); ++i) {
                        GLSLD_ASSERT(i < expr.GetArguments().size());
                        const auto& lexContext = this->GetLexContext();

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

    private:
        TextRange range;

        std::vector<lsp::InlayHint> result;
    };

    auto ComputeInlayHint(const CompilerObject& compileResult, lsp::Range range) -> std::vector<lsp::InlayHint>
    {
        return InlayHintVisitor{compileResult}.Execute(FromLspRange(range));
    }

} // namespace glsld