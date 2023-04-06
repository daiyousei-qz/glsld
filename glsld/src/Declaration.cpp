#include "LanguageService.h"
#include "AstHelper.h"

namespace glsld
{
    auto ComputeDeclaration(const CompilerObject& compileResult, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>
    {
        auto declTokenResult = FindDeclToken(compileResult, FromLspPosition(position));
        if (declTokenResult && declTokenResult->accessedDecl.IsValid()) {
            AstDecl& decl          = *declTokenResult->accessedDecl.GetDecl();
            size_t declaratorIndex = declTokenResult->accessedDecl.GetIndex();

            // Avoid giving declaration if the accessed decl isn't in this module
            if (decl.GetModuleId() != compileResult.GetId()) {
                return {};
            }

            TextRange declRange;
            if (auto funcDecl = decl.As<AstFunctionDecl>(); funcDecl) {
                declRange = compileResult.GetLexContext().LookupTextRange(funcDecl->GetName());
            }
            else if (auto paramDecl = decl.As<AstParamDecl>();
                     paramDecl && paramDecl->GetDeclarator() && paramDecl->GetDeclarator()->declTok.IsIdentifier()) {
                declRange = compileResult.GetLexContext().LookupTextRange(paramDecl->GetDeclarator()->declTok);
            }
            else if (auto varDecl = decl.As<AstVariableDecl>(); varDecl) {
                declRange =
                    compileResult.GetLexContext().LookupTextRange(varDecl->GetDeclarators()[declaratorIndex].declTok);
            }
            else if (auto memberDecl = decl.As<AstStructMemberDecl>(); memberDecl) {
                declRange = compileResult.GetLexContext().LookupTextRange(
                    memberDecl->GetDeclarators()[declaratorIndex].declTok);
            }
            else if (auto structDecl = decl.As<AstStructDecl>();
                     structDecl && structDecl->GetDeclToken() && structDecl->GetDeclToken()->IsIdentifier()) {
                declRange = compileResult.GetLexContext().LookupTextRange(*structDecl->GetDeclToken());
            }
            else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>(); blockDecl) {
                declRange = compileResult.GetLexContext().LookupTextRange(blockDecl->GetDeclarator()->declTok);
            }
            else {
                declRange = compileResult.GetLexContext().LookupTextRange(decl.GetRange());
            }

            return {lsp::Location{
                .uri   = uri,
                .range = ToLspRange(declRange),
            }};
        }

        return {};
    }

} // namespace glsld