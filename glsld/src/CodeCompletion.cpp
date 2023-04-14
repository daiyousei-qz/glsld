#include "LanguageService.h"
#include "ModuleVisitor.h"

// FIXME: Currently, this is implemented as:
//        - all library decl
//        - all global decl
//        - all local decl in the same function
//        Perhaps need improvement or good enough?

namespace glsld
{
    enum class CompletionType
    {
        None,

        // Typing in an environment where expression is not allowed
        // For example, inside a struct definition
        NoExpr,

        // Typing in an environment where expression is allowed
        // For example, inside a function definition
        AllowExpr,

        // Assuming AllowExpr, typing a.xxx
        AccessChain,
    };

    class CompletionTypeDecider : public ModuleVisitor<CompletionTypeDecider>
    {
    public:
        using ModuleVisitor::ModuleVisitor;

        auto Execute(TextPosition editPosition) -> CompletionType
        {
            this->inFunctionDefinition = false;
            this->inStructDefinition   = false;
            this->inBlockDefinition    = false;
            this->editPosition         = editPosition;

            this->Traverse();

            if (inStructDefinition || inBlockDefinition) {
                return CompletionType::NoExpr;
            }
            else if (inFunctionDefinition) {
                return CompletionType::AllowExpr;
            }
            else {
                return CompletionType::None;
            }
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            return this->EnterIfContainsPosition(node, editPosition);
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            if (decl.GetBody() && this->NodeContainPosition(*decl.GetBody(), editPosition)) {
                inFunctionDefinition = true;
            }
        }

        auto VisitAstStructDecl(AstStructDecl& decl) -> void
        {
            inStructDefinition = true;
        }

        auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
        {
            inBlockDefinition = true;
        }

    private:
        bool inFunctionDefinition = false;
        bool inStructDefinition   = false;
        bool inBlockDefinition    = false;

        TextPosition editPosition;
    };

    class AccessChainExprFinder : public ModuleVisitor<AccessChainExprFinder>
    {
    public:
        using ModuleVisitor::ModuleVisitor;

        auto Execute(SyntaxToken dotToken) -> AstNameAccessExpr*
        {
            this->result   = nullptr;
            this->dotToken = dotToken;

            this->Traverse();

            return result;
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            if (node.GetSyntaxRange().startTokenIndex <= dotToken.index &&
                node.GetSyntaxRange().endTokenIndex > dotToken.index) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            result = &expr;
        }

    private:
        AstNameAccessExpr* result;
        SyntaxToken dotToken;
    };

    class GenericCompletionCollector : public ModuleVisitor<GenericCompletionCollector>
    {
    public:
        using ModuleVisitor::ModuleVisitor;

        auto Execute(CompletionType type, TextPosition editPosition, bool externalModule)
            -> std::vector<lsp::CompletionItem>
        {
            this->type           = type;
            this->editPosition   = editPosition;
            this->externalModule = externalModule;

            this->Traverse();

            return std::move(result);
        }

        auto EnterAstDecl(AstDecl& decl) -> AstVisitPolicy
        {
            if (externalModule || NodePrecedesPosition(decl, editPosition)) {
                return AstVisitPolicy::Visit;
            }
            else if (NodeContainPosition(decl, editPosition)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            if (type != CompletionType::AllowExpr) {
                return;
            }

            TryAddCompletionItem(decl.GetName(), lsp::CompletionItemKind::Function);
        }

        auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
        {
            if (type != CompletionType::AllowExpr) {
                return;
            }

            if (decl.GetDeclarator()) {
                // FIXME: is this a variable?
                TryAddCompletionItem(decl.GetDeclarator()->declTok, lsp::CompletionItemKind::Variable);
            }
        }

        auto VisitAstParamDecl(AstParamDecl& decl) -> void
        {
            if (type != CompletionType::AllowExpr) {
                return;
            }

            if (decl.GetDeclarator()) {
                TryAddCompletionItem(decl.GetDeclarator()->declTok, lsp::CompletionItemKind::Variable);
            }
        }

        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            if (type != CompletionType::AllowExpr) {
                return;
            }

            for (const auto& declarator : decl.GetDeclarators()) {
                TryAddCompletionItem(declarator.declTok, lsp::CompletionItemKind::Variable);
            }
        }

        auto VisitAstStructDecl(AstStructDecl& decl) -> void
        {
            if (decl.GetDeclToken()) {
                TryAddCompletionItem(*decl.GetDeclToken(), lsp::CompletionItemKind::Struct);
            }
        }

    private:
        auto TryAddCompletionItem(const SyntaxToken& declTok, lsp::CompletionItemKind kind) -> void
        {
            if (declTok.IsIdentifier()) {
                if (auto it = itemIndexMap.find(declTok.text); it != itemIndexMap.end()) {
                    result[it->second].kind = kind;
                }
                else {
                    result.push_back({lsp::CompletionItem{
                        .label = declTok.text.Str(),
                        .kind  = kind,
                    }});
                    itemIndexMap[declTok.text] = result.size() - 1;
                }
            }
        }

        bool externalModule;

        CompletionType type;
        TextPosition editPosition;

        std::unordered_map<AtomString, size_t> itemIndexMap;
        std::vector<lsp::CompletionItem> result;
    };

    auto GetDefaultLibraryCompletionList() -> std::vector<lsp::CompletionItem>
    {
        static const auto cachedCompletionItems = []() {
            std::vector<lsp::CompletionItem> result;

            // Builtins
            result = GenericCompletionCollector{*GetStandardLibraryModule()}.Execute(CompletionType::AllowExpr,
                                                                                     TextPosition{}, true);
            // Keywords
            for (auto [keywordKlass, keywordText] : GetAllKeywords()) {
                result.push_back(lsp::CompletionItem{
                    .label = std::string{keywordText},
                    .kind  = lsp::CompletionItemKind::Keyword,
                });
            }

            return result;
        }();

        return cachedCompletionItems;
    }

    // FIXME: do we really need swizzle?
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

    auto FindAccessChainExpr(const CompilerObject& compilerObject, TextPosition editPosition) -> AstNameAccessExpr*
    {
        auto lastToken = compilerObject.GetLexContext().FindTokenByTextPosition(editPosition);
        if (lastToken.klass == TokenKlass::Dot) {
            return AccessChainExprFinder{compilerObject}.Execute(lastToken);
        }
        else if (lastToken.klass == TokenKlass::Identifier && lastToken.index > 0) {
            if (compilerObject.GetLexContext().GetToken(lastToken.index - 1).klass == TokenKlass::Dot) {
                return AccessChainExprFinder{compilerObject}.Execute(lastToken);
            }
        }

        return nullptr;
    }

    auto ComputeCompletion(const CompilerObject& compilerObject, lsp::Position lspPosition)
        -> std::vector<lsp::CompletionItem>
    {
        auto editPosition = FromLspPosition(lspPosition);

        AstNameAccessExpr* nameAccessExpr = nullptr;
        CompletionType completionType     = CompletionType::None;
        if ((nameAccessExpr = FindAccessChainExpr(compilerObject, editPosition))) {
            completionType = CompletionType::AccessChain;
        }
        else {
            completionType = CompletionTypeDecider{compilerObject}.Execute(editPosition);
        }

        std::vector<lsp::CompletionItem> result;
        if (completionType == CompletionType::AccessChain) {
            GLSLD_ASSERT(nameAccessExpr);
            if (!nameAccessExpr->GetAccessChain()) {
                return result;
            }

            auto type = nameAccessExpr->GetAccessChain()->GetDeducedType();
            if (type->IsArray() || type->IsVector()) {
                // .length operator
                result.push_back({lsp::CompletionItem{
                    .label = "length",
                    .kind  = lsp::CompletionItemKind::Method,
                }});
            }
            // if (type->IsScalar() || type->IsVector() || type->IsMatrix()) {
            //     // FIXME: swizzle?
            // }
            if (auto structDesc = type->GetStructDesc()) {
                for (const auto& [memberName, memberType] : structDesc->members) {
                    result.push_back({lsp::CompletionItem{
                        .label = memberName,
                        .kind  = lsp::CompletionItemKind::Field,
                    }});
                }
            }

            // FIXME: handle other type
        }
        else if (completionType != CompletionType::None) {

            // Copy the completion items from the language and standard library
            std::ranges::copy_if(GetDefaultLibraryCompletionList(), std::back_inserter(result),
                                 [completionType](const lsp::CompletionItem& item) -> bool {
                                     if (completionType == CompletionType::AllowExpr) {
                                         return true;
                                     }
                                     else {
                                         GLSLD_ASSERT(completionType == CompletionType::NoExpr);
                                         switch (item.kind) {
                                         case lsp::CompletionItemKind::Struct:
                                         case lsp::CompletionItemKind::Keyword:
                                             return true;
                                         default:
                                             return false;
                                         }
                                     }
                                 });

            // Add the completion items from the AST
            std::ranges::copy(GenericCompletionCollector{compilerObject}.Execute(completionType, editPosition, false),
                              std::back_inserter(result));

            // FIXME: add the completion items from the preprocessor, aka. macros
        }

        return result;
    }
} // namespace glsld