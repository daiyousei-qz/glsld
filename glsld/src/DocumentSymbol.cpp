#include "DocumentSymbol.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

namespace glsld
{
    // FIXME: Support struct member
    class DocumentSymbolCollector : public LanguageQueryVisitor<DocumentSymbolCollector>
    {
    public:
        DocumentSymbolCollector(const LanguageQueryProvider& provider) : LanguageQueryVisitor(provider)
        {
        }

        auto Execute() -> std::vector<lsp::DocumentSymbol>
        {
            TraverseAllGlobalDecl();
            return std::move(result);
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            // Only visit global decl
            return AstVisitPolicy::Visit;
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            TryAddSymbol(decl.GetName(), lsp::SymbolKind::Function);
        }

        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            if (auto structDecl = decl.GetType()->GetStructDecl()) {
                if (structDecl->GetDeclToken()) {
                    if (TryAddSymbol(*structDecl->GetDeclToken(), lsp::SymbolKind::Struct)) {
                        for (auto memberDecl : structDecl->GetMembers()) {
                            for (const auto& declarator : memberDecl->GetDeclarators()) {
                                TryAddSymbol(result.back().children, declarator.declTok, lsp::SymbolKind::Field);
                            }
                        }
                    }
                }
            }

            for (const auto& declarator : decl.GetDeclarators()) {
                TryAddSymbol(declarator.declTok, lsp::SymbolKind::Variable);
            }
        }

        auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
        {
            if (decl.GetDeclarator()) {
                // Named block
                if (TryAddSymbol(decl.GetDeclarator()->declTok, lsp::SymbolKind::Variable)) {
                    for (auto memberDecl : decl.GetMembers()) {
                        for (const auto& declarator : memberDecl->GetDeclarators()) {
                            TryAddSymbol(result.back().children, declarator.declTok, lsp::SymbolKind::Field);
                        }
                    }
                }
            }
            else {
                // Anonymous block
                for (auto blockMember : decl.GetMembers()) {
                    for (const auto& declarator : blockMember->GetDeclarators()) {
                        TryAddSymbol(declarator.declTok, lsp::SymbolKind::Variable);
                    }
                }
            }
        }

    private:
        auto TryAddSymbol(SyntaxToken token, lsp::SymbolKind kind) -> bool
        {
            return TryAddSymbol(result, token, kind);
        }

        auto TryAddSymbol(std::vector<lsp::DocumentSymbol>& buffer, SyntaxToken token, lsp::SymbolKind kind) -> bool
        {
            if (token.IsIdentifier() && GetProvider().InMainFile(token)) {
                auto tokRange = ToLspRange(GetProvider().GetLexContext().LookupSpelledTextRange(token));
                buffer.push_back(lsp::DocumentSymbol{
                    .name           = token.text.Str(),
                    .kind           = kind,
                    .range          = tokRange,
                    .selectionRange = tokRange,
                    .children       = {},
                });

                return true;
            }

            return false;
        }

        std::vector<lsp::DocumentSymbol> result;
    };

    auto ComputeDocumentSymbol(const LanguageQueryProvider& provider) -> std::vector<lsp::DocumentSymbol>
    {
        return DocumentSymbolCollector{provider}.Execute();
    }

} // namespace glsld