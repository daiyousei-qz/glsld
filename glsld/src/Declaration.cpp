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

        auto decl = ProcessDeclToken(compiler, TextPosition::FromLspPosition(position), DeclarationProcessor{});
        if (decl.has_value()) {
            auto srcRange = (*decl)->GetRange();
            if (auto funcDecl = (*decl)->As<AstFunctionDecl>(); funcDecl) {
                srcRange = funcDecl->GetName().range;
            }
            else if (auto paramDecl = (*decl)->As<AstParamDecl>(); paramDecl && paramDecl->GetDeclTok()) {
                srcRange = paramDecl->GetDeclTok()->range;
            }
            // FIXME:
            // else if (auto varDecl = (*decl)->As<AstVariableDecl>(); varDecl) {
            // }

            auto locBegin = compiler.GetLexContext().LookupSyntaxLocation(srcRange.begin);
            auto locEnd   = compiler.GetLexContext().LookupSyntaxLocation(srcRange.end);

            return {lsp::Location{
                .uri   = uri,
                .range = TextRange::ToLspRange(TextRange{
                    .start = {.line = locBegin.line, .character = locBegin.column},
                    .end   = {.line = locEnd.line, .character = locEnd.column},
                }),
            }};
        }
        else {
            return {};
        }

        return {};
    }

} // namespace glsld