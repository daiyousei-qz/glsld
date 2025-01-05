#include "Declaration.h"
#include "SourceText.h"
#include "SymbolQuery.h"

namespace glsld
{
    auto ComputeDeclaration(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>
    {
        auto declTokenResult = QuerySymbolByPosition(provider, FromLspPosition(position));
        if (declTokenResult && declTokenResult->symbolDecl.IsValid()) {
            const AstDecl& accessedDecl = *declTokenResult->symbolDecl.GetDecl();
            size_t declaratorIndex      = declTokenResult->symbolDecl.GetIndex();

            // FIXME:
            // Avoid giving declaration if the accessed decl isn't in this module
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

            // FIXME: Support goto declaration in included files
            if (accessedDeclTok && provider.IsSpelledInMainFile(accessedDeclTok->id)) {
                if (auto spelledRange = provider.LookupSpelledTextRangeInMainFile(accessedDeclTok->id)) {
                    return {lsp::Location{
                        .uri   = uri,
                        .range = ToLspRange(*spelledRange),
                    }};
                }
            }
        }

        return {};
    }

} // namespace glsld