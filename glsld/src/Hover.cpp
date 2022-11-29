#include "LanguageService.h"
#include "AstVisitor.h"

namespace glsld
{
    class HoverVisitor : public AstVisitor<HoverVisitor>
    {
    public:
        HoverVisitor(GlsldCompiler& compiler, lsp::Position position)
            : compiler(compiler), line(position.line), column(position.character)
        {
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            // we already find the hover
            if (hover) {
                return AstVisitPolicy::Leave;
            }

            const auto& lexContext = compiler.GetLexContext();

            auto locBegin = lexContext.LookupSyntaxLocation(node.GetRange().begin);
            auto locEnd   = lexContext.LookupSyntaxLocation(node.GetRange().end);
            if (locBegin.line <= line && locEnd.line >= line) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            const auto& lexContext = compiler.GetLexContext();

            auto locBegin = lexContext.LookupSyntaxLocation(expr.GetAccessName().range.begin);
            auto locEnd   = lexContext.LookupSyntaxLocation(expr.GetAccessName().range.end);
            if (locBegin.line != line || locEnd.line != line) {
                return;
            }
            if (locBegin.column <= column && locEnd.column >= column) {
                GLSLD_ASSERT(!hover.has_value());
                hover = lsp::Hover{.contents = expr.GetAccessName().text.Str(),
                                   .range    = lsp::Range{
                                          .start = {.line      = static_cast<uint32_t>(locBegin.line),
                                                    .character = static_cast<uint32_t>(locBegin.column)},
                                          .end   = {.line      = static_cast<uint32_t>(locEnd.line),
                                                    .character = static_cast<uint32_t>(locEnd.column)},
                                   }};
            }
        }

        auto GetHover() -> std::optional<lsp::Hover>&
        {
            return hover;
        }

    private:
        std::optional<lsp::Hover> hover;

        GlsldCompiler& compiler;
        int line;
        int column;
    }; // namespace glsld

    auto ComputeHover(GlsldCompiler& compiler, lsp::Position position) -> std::optional<lsp::Hover>
    {
        HoverVisitor visitor{compiler, position};
        visitor.TraverseAst(compiler.GetAstContext());

        return std::move(visitor.GetHover());
    }

} // namespace glsld