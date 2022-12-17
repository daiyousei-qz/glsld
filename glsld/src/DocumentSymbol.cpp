#include "LanguageService.h"

namespace glsld
{
    auto ComputeDocumentSymbol(const CompileResult& compileResult) -> std::vector<lsp::DocumentSymbol>
    {
        std::vector<lsp::DocumentSymbol> result;

        for (auto decl : compileResult.GetAstContext().globalDecls) {
            auto declRange       = compileResult.GetLexContext().LookupTextRange(decl->GetRange());
            lsp::Range testRange = ToLspRange(declRange);

            if (auto funcDecl = decl->As<AstFunctionDecl>()) {
                if (funcDecl->GetName().klass == TokenKlass::Identifier) {
                    result.push_back(lsp::DocumentSymbol{
                        .name           = funcDecl->GetName().text.Str(),
                        .kind           = lsp::SymbolKind::Function,
                        .range          = testRange,
                        .selectionRange = testRange,
                    });
                }
            }
            else if (auto varDecl = decl->As<AstVariableDecl>()) {
                for (const auto& declarator : varDecl->GetDeclarators()) {
                    if (declarator.declTok.klass == TokenKlass::Identifier) {
                        result.push_back(lsp::DocumentSymbol{
                            .name           = declarator.declTok.text.Str(),
                            .kind           = lsp::SymbolKind::Variable,
                            .range          = testRange,
                            .selectionRange = testRange,
                        });
                    }
                }
            }
        }

        return result;
    }

} // namespace glsld