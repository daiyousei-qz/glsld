#include "LanguageService.h"
#include "AstHelper.h"

namespace glsld
{
    auto ComputeDeclaration(CompiledModule& compiler, const lsp::DocumentUri& uri, lsp::Position position)
        -> std::vector<lsp::Location>
    {
        struct DeclarationProcessor : public DeclTokenCallback<AstDecl*>
        {
            auto ProcessToken(SyntaxToken token, TextRange range, AstDecl& decl) const
                -> std::optional<AstDecl*> override
            {
                return &decl;
            }
            auto ProcessTokenWithoutDecl(SyntaxToken token, TextRange range, NameAccessType type) const
                -> std::optional<AstDecl*> override
            {
                return std::nullopt;
            }
        };

        auto decl = ProcessDeclToken(compiler, FromLspPosition(position), DeclarationProcessor{});
        if (decl.has_value()) {
            TextRange declRange;
            if (auto funcDecl = (*decl)->As<AstFunctionDecl>(); funcDecl) {
                declRange = compiler.GetLexContext().LookupTextRange(funcDecl->GetName());
            }
            else if (auto paramDecl = (*decl)->As<AstParamDecl>(); paramDecl && paramDecl->GetDeclToken()) {
                declRange = compiler.GetLexContext().LookupTextRange(*paramDecl->GetDeclToken());
            }
            // FIXME:
            // else if (auto varDecl = (*decl)->As<AstVariableDecl>(); varDecl) {
            // }
            else {
                declRange = compiler.GetLexContext().LookupTextRange((*decl)->GetRange());
            }
            return {lsp::Location{
                .uri   = uri,
                .range = ToLspRange(declRange),
            }};
        }
        else {
            return {};
        }

        return {};
    }

} // namespace glsld