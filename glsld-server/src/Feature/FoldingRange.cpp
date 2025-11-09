#include "Feature/FoldingRange.h"
#include "Ast/Decl.h"
#include "Ast/Stmt.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"

namespace glsld
{
    class FoldingRangeCollector : public LanguageQueryVisitor<FoldingRangeCollector>
    {
    private:
        std::vector<lsp::FoldingRange> result;

    public:
        FoldingRangeCollector(const LanguageQueryInfo& info) : LanguageQueryVisitor(info)
        {
        }

        auto Execute() -> std::vector<lsp::FoldingRange>
        {
            TraverseTranslationUnit();

            // Sort folding ranges by start line
            std::sort(result.begin(), result.end(), [](const lsp::FoldingRange& a, const lsp::FoldingRange& b) {
                return a.startLine < b.startLine;
            });

            return std::move(result);
        }

        auto VisitAstCompoundStmt(const AstCompoundStmt& stmt) -> void
        {
            TryAddFoldingRange(stmt.GetSyntaxRange());
        }

        auto VisitAstStructDecl(const AstStructDecl& decl) -> void
        {
            TryAddFoldingRange(decl.GetSyntaxRange());
        }

        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
        {
            TryAddFoldingRange(decl.GetSyntaxRange());
        }

    private:
        auto TryAddFoldingRange(AstSyntaxRange range) -> void
        {
            // Look up the spelled text range in the main file
            auto startSpelledRange = GetInfo().LookupSpelledTextRangeInMainFile(range.GetBeginID());
            auto endSpelledRange   = GetInfo().LookupSpelledTextRangeInMainFile(range.GetBackID());

            if (!startSpelledRange || !endSpelledRange) {
                return;
            }

            // Only create folding range if it spans multiple lines
            if (startSpelledRange->start.line < endSpelledRange->end.line) {
                result.push_back(lsp::FoldingRange{
                    .startLine = static_cast<uint32_t>(startSpelledRange->start.line),
                    .endLine   = static_cast<uint32_t>(endSpelledRange->end.line),
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

        FoldingRangeCollector collector(info);
        return collector.Execute();
    }

} // namespace glsld
