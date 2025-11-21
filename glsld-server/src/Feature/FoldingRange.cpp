#include "Feature/FoldingRange.h"
#include "Ast/Decl.h"
#include "Ast/Stmt.h"
#include "Server/LanguageQueryVisitor.h"

namespace glsld
{
    class FoldingRangeCollector : public LanguageQueryVisitor<FoldingRangeCollector, std::vector<lsp::FoldingRange>>
    {
    private:
        std::vector<lsp::FoldingRange> result;

    public:
        FoldingRangeCollector(const LanguageQueryInfo& info) : LanguageQueryVisitor(info)
        {
        }

        auto Finish() -> std::vector<lsp::FoldingRange> GLSLD_AST_VISITOR_OVERRIDE
        {
            // Sort folding ranges by start line then end line
            std::ranges::sort(result, [](const lsp::FoldingRange& a, const lsp::FoldingRange& b) {
                return std::tie(a.startLine, a.endLine) < std::tie(b.startLine, b.endLine);
            });

            return std::move(result);
        }

        auto VisitAstCompoundStmt(const AstCompoundStmt& stmt) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            TryAddFoldingRange(stmt.GetSyntaxRange());
        }

        auto VisitAstStructDecl(const AstStructDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            // FIXME: We may want to only fold contents within the struct braces
            TryAddFoldingRange(decl.GetSyntaxRange());
        }

        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            // FIXME: We may want to only fold contents within the block braces
            TryAddFoldingRange(decl.GetSyntaxRange());
        }

    private:
        auto TryAddFoldingRange(AstSyntaxRange range) -> void
        {
            // Look up the spelled text range in the main file
            auto textRange = GetInfo().LookupExpandedTextRange(range);

            // Only create folding range if it spans multiple lines
            if (textRange.start.line < textRange.end.line) {
                result.push_back(lsp::FoldingRange{
                    .startLine = static_cast<uint32_t>(textRange.start.line),
                    .endLine   = static_cast<uint32_t>(textRange.end.line),
                });
            }
        }
    };

    auto GetFoldingRangeOptions(const FoldingRangeConfig& config) -> std::optional<lsp::FoldingRangeOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::FoldingRangeOptions{};
    }

    auto HandleFoldingRange(const FoldingRangeConfig& config, const LanguageQueryInfo& info,
                            const lsp::FoldingRangeParams& params) -> std::vector<lsp::FoldingRange>
    {
        if (!config.enable) {
            return {};
        }

        return TraverseAst(FoldingRangeCollector{info}, info.GetUserFileAst());
    }

} // namespace glsld
