#include "Feature/DocumentSymbol.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"

namespace glsld
{
    static auto CollectPPDocumentSymbols(std::vector<lsp::DocumentSymbol>& output, const LanguageQueryInfo& info)
        -> void
    {
        for (const auto& ppOccurrence : info.GetPreprocessInfo().GetAllOccurrences()) {
            if (auto macroInfo = ppOccurrence.GetMacroInfo();
                macroInfo && macroInfo->occurrenceType == PPMacroOccurrenceType::Define) {
                output.push_back(lsp::DocumentSymbol{
                    .name           = macroInfo->macroName.text.Str(),
                    .detail         = "macro",
                    .kind           = lsp::SymbolKind::String,
                    .range          = ToLspRange(macroInfo->macroName.spelledRange),
                    .selectionRange = ToLspRange(macroInfo->macroName.spelledRange),
                    .children       = {},
                });
            }
        }
    }

    static auto CollectAstDocumentSymbols(std::vector<lsp::DocumentSymbol>& output, const LanguageQueryInfo& info)
        -> void
    {
        auto tryAddSymbol = [&info](std::vector<lsp::DocumentSymbol>& buffer, AstSyntaxToken token,
                                    lsp::SymbolKind kind, std::string detail) -> bool {
            if (!token.IsIdentifier()) {
                return false;
            }

            if (auto spelledRange = info.LookupSpelledTextRangeInMainFile(token.id)) {
                auto lspSpelledRange = ToLspRange(*spelledRange);
                buffer.push_back(lsp::DocumentSymbol{
                    .name           = token.text.Str(),
                    .detail         = std::move(detail),
                    .kind           = kind,
                    .range          = lspSpelledRange,
                    .selectionRange = lspSpelledRange,
                    .children       = {},
                });

                return true;
            }

            return false;
        };

        for (const auto& decl : info.GetUserFileAst().GetGlobalDecls()) {
            if (auto functionDecl = decl->As<AstFunctionDecl>(); functionDecl) {
                tryAddSymbol(output, functionDecl->GetNameToken(), lsp::SymbolKind::Function, "function");
            }
            else if (auto variableDecl = decl->As<AstVariableDecl>(); variableDecl) {
                // Struct declaration if any
                if (auto structDecl = variableDecl->GetQualType()->GetStructDecl(); structDecl) {
                    if (structDecl->GetNameToken()) {
                        if (tryAddSymbol(output, *structDecl->GetNameToken(), lsp::SymbolKind::Struct, "struct")) {
                            for (auto memberDecl : structDecl->GetMembers()) {
                                for (const auto& declarator : memberDecl->GetDeclarators()) {
                                    tryAddSymbol(output.back().children, declarator.nameToken, lsp::SymbolKind::Field,
                                                 "field");
                                }
                            }
                        }
                    }
                }

                // Variable declaration
                for (const auto& declarator : variableDecl->GetDeclarators()) {
                    tryAddSymbol(output, declarator.nameToken, lsp::SymbolKind::Variable, "variable");
                }
            }
            else if (auto interfaceBlockDecl = decl->As<AstInterfaceBlockDecl>(); interfaceBlockDecl) {
                if (interfaceBlockDecl->GetDeclarator()) {
                    // Named block
                    // FIXME: should block name be added as a symbol?
                    if (tryAddSymbol(output, interfaceBlockDecl->GetDeclarator()->nameToken, lsp::SymbolKind::Variable,
                                     "interface block")) {
                        for (auto memberDecl : interfaceBlockDecl->GetMembers()) {
                            for (const auto& declarator : memberDecl->GetDeclarators()) {
                                tryAddSymbol(output.back().children, declarator.nameToken, lsp::SymbolKind::Field,
                                             "field");
                            }
                        }
                    }
                }
                else {
                    // Unnamed block.
                    // We add members to global scope as if they are global variables.
                    for (auto memberDecl : interfaceBlockDecl->GetMembers()) {
                        for (const auto& declarator : memberDecl->GetDeclarators()) {
                            tryAddSymbol(output, declarator.nameToken, lsp::SymbolKind::Variable,
                                         "unnamed block field");
                        }
                    }
                }
            }
        }
    }

    auto GetDocumentSymbolOptions(const DocumentSymbolConfig& config) -> std::optional<lsp::DocumentSymbolOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::DocumentSymbolOptions{};
    }

    auto HandleDocumentSymbol(const DocumentSymbolConfig& config, const LanguageQueryInfo& info,
                              const lsp::DocumentSymbolParams& params) -> std::vector<lsp::DocumentSymbol>
    {
        if (!config.enable) {
            return {};
        }

        std::vector<lsp::DocumentSymbol> result;

        // Collect document symbols
        CollectPPDocumentSymbols(result, info);
        CollectAstDocumentSymbols(result, info);

        // Sort symbols by range
        std::sort(result.begin(), result.end(), [](const lsp::DocumentSymbol& a, const lsp::DocumentSymbol& b) {
            const auto aPos = a.range.start;
            const auto bPos = b.range.start;
            return std::tie(aPos.line, aPos.character) < std::tie(bPos.line, bPos.character);
        });

        return result;
    }

} // namespace glsld