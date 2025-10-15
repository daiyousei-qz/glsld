#include "Feature/Definition.h"
#include "Support/SourceText.h"

namespace glsld
{
    auto GetDefinitionOptions(const DefinitionConfig& config) -> std::optional<lsp::DefinitionOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::DefinitionOptions{};
    }

    auto HandleDefinition(const DefinitionConfig& config, const LanguageQueryInfo& info,
                          const lsp::DefinitionParams& params) -> std::vector<lsp::Location>
    {
        // FIXME: we assume single source file for now
        if (!config.enable) {
            return {};
        }

        auto symbolInfo = info.QuerySymbolByPosition(FromLspPosition(params.position));
        if (!symbolInfo) {
            return {};
        }

        // Handle PP symbols
        if (symbolInfo->symbolType == SymbolDeclType::HeaderName) {
            // FIXME: goto include file
            return {};
        }
        else if (symbolInfo->symbolType == SymbolDeclType::Macro) {
            if (auto macroInfo = symbolInfo->ppSymbolOccurrence->GetMacroInfo(); macroInfo && macroInfo->definition) {
                const auto& declToken = macroInfo->definition->macroName;
                if (info.IsMainFile(declToken.spelledFile)) {
                    // FIXME: Currently we only support macros defined in the main file. Need support include file.
                    return {lsp::Location{
                        .uri   = params.textDocument.uri,
                        .range = ToLspRange(declToken.spelledRange),
                    }};
                }
            }
        }
        else if (symbolInfo->symbolDecl) {
            // Handle AST symbols
            const AstDecl& accessedDecl = *symbolInfo->symbolDecl;

            // FIXME:
            // Avoid giving Definition if the accessed decl isn't in this module
            // if (accessedDecl.GetModuleId() != compilerObject.GetId()) {
            //     return {};
            // }

            std::optional<AstSyntaxToken> accessedDeclTok;
            if (auto funcDecl = accessedDecl.As<AstFunctionDecl>(); funcDecl) {
                accessedDeclTok = funcDecl->GetNameToken();
            }
            else if (auto paramDecl = accessedDecl.As<AstParamDecl>();
                     paramDecl && paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->nameToken.IsIdentifier()) {
                accessedDeclTok = paramDecl->GetDeclarator()->nameToken;
            }
            else if (auto varDecl = accessedDecl.As<AstVariableDeclaratorDecl>(); varDecl) {
                accessedDeclTok = varDecl->GetNameToken();
            }
            else if (auto structFieldDeclaratorDecl = accessedDecl.As<AstStructFieldDeclaratorDecl>();
                     structFieldDeclaratorDecl) {
                accessedDeclTok = structFieldDeclaratorDecl->GetNameToken();
            }
            else if (auto structDecl = accessedDecl.As<AstStructDecl>();
                     structDecl && structDecl->GetNameToken() && structDecl->GetNameToken()->IsIdentifier()) {
                accessedDeclTok = *structDecl->GetNameToken();
            }
            else if (auto blockDecl = accessedDecl.As<AstInterfaceBlockDecl>(); blockDecl) {
                accessedDeclTok = blockDecl->GetDeclarator()->nameToken;
            }
            else if (auto blockFieldDeclaratorDecl = accessedDecl.As<AstBlockFieldDeclaratorDecl>();
                     blockFieldDeclaratorDecl) {
                accessedDeclTok = blockFieldDeclaratorDecl->GetNameToken();
            }

            // FIXME: Support goto Definition in included files
            if (accessedDeclTok && info.IsSpelledInMainFile(accessedDeclTok->id)) {
                if (auto spelledRange = info.LookupSpelledTextRangeInMainFile(accessedDeclTok->id)) {
                    return {lsp::Location{
                        .uri   = params.textDocument.uri,
                        .range = ToLspRange(*spelledRange),
                    }};
                }
            }
        }

        return {};
    }
} // namespace glsld