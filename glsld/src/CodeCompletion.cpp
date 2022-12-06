#include "LanguageService.h"

// FIXME: Currently, this is implemented as:
//        - all library decl
//        - all global decl
//        - all local decl in the same function
//        Perhaps need improvement or good enough?

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

            auto declRange = lexContext.LookupTextRange(decl.GetRange());
            if (declRange.Contains(position)) {
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
            if (decl.GetDeclToken() && decl.GetDeclToken()->klass == TokenKlass::Identifier) {
                AddCompletionItem(decl.GetDeclToken()->text.Str(), lsp::CompletionItemKind::Variable);
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
            std::vector<lsp::CompletionItem> result;

            // Builtins
            auto defaultLibraryModule = GetDefaultLibraryModule();
            CompletionVisitor visitor{defaultLibraryModule->GetLexContext(), TextPosition{}, true};
            visitor.TraverseAst(defaultLibraryModule->GetAstContext());
            result = visitor.Export();

            // Keywords
            for (auto [keywordKlass, keywordText] : GetAllKeywords()) {
                result.push_back(lsp::CompletionItem{
                    .label = std::string{keywordText},
                    .kind  = lsp::CompletionItemKind::Keyword,
                });
            }

            // Swizzle
            for (int i = 0; i < 4; ++i) {
                result.push_back(lsp::CompletionItem{
                    .label = std::string{"xxxx"},
                    .kind  = lsp::CompletionItemKind::Field,
                });
            }

            return result;
        }();

        return cachedCompletionItems;
    }

    auto GetSwizzleCompletionList(size_t n) -> std::vector<lsp::CompletionItem>
    {
        static const auto cachedCompletionItems = []() {
            constexpr StringView swizzleSets[] = {
                "xyzw",
                "rgba",
                "stqp",
            };

            std::array<std::vector<lsp::CompletionItem>, 4> result;

            for (int numSrcComp = 1; numSrcComp <= 4; ++numSrcComp) {
                std::vector<lsp::CompletionItem> tmp;

                for (int n = 0; n < numSrcComp; ++n) {
                    int i = n;
                    for (auto set : swizzleSets) {
                        tmp.push_back(lsp::CompletionItem{
                            .label = std::string{set[i]},
                            .kind  = lsp::CompletionItemKind::Field,
                        });
                    }
                }
                for (int n = 0; n < numSrcComp * numSrcComp; ++n) {
                    int i = n / numSrcComp;
                    int j = n % numSrcComp;
                    for (auto set : swizzleSets) {
                        tmp.push_back(lsp::CompletionItem{
                            .label = std::string{set[i], set[j]},
                            .kind  = lsp::CompletionItemKind::Field,
                        });
                    }
                }
                for (int n = 0; n < numSrcComp * numSrcComp * numSrcComp; ++n) {
                    int i = n / numSrcComp / numSrcComp;
                    int j = n / numSrcComp % numSrcComp;
                    int k = n % numSrcComp;
                    for (auto set : swizzleSets) {
                        tmp.push_back(lsp::CompletionItem{
                            .label = std::string{set[i], set[j], set[k]},
                            .kind  = lsp::CompletionItemKind::Field,
                        });
                    }
                }
                for (int n = 0; n < numSrcComp * numSrcComp * numSrcComp * numSrcComp; ++n) {
                    int i = n / numSrcComp / numSrcComp / numSrcComp;
                    int j = n / numSrcComp / numSrcComp % numSrcComp;
                    int k = n / numSrcComp % numSrcComp;
                    int l = n % numSrcComp;
                    for (auto set : swizzleSets) {
                        tmp.push_back(lsp::CompletionItem{
                            .label = std::string{set[i], set[j], set[k], set[l]},
                            .kind  = lsp::CompletionItemKind::Field,
                        });
                    }
                }

                result[numSrcComp - 1] = std::move(tmp);
            }

            return result;
        }();

        GLSLD_ASSERT(n < 4);
        return cachedCompletionItems[n];
    }

    // FIXME: not populate keyword while typing a decl
    // FIXME: not populate functions while typing a dot
    auto ComputeCompletion(CompiledModule& compiler, lsp::Position position) -> std::vector<lsp::CompletionItem>
    {
        auto result = GetDefaultLibraryCompletionList();

        CompletionVisitor visitor{compiler.GetLexContext(), FromLspPosition(position), false};
        visitor.TraverseAst(compiler.GetAstContext());
        std::ranges::copy(visitor.Export(), std::back_inserter(result));

        return result;
    }
} // namespace glsld