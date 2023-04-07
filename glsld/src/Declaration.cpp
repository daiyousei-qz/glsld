#include "LanguageService.h"
#include "AstHelper.h"

namespace glsld
{
    auto ComputeDeclaration(const CompilerObject& compilerObject, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>
    {
        auto declTokenResult = FindDeclToken(compilerObject, FromLspPosition(position));
        if (declTokenResult && declTokenResult->accessedDecl.IsValid()) {
            AstDecl& decl          = *declTokenResult->accessedDecl.GetDecl();
            size_t declaratorIndex = declTokenResult->accessedDecl.GetIndex();

            // Avoid giving declaration if the accessed decl isn't in this module
            if (decl.GetModuleId() != compilerObject.GetId()) {
                return {};
            }

            TextRange declRange;
            if (auto funcDecl = decl.As<AstFunctionDecl>(); funcDecl) {
                declRange = compilerObject.GetLexContext().LookupTextRange(funcDecl->GetName());
            }
            else if (auto paramDecl = decl.As<AstParamDecl>();
                     paramDecl && paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->declTok.IsIdentifier()) {
                declRange = compilerObject.GetLexContext().LookupTextRange(paramDecl->GetDeclarator()->declTok);
            }
            else if (auto varDecl = decl.As<AstVariableDecl>(); varDecl) {
                declRange =
                    compilerObject.GetLexContext().LookupTextRange(varDecl->GetDeclarators()[declaratorIndex].declTok);
            }
            else if (auto memberDecl = decl.As<AstStructMemberDecl>(); memberDecl) {
                declRange = compilerObject.GetLexContext().LookupTextRange(
                    memberDecl->GetDeclarators()[declaratorIndex].declTok);
            }
            else if (auto structDecl = decl.As<AstStructDecl>();
                     structDecl && structDecl->GetDeclToken() && structDecl->GetDeclToken()->IsIdentifier()) {
                declRange = compilerObject.GetLexContext().LookupTextRange(*structDecl->GetDeclToken());
            }
            else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>(); blockDecl) {
                declRange = compilerObject.GetLexContext().LookupTextRange(blockDecl->GetDeclarator()->declTok);
            }
            else {
                declRange = compilerObject.GetLexContext().LookupTextRange(decl.GetRange());
            }

            return {lsp::Location{
                .uri   = uri,
                .range = ToLspRange(declRange),
            }};
        }

        return {};
    }

} // namespace glsld