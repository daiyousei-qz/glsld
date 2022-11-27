#include "LanguageService.h"

namespace glsld
{
    auto ComputeDocumentSymbol(GlsldCompiler& compiler) -> std::vector<lsp::DocumentSymbol>
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
                result.push_back(lsp::DocumentSymbol{
                    .name           = funcDecl->GetName().text.Str(),
                    .kind           = lsp::SymbolKind::Function,
                    .range          = testRange,
                    .selectionRange = testRange,
                });
            }
            else if (auto varDecl = decl->As<AstVariableDecl>()) {
                for (const auto& declarator : varDecl->GetDeclarators()) {
                    result.push_back(lsp::DocumentSymbol{
                        .name           = declarator.declTok.text.Str(),
                        .kind           = lsp::SymbolKind::Variable,
                        .range          = testRange,
                        .selectionRange = testRange,
                    });
                }
            }
        }

        // for (auto var : compiler.GetAstContext()->GetVarSymbols()) {
        //     result.push_back(lsp::DocumentSymbol{
        //         .name           = var,
        //         .kind           = lsp::SymbolKind::Variable,
        //         .range          = testRange,
        //         .selectionRange = testRange,
        //     });
        // }
        // for (auto f : compiler.GetAstContext()->GetFuncSymbols()) {
        //     result.push_back(lsp::DocumentSymbol{
        //         .name           = f,
        //         .kind           = lsp::SymbolKind::Function,
        //         .range          = testRange,
        //         .selectionRange = testRange,
        //     });
        // }

        return result;
    }

} // namespace glsld