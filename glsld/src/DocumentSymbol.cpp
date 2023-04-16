#include "LanguageService.h"
#include "ModuleVisitor.h"

namespace glsld
{
    // FIXME: Support struct member
    class DocumentSymbolCollector : public ModuleVisitor<DocumentSymbolCollector>
    {
    public:
        using ModuleVisitor::ModuleVisitor;

        auto Execute() -> std::vector<lsp::DocumentSymbol>
        {
            this->Traverse();

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
                    TryAddSymbol(*structDecl->GetDeclToken(), lsp::SymbolKind::Struct);
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
                TryAddSymbol(decl.GetDeclarator()->declTok, lsp::SymbolKind::Variable);
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
        auto TryAddSymbol(SyntaxToken token, lsp::SymbolKind kind) -> void
        {
            if (token.IsIdentifier() &&
                GetLexContext().LookupSpelledFile(token) == GetSourceContext().GetMainFile()->GetID()) {
                auto tokRange = ToLspRange(GetLexContext().LookupSpelledTextRange(token));
                result.push_back(lsp::DocumentSymbol{
                    .name           = token.text.Str(),
                    .kind           = kind,
                    .range          = tokRange,
                    .selectionRange = tokRange,
                    .children       = {},
                });
            }
        }

        std::vector<lsp::DocumentSymbol> result;
    };

    auto ComputeDocumentSymbol(const CompilerObject& compilerObject) -> std::vector<lsp::DocumentSymbol>
    {
        return DocumentSymbolCollector{compilerObject}.Execute();
    }

} // namespace glsld