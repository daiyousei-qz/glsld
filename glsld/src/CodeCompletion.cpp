#include "LanguageService.h"

namespace glsld
{
    class CompletionVisitor : public AstVisitor<CompletionVisitor>
    {
    public:
        CompletionVisitor(const LexContext& lexContext, TextPosition position, bool externalModule)
            : lexContext(lexContext), position(position), externalModule(externalModule)
        {
        }

        auto EnterAstFunctionDecl(AstFunctionDecl& decl) -> AstVisitPolicy
        {
            // NOTE we only collect global symbols for external module
            if (externalModule) {
                return AstVisitPolicy::Visit;
            }

            auto locBegin = lexContext.LookupSyntaxLocation(decl.GetRange().begin);
            auto locEnd   = lexContext.LookupSyntaxLocation(decl.GetRange().end);
            auto range    = TextRange{
                   .start = {.line = locBegin.line, .character = locBegin.column},
                   .end   = {.line = locEnd.line, .character = locEnd.column},
            };

            if (range.ContainPosition(position)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Visit;
            }
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            if (decl.GetName().klass == TokenKlass::Identifier) {
                AddCompletionItem(decl.GetName().text.Str(), lsp::CompletionItemKind::Function);
            }
        }

        auto VisitAstParamDecl(AstParamDecl& decl) -> void
        {
            if (decl.GetDeclTok() && decl.GetDeclTok()->klass == TokenKlass::Identifier) {
                AddCompletionItem(decl.GetDeclTok()->text.Str(), lsp::CompletionItemKind::Variable);
            }
        }

        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            for (const auto& declarator : decl.GetDeclarators()) {
                if (declarator.declTok.klass == TokenKlass::Identifier) {
                    AddCompletionItem(declarator.declTok.text.Str(), lsp::CompletionItemKind::Variable);
                }
            }
        }

        auto Export() -> std::vector<lsp::CompletionItem>
        {
            return std::move(result);
        }

    private:
        auto AddCompletionItem(std::string label, lsp::CompletionItemKind kind) -> void
        {
            // We do deduplicate for external modules
            if (externalModule) {
                if (addedSymbol.find(label) != addedSymbol.end()) {
                    return;
                }

                addedSymbol.insert(label);
            }

            result.push_back({lsp::CompletionItem{
                .label = label,
                .kind  = kind,
            }});
        }

        const LexContext& lexContext;
        TextPosition position;
        bool externalModule;
        std::unordered_set<std::string> addedSymbol;

        std::vector<lsp::CompletionItem> result;
    };

    auto GetDefaultLibraryCompletionList() -> std::vector<lsp::CompletionItem>
    {
        static const auto cachedCompletionItems = []() {
            auto defaultLibraryModule = GetDefaultLibraryModule();
            CompletionVisitor visitor{defaultLibraryModule->GetLexContext(), TextPosition{}, true};
            visitor.TraverseAst(defaultLibraryModule->GetAstContext());
            return visitor.Export();
        }();

        return cachedCompletionItems;
    }

    auto ComputeCompletion(CompiledModule& compiler, lsp::Position position) -> std::vector<lsp::CompletionItem>
    {
        auto result = GetDefaultLibraryCompletionList();

        CompletionVisitor visitor{compiler.GetLexContext(), TextPosition::FromLspPosition(position), false};
        visitor.TraverseAst(compiler.GetAstContext());
        std::ranges::copy(visitor.Export(), std::back_inserter(result));

        return result;
    }
} // namespace glsld