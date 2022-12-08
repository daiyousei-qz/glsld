#include "LanguageService.h"
#include "AstHelper.h"

namespace glsld
{
    auto ComputeDeclaration(CompiledModule& compiler, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>
    {
        auto declTokenResult = FindDeclToken(compiler, FromLspPosition(position));
        if (declTokenResult && declTokenResult->accessedDecl.IsValid()) {
            AstDecl& decl          = *declTokenResult->accessedDecl.GetDecl();
            size_t declaratorIndex = declTokenResult->accessedDecl.GetIndex();

            TextRange declRange;
            if (auto funcDecl = decl.As<AstFunctionDecl>(); funcDecl) {
                declRange = compiler.GetLexContext().LookupTextRange(funcDecl->GetName());
            }
            else if (auto paramDecl = decl.As<AstParamDecl>(); paramDecl && paramDecl->GetDeclToken()) {
                declRange = compiler.GetLexContext().LookupTextRange(*paramDecl->GetDeclToken());
            }
            else if (auto varDecl = decl.As<AstVariableDecl>(); varDecl) {
                declRange =
                    compiler.GetLexContext().LookupTextRange(varDecl->GetDeclarators()[declaratorIndex].declTok);
            }
            else if (auto memberDecl = decl.As<AstStructMemberDecl>(); memberDecl) {
                declRange =
                    compiler.GetLexContext().LookupTextRange(memberDecl->GetDeclarators()[declaratorIndex].declTok);
            }
            else {
                declRange = compiler.GetLexContext().LookupTextRange(decl.GetRange());
            }

            return {lsp::Location{
                .uri   = uri,
                .range = ToLspRange(declRange),
            }};
        }

        return {};
    }

} // namespace glsld