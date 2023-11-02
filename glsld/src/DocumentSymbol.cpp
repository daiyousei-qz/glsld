#include "DocumentSymbol.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

namespace glsld
{
    class DocumentSymbolCollector : public LanguageQueryVisitor<DocumentSymbolCollector>
    {
    private:
        std::vector<lsp::DocumentSymbol> result;

    public:
        DocumentSymbolCollector(const LanguageQueryProvider& provider) : LanguageQueryVisitor(provider)
        {
        }

        auto Execute() -> std::vector<lsp::DocumentSymbol>
        {
            TraverseTranslationUnit();
            return std::move(result);
        }

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy
        {
            // Only visit global decl
            return AstVisitPolicy::Visit;
        }

        auto EnterAstTranslationUnit(const AstTranslationUnit& tu) -> AstVisitPolicy
        {
            return AstVisitPolicy::Traverse;
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
        {
            TryAddSymbol(decl.GetDeclTok(), lsp::SymbolKind::Function);
        }

        auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
        {
            if (auto structDecl = decl.GetQualType()->GetStructDecl()) {
                if (structDecl->GetDeclTok()) {
                    if (TryAddSymbol(*structDecl->GetDeclTok(), lsp::SymbolKind::Struct)) {
                        TryAddStructMembers(result.back().children, structDecl->GetMembers(), lsp::SymbolKind::Field);
                    }
                }
            }

            for (const auto& declarator : decl.GetDeclarators()) {
                TryAddSymbol(declarator.declTok, lsp::SymbolKind::Variable);
            }
        }

        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
        {
            if (decl.GetDeclarator()) {
                // Named block
                // FIXME: should block name be added as a symbol?
                if (TryAddSymbol(decl.GetDeclarator()->declTok, lsp::SymbolKind::Variable)) {
                    TryAddStructMembers(result.back().children, decl.GetMembers(), lsp::SymbolKind::Field);
                }
            }
            else {
                // Unnamed block.
                // We add members to global scope as if they are global variables.
                TryAddStructMembers(result, decl.GetMembers(), lsp::SymbolKind::Variable);
            }
        }

    private:
        auto TryAddSymbol(SyntaxToken token, lsp::SymbolKind kind) -> bool
        {
            return TryAddSymbol(result, token, kind);
        }

        auto TryAddSymbol(std::vector<lsp::DocumentSymbol>& buffer, SyntaxToken token, lsp::SymbolKind kind) -> bool
        {
            if (!token.IsIdentifier()) {
                return false;
            }

            if (auto spelledRange = GetProvider().GetSpelledTextRangeInMainFile(token)) {
                auto lspSpelledRange = ToLspRange(*spelledRange);
                buffer.push_back(lsp::DocumentSymbol{
                    .name           = token.text.Str(),
                    .kind           = kind,
                    .range          = lspSpelledRange,
                    .selectionRange = lspSpelledRange,
                    .children       = {},
                });

                return true;
            }

            return false;
        }

        auto TryAddStructMembers(std::vector<lsp::DocumentSymbol>& buffer, ArrayView<const AstFieldDecl*> memberDecls,
                                 lsp::SymbolKind kind) -> void
        {
            for (auto memberDecl : memberDecls) {
                for (const auto& declarator : memberDecl->GetDeclarators()) {
                    TryAddSymbol(buffer, declarator.declTok, kind);
                }
            }
        }
    };

    auto ComputeDocumentSymbol(const LanguageQueryProvider& provider) -> std::vector<lsp::DocumentSymbol>
    {
        return DocumentSymbolCollector{provider}.Execute();
    }

} // namespace glsld