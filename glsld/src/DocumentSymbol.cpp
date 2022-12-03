#include "LanguageService.h"

namespace glsld
{
    auto ComputeDocumentSymbol(CompiledModule& compiler) -> std::vector<lsp::DocumentSymbol>
    {
        std::vector<lsp::DocumentSymbol> result;

        for (auto decl : compiler.GetAstContext().globalDecls) {
            auto declBeginLoc = compiler.GetLexContext().LookupSyntaxLocation(decl->GetRange().begin);
            auto declEndLoc   = compiler.GetLexContext().LookupSyntaxLocation(decl->GetRange().end);
            lsp::Range testRange{
                .start =
                    {
                        .line      = static_cast<uint32_t>(declBeginLoc.line),
                        .character = static_cast<uint32_t>(declBeginLoc.column),
                    },
                .end =
                    {
                        .line      = static_cast<uint32_t>(declEndLoc.line),
                        .character = static_cast<uint32_t>(declEndLoc.column),
                    },
            };

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