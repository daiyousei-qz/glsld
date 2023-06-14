#include "CodeCompletion.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

// FIXME: Currently, this is implemented as:
//        - all library decl
//        - all global decl
//        - all local decl in the same function
//        Perhaps need improvement or good enough?

namespace glsld
{
    struct CompletionTypeResult
    {
        // If the completion expects an expression
        bool allowExpr = false;

        // If the completion expects a type
        bool allowType = false;

        AstExpr* accessChainExpr = nullptr;
    };

    class CompletionTypeDecider : public LanguageQueryVisitor<CompletionTypeDecider>
    {
    public:
        CompletionTypeDecider(const LanguageQueryProvider& provider, TextPosition cursorPosition)
            : LanguageQueryVisitor(provider), cursorPosition(cursorPosition)
        {
        }

        auto Execute() -> CompletionTypeResult
        {
            TraverseGlobalDeclUntil(cursorPosition);

            CompletionTypeResult result;
            if (inDeclarator) {
                result.allowExpr = false;
                result.allowType = false;
            }
            else if (accessChainExpr) {
                result.allowExpr       = false;
                result.allowType       = false;
                result.accessChainExpr = accessChainExpr;
            }
            else if (inFunctionDefinition) {
                result.allowExpr = true;
                result.allowType = true;
            }
            else if (inStructDefinition) {
                result.allowExpr = false;
                result.allowType = true;
            }

            return result;
        }

        auto EnterAstNodeBase(AstNodeBase& node) -> AstVisitPolicy
        {
            if (GetProvider().ContainsPositionExtended(node, cursorPosition)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
        {
            if (decl.GetBody() && GetProvider().ContainsPosition(*decl.GetBody(), cursorPosition)) {
                inFunctionDefinition = true;
            }
            else {
                // If a parameter decl is already parsed, the parameter type is already well-formed.
                for (auto paramDecl : decl.GetParams()) {
                    // FIXME: what about cursor?
                    if (GetProvider().ContainsPositionExtended(*paramDecl, cursorPosition)) {
                        inDeclarator = true;
                    }
                }
            }
        }

        auto VisitAstStructDecl(AstStructDecl& decl) -> void
        {
            if (decl.GetDeclToken() && GetProvider().ContainsPositionExtended(*decl.GetDeclToken(), cursorPosition)) {
                inDeclarator = true;
            }

            // FIXME: if cursor is outside the braces
            inStructDefinition = true;
        }

        auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
        {
            // FIXME: test if cursor is in the instance name
            inStructDefinition = true;
        }

        auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
        {
            // FIXME: layout?
            for (const auto& declarator : decl.GetDeclarators()) {
                TestDeclarator(declarator);
            }
        }

        auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
        {
            for (const auto& declarator : decl.GetDeclarators()) {
                TestDeclarator(declarator);
            }
        }

        auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
        {
            if (expr.GetAccessChain() &&
                GetProvider().ContainsPositionExtended(expr.GetLastTokenIndex(), cursorPosition)) {
                accessChainExpr = expr.GetAccessChain();
            }
        }

    private:
        auto TestDeclarator(const VariableDeclarator& declarator) -> void
        {
            if (GetProvider().ContainsPositionExtended(declarator.declTok, cursorPosition)) {
                inDeclarator = true;
            }
        }

        bool inDeclarator         = false;
        bool inFunctionDefinition = false;
        bool inStructDefinition   = false;
        AstExpr* accessChainExpr  = nullptr;

        TextPosition cursorPosition;
    };

    template <typename F>
        requires std::invocable<F, const SyntaxToken&, lsp::CompletionItemKind>
    auto CollectCompletionFromDecl(F&& callback, AstDecl& decl)
    {
        if (auto funcDecl = decl.As<AstFunctionDecl>(); funcDecl) {
            callback(funcDecl->GetName(), lsp::CompletionItemKind::Function);
        }
        else if (auto paramDecl = decl.As<AstParamDecl>(); paramDecl) {
            if (paramDecl->GetDeclarator()) {
                callback(paramDecl->GetDeclarator()->declTok, lsp::CompletionItemKind::Variable);
            }
        }
        else if (auto varDecl = decl.As<AstVariableDecl>(); varDecl) {
            for (const auto& declarator : varDecl->GetDeclarators()) {
                callback(declarator.declTok, lsp::CompletionItemKind::Variable);
            }

            if (auto structDecl = varDecl->GetType()->GetStructDecl(); structDecl) {
                if (structDecl->GetDeclToken()) {
                    callback(*structDecl->GetDeclToken(), lsp::CompletionItemKind::Struct);
                }
            }
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>(); blockDecl) {
            if (blockDecl->GetDeclarator()) {
                callback(blockDecl->GetDeclarator()->declTok, lsp::CompletionItemKind::Variable);
            }
            else {
                for (auto memberDecl : blockDecl->GetMembers()) {
                    for (const auto& declarator : memberDecl->GetDeclarators()) {
                        callback(declarator.declTok, lsp::CompletionItemKind::Variable);
                    }
                }
            }
        }
        else if (decl.Is<AstEmptyDecl>() || decl.Is<AstStructDecl>()) {
            // Do nothing here.
            // NOTE we don't have struct decl in our AST. Instead, those are hidden in the variable decl.
        }
        else {
            GLSLD_UNREACHABLE();
        }
    }

    class CompletionCollector : public LanguageQueryVisitor<CompletionCollector>
    {
    public:
        CompletionCollector(std::vector<lsp::CompletionItem>& output, const LanguageQueryProvider& provider,
                            CompletionTypeResult completionType, TextPosition cursorPosition)
            : LanguageQueryVisitor(provider), output(output), completionType(completionType),
              cursorPosition(cursorPosition)
        {
        }

        auto Execute() -> void
        {
            TraverseGlobalDeclUntil(cursorPosition);
        }

    protected:
        auto EnterAstDecl(AstDecl& decl) -> AstVisitPolicy
        {
            if (decl.Is<AstFunctionDecl>() && GetProvider().ContainsPosition(decl, cursorPosition)) {
                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Visit;
            }
        }

        auto EnterAstStmt(AstStmt& stmt) -> AstVisitPolicy
        {
            if (stmt.Is<AstCompoundStmt>() && GetProvider().PrecedesPosition(stmt, cursorPosition)) {
                return AstVisitPolicy::Leave;
            }

            return AstVisitPolicy::Traverse;
        }

        auto VisitAstDecl(AstDecl& decl) -> void
        {
            CollectCompletionFromDecl(
                [this](const SyntaxToken& token, lsp::CompletionItemKind kind) { TryAddCompletionItem(token, kind); },
                decl);
        }

    private:
        auto TryAddCompletionItem(const SyntaxToken& declTok, lsp::CompletionItemKind kind) -> void
        {
            switch (kind) {
            // We only populate completion items of these kinds
            case lsp::CompletionItemKind::Function:
            case lsp::CompletionItemKind::Variable:
                if (!completionType.allowExpr) {
                    return;
                }
                else {
                    break;
                }
            case lsp::CompletionItemKind::Struct:
                if (!completionType.allowType) {
                    return;
                }
                else {
                    break;
                }
            default:
                return;
            }

            if (declTok.IsIdentifier()) {
                if (auto it = itemIndexMap.find(declTok.text); it != itemIndexMap.end()) {
                    output[it->second].kind = kind;
                }
                else {
                    output.push_back({lsp::CompletionItem{
                        .label = declTok.text.Str(),
                        .kind  = kind,
                    }});
                    itemIndexMap[declTok.text] = output.size() - 1;
                }
            }
        }

        std::vector<lsp::CompletionItem>& output;
        CompletionTypeResult completionType;
        TextPosition cursorPosition;

        std::unordered_map<AtomString, size_t> itemIndexMap;
    };

    auto GetDefaultLibraryCompletionList() -> ArrayView<lsp::CompletionItem>
    {
        static const auto cachedCompletionItems = []() {
            std::vector<lsp::CompletionItem> result;
            std::unordered_set<AtomString> seenIds;

            // Builtins
            for (AstDecl* decl : GetStandardLibraryModule()->GetAstContext().GetGlobalDecls()) {
                CollectCompletionFromDecl(
                    [&](const SyntaxToken& declTok, lsp::CompletionItemKind kind) {
                        if (seenIds.find(declTok.text) == seenIds.end()) {
                            seenIds.insert(declTok.text);
                            result.push_back({lsp::CompletionItem{
                                .label = declTok.text.Str(),
                                .kind  = kind,
                            }});
                        }
                    },
                    *decl);
            }

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

    auto GetSwizzleCompletionItem() -> ArrayView<lsp::CompletionItem>
    {
        static const auto cachedCompletionItems = []() {
            std::vector<lsp::CompletionItem> result;

            // FIXME: multi-dimensional swizzle?
            static constexpr StringView swizzles = "xyzwrgbapqst";
            for (char ch : swizzles) {
                result.push_back({lsp::CompletionItem{
                    .label = std::string{ch},
                    .kind  = lsp::CompletionItemKind::Property,
                }});
            }

            return result;
        }();

        return cachedCompletionItems;
    }

    auto ComputeCompletion(const LanguageQueryProvider& provider, lsp::Position lspPosition)
        -> std::vector<lsp::CompletionItem>
    {
        const auto& compilerObject = provider.GetCompilerObject();

        auto cursorPosition = FromLspPosition(lspPosition);

        std::vector<lsp::CompletionItem> result;
        auto completionType = CompletionTypeDecider{provider, cursorPosition}.Execute();
        if (completionType.accessChainExpr) {
            auto type = completionType.accessChainExpr->GetDeducedType();
            if (type->IsArray() || type->IsVector()) {
                // .length operator
                result.push_back({lsp::CompletionItem{
                    .label = "length",
                    .kind  = lsp::CompletionItemKind::Method,
                }});
            }
            if (type->IsScalar() || type->IsVector() || type->IsMatrix()) {
                std::ranges::copy(GetSwizzleCompletionItem(), std::back_inserter(result));
            }
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
        else {
            // Copy the completion items from the language and standard library
            std::ranges::copy_if(GetDefaultLibraryCompletionList(), std::back_inserter(result),
                                 [&](const lsp::CompletionItem& item) -> bool {
                                     switch (item.kind) {
                                     case lsp::CompletionItemKind::Struct:
                                         return completionType.allowType;
                                     case lsp::CompletionItemKind::Function:
                                     case lsp::CompletionItemKind::Variable:
                                         return completionType.allowExpr;
                                     case lsp::CompletionItemKind::Keyword:
                                         return true;
                                     default:
                                         return false;
                                     }
                                 });

            // Add the completion items from the AST
            if (completionType.allowExpr || completionType.allowType) {
                CompletionCollector{result, provider, completionType, cursorPosition}.Execute();
            }

            // FIXME: add the completion items from the preprocessor, aka. macros
        }

        return result;
    }
} // namespace glsld