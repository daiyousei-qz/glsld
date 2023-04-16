#include "LanguageService.h"
#include "AstHelper.h"

namespace glsld
{
    auto ComputeDeclaration(const CompilerObject& compilerObject, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>
    {
        const auto& lexContext = compilerObject.GetLexContext();

        auto declTokenResult = FindDeclToken(compilerObject, FromLspPosition(position));
        if (declTokenResult && declTokenResult->accessedDecl.IsValid()) {
            AstDecl& accessedDecl  = *declTokenResult->accessedDecl.GetDecl();
            size_t declaratorIndex = declTokenResult->accessedDecl.GetIndex();

            // Avoid giving declaration if the accessed decl isn't in this module
            if (accessedDecl.GetModuleId() != compilerObject.GetId()) {
                return {};
            }

            std::optional<SyntaxToken> accessedDeclTok;
            if (auto funcDecl = accessedDecl.As<AstFunctionDecl>(); funcDecl) {
                accessedDeclTok = funcDecl->GetName();
            }
            else if (auto paramDecl = accessedDecl.As<AstParamDecl>();
                     paramDecl && paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->declTok.IsIdentifier()) {
                accessedDeclTok = paramDecl->GetDeclarator()->declTok;
            }
            else if (auto varDecl = accessedDecl.As<AstVariableDecl>(); varDecl) {
                accessedDeclTok = varDecl->GetDeclarators()[declaratorIndex].declTok;
            }
            else if (auto memberDecl = accessedDecl.As<AstStructMemberDecl>(); memberDecl) {
                accessedDeclTok = memberDecl->GetDeclarators()[declaratorIndex].declTok;
            }
            else if (auto structDecl = accessedDecl.As<AstStructDecl>();
                     structDecl && structDecl->GetDeclToken() && structDecl->GetDeclToken()->IsIdentifier()) {
                accessedDeclTok = *structDecl->GetDeclToken();
            }
            else if (auto blockDecl = accessedDecl.As<AstInterfaceBlockDecl>(); blockDecl) {
                accessedDeclTok = blockDecl->GetDeclarator()->declTok;
            }

            // FIXME: Support goto declaration in included files
            if (accessedDeclTok && lexContext.LookupSpelledFile(*accessedDeclTok) ==
                                       compilerObject.GetSourceContext().GetMainFile()->GetID())
                return {lsp::Location{
                    .uri   = uri,
                    .range = ToLspRange(lexContext.LookupSpelledTextRange(*accessedDeclTok)),
                }};
        }

        return {};
    }

} // namespace glsld