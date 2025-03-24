#include "Feature/DocumentSymbol.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"

namespace glsld
{
    namespace
    {
        class DocumentSymbolCollector : public LanguageQueryVisitor<DocumentSymbolCollector>
        {
        private:
            std::vector<lsp::DocumentSymbol> result;

            auto TryAddSymbol(AstSyntaxToken token, lsp::SymbolKind kind) -> bool
            {
                return TryAddSymbol(result, token, kind);
            }

            auto TryAddSymbol(std::vector<lsp::DocumentSymbol>& buffer, AstSyntaxToken token, lsp::SymbolKind kind)
                -> bool
            {
                if (!token.IsIdentifier()) {
                    return false;
                }

                if (auto spelledRange = GetInfo().LookupSpelledTextRangeInMainFile(token.id)) {
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

            auto TryAddStructMembers(std::vector<lsp::DocumentSymbol>& buffer,
                                     ArrayView<const AstStructFieldDecl*> memberDecls, lsp::SymbolKind kind) -> void
            {
                for (auto memberDecl : memberDecls) {
                    for (const auto& declarator : memberDecl->GetDeclarators()) {
                        TryAddSymbol(buffer, declarator.nameToken, kind);
                    }
                }
            }

            auto TryAddBlockMembers(std::vector<lsp::DocumentSymbol>& buffer,
                                    ArrayView<const AstBlockFieldDecl*> memberDecls, lsp::SymbolKind kind) -> void
            {
                for (auto memberDecl : memberDecls) {
                    for (const auto& declarator : memberDecl->GetDeclarators()) {
                        TryAddSymbol(buffer, declarator.nameToken, kind);
                    }
                }
            }

        public:
            DocumentSymbolCollector(const LanguageQueryInfo& info) : LanguageQueryVisitor(info)
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
                TryAddSymbol(decl.GetNameToken(), lsp::SymbolKind::Function);
            }

            auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
            {
                if (auto structDecl = decl.GetQualType()->GetStructDecl()) {
                    if (structDecl->GetNameToken()) {
                        if (TryAddSymbol(*structDecl->GetNameToken(), lsp::SymbolKind::Struct)) {
                            TryAddStructMembers(result.back().children, structDecl->GetMembers(),
                                                lsp::SymbolKind::Field);
                        }
                    }
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    TryAddSymbol(declarator.nameToken, lsp::SymbolKind::Variable);
                }
            }

            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
            {
                if (decl.GetDeclarator()) {
                    // Named block
                    // FIXME: should block name be added as a symbol?
                    if (TryAddSymbol(decl.GetDeclarator()->nameToken, lsp::SymbolKind::Variable)) {
                        TryAddBlockMembers(result.back().children, decl.GetMembers(), lsp::SymbolKind::Field);
                    }
                }
                else {
                    // Unnamed block.
                    // We add members to global scope as if they are global variables.
                    TryAddBlockMembers(result, decl.GetMembers(), lsp::SymbolKind::Variable);
                }
            }
        };
    } // namespace

    auto GetDocumentSymbolOptions(const DocumentSymbolConfig& config) -> lsp::DocumentSymbolOptions
    {
        return lsp::DocumentSymbolOptions{};
    }

    auto HandleDocumentSymbol(const DocumentSymbolConfig& config, const LanguageQueryInfo& info,
                              const lsp::DocumentSymbolParams& params) -> std::vector<lsp::DocumentSymbol>
    {
        return DocumentSymbolCollector{info}.Execute();
    }

} // namespace glsld