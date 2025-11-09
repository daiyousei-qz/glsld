#include "Feature/FoldingRange.h"
#include "Support/SourceText.h"

namespace glsld
{
    // Helper function to add a folding range if it spans multiple lines
    static auto TryAddFoldingRange(std::vector<lsp::FoldingRange>& output, const LanguageQueryInfo& info,
                                   AstSyntaxRange range) -> void
    {
        // Look up the spelled text range in the main file
        auto startSpelledRange = info.LookupSpelledTextRangeInMainFile(range.startID);
        auto endSpelledRange   = info.LookupSpelledTextRangeInMainFile(range.endID);

        if (!startSpelledRange || !endSpelledRange) {
            return;
        }

        // Only create folding range if it spans multiple lines
        if (startSpelledRange->start.line < endSpelledRange->end.line) {
            output.push_back(lsp::FoldingRange{
                .startLine = static_cast<uint32_t>(startSpelledRange->start.line),
                .endLine   = static_cast<uint32_t>(endSpelledRange->end.line),
            });
        }
    }

    // Collect folding ranges from compound statements (blocks)
    static auto CollectCompoundStmtFoldingRanges(std::vector<lsp::FoldingRange>& output,
                                                  const LanguageQueryInfo& info, const AstStmt* stmt) -> void
    {
        if (!stmt) {
            return;
        }

        // Visit compound statements
        if (auto compoundStmt = stmt->As<AstCompoundStmt>(); compoundStmt) {
            TryAddFoldingRange(output, info, compoundStmt->GetSyntaxRange());

            // Recursively visit children
            for (const auto* child : compoundStmt->GetChildren()) {
                CollectCompoundStmtFoldingRanges(output, info, child);
            }
        }
        // Visit if statements
        else if (auto ifStmt = stmt->As<AstIfStmt>(); ifStmt) {
            CollectCompoundStmtFoldingRanges(output, info, ifStmt->GetThenStmt());
            CollectCompoundStmtFoldingRanges(output, info, ifStmt->GetElseStmt());
        }
        // Visit switch statements
        else if (auto switchStmt = stmt->As<AstSwitchStmt>(); switchStmt) {
            CollectCompoundStmtFoldingRanges(output, info, switchStmt->GetBody());
        }
        // Visit for statements
        else if (auto forStmt = stmt->As<AstForStmt>(); forStmt) {
            CollectCompoundStmtFoldingRanges(output, info, forStmt->GetBody());
        }
        // Visit while statements
        else if (auto whileStmt = stmt->As<AstWhileStmt>(); whileStmt) {
            CollectCompoundStmtFoldingRanges(output, info, whileStmt->GetBody());
        }
        // Visit do-while statements
        else if (auto doWhileStmt = stmt->As<AstDoWhileStmt>(); doWhileStmt) {
            CollectCompoundStmtFoldingRanges(output, info, doWhileStmt->GetBody());
        }
        // Visit declaration statements
        else if (auto declStmt = stmt->As<AstDeclStmt>(); declStmt) {
            // Check if the declaration statement contains a compound statement
            // (e.g., struct or function definition)
            CollectCompoundStmtFoldingRanges(output, info, declStmt->GetDecl());
        }
    }

    // Helper overload for declarations
    static auto CollectCompoundStmtFoldingRanges(std::vector<lsp::FoldingRange>& output,
                                                  const LanguageQueryInfo& info, const AstDecl* decl) -> void
    {
        if (!decl) {
            return;
        }

        // For function declarations with bodies
        if (auto functionDecl = decl->As<AstFunctionDecl>(); functionDecl) {
            if (functionDecl->GetBody()) {
                CollectCompoundStmtFoldingRanges(output, info, functionDecl->GetBody());
            }
        }
    }

    // Collect folding ranges from struct declarations
    static auto CollectStructFoldingRanges(std::vector<lsp::FoldingRange>& output, const LanguageQueryInfo& info)
        -> void
    {
        for (const auto& decl : info.GetUserFileAst().GetGlobalDecls()) {
            // Check for struct declarations in variable declarations
            if (auto variableDecl = decl->As<AstVariableDecl>(); variableDecl) {
                if (auto structDecl = variableDecl->GetQualType()->GetStructDecl(); structDecl) {
                    TryAddFoldingRange(output, info, structDecl->GetSyntaxRange());
                }
            }
            // Check for interface block declarations
            else if (auto interfaceBlockDecl = decl->As<AstInterfaceBlockDecl>(); interfaceBlockDecl) {
                TryAddFoldingRange(output, info, interfaceBlockDecl->GetSyntaxRange());
            }
        }
    }

    // Collect folding ranges from function declarations
    static auto CollectFunctionFoldingRanges(std::vector<lsp::FoldingRange>& output, const LanguageQueryInfo& info)
        -> void
    {
        for (const auto& decl : info.GetUserFileAst().GetGlobalDecls()) {
            if (auto functionDecl = decl->As<AstFunctionDecl>(); functionDecl) {
                // Add folding range for the function body if it exists
                if (functionDecl->GetBody()) {
                    CollectCompoundStmtFoldingRanges(output, info, functionDecl->GetBody());
                }
            }
        }
    }

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

        std::vector<lsp::FoldingRange> result;

        // Collect folding ranges from various sources
        CollectStructFoldingRanges(result, info);
        CollectFunctionFoldingRanges(result, info);

        // Sort folding ranges by start line
        std::sort(result.begin(), result.end(), [](const lsp::FoldingRange& a, const lsp::FoldingRange& b) {
            return a.startLine < b.startLine;
        });

        return result;
    }

} // namespace glsld
