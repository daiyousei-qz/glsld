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
        else if (symbolInfo->symbolDecl.IsValid()) {
            // Handle AST symbols
            const AstDecl& accessedDecl = *symbolInfo->symbolDecl.GetDecl();
            size_t declaratorIndex      = symbolInfo->symbolDecl.GetIndex();

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
            else if (auto varDecl = accessedDecl.As<AstVariableDecl>(); varDecl) {
                accessedDeclTok = varDecl->GetDeclarators()[declaratorIndex].nameToken;
            }
            else if (auto memberDecl = accessedDecl.As<AstStructFieldDecl>(); memberDecl) {
                accessedDeclTok = memberDecl->GetDeclarators()[declaratorIndex].nameToken;
            }
            else if (auto structDecl = accessedDecl.As<AstStructDecl>();
                     structDecl && structDecl->GetNameToken() && structDecl->GetNameToken()->IsIdentifier()) {
                accessedDeclTok = *structDecl->GetNameToken();
            }
            else if (auto blockDecl = accessedDecl.As<AstInterfaceBlockDecl>(); blockDecl) {
                accessedDeclTok = blockDecl->GetDeclarator()->nameToken;
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