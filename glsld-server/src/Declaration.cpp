#include "Declaration.h"
#include "SourceText.h"

namespace glsld
{
    auto ComputeDeclaration(const LanguageQueryProvider& provider, const lsp::DocumentUri& uri,
                            lsp::Position position) -> std::vector<lsp::Location>
    {
        auto declTokenResult = provider.LookupSymbolAccess(FromLspPosition(position));
        if (declTokenResult && declTokenResult->symbolDecl.IsValid()) {
            const AstDecl& accessedDecl = *declTokenResult->symbolDecl.GetDecl();
            size_t declaratorIndex      = declTokenResult->symbolDecl.GetIndex();

            // FIXME:
            // Avoid giving declaration if the accessed decl isn't in this module
            // if (accessedDecl.GetModuleId() != compilerObject.GetId()) {
            //     return {};
            // }

            std::optional<SyntaxToken> accessedDeclTok;
            if (auto funcDecl = accessedDecl.As<AstFunctionDecl>(); funcDecl) {
                accessedDeclTok = funcDecl->GetDeclTok();
            }
            else if (auto paramDecl = accessedDecl.As<AstParamDecl>();
                     paramDecl && paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->declTok.IsIdentifier()) {
                accessedDeclTok = paramDecl->GetDeclarator()->declTok;
            }
            else if (auto varDecl = accessedDecl.As<AstVariableDecl>(); varDecl) {
                accessedDeclTok = varDecl->GetDeclarators()[declaratorIndex].declTok;
            }
            else if (auto memberDecl = accessedDecl.As<AstFieldDecl>(); memberDecl) {
                accessedDeclTok = memberDecl->GetDeclarators()[declaratorIndex].declTok;
            }
            else if (auto structDecl = accessedDecl.As<AstStructDecl>();
                     structDecl && structDecl->GetDeclTok() && structDecl->GetDeclTok()->IsIdentifier()) {
                accessedDeclTok = *structDecl->GetDeclTok();
            }
            else if (auto blockDecl = accessedDecl.As<AstInterfaceBlockDecl>(); blockDecl) {
                accessedDeclTok = blockDecl->GetDeclarator()->declTok;
            }

            // FIXME: Support goto declaration in included files
            if (accessedDeclTok && provider.IsSpelledInMainFile(accessedDeclTok->index)) {
                if (auto spelledRange = provider.LookupSpelledTextRangeInMainFile(accessedDeclTok->index)) {
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