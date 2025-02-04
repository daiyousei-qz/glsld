#include "CodeCompletion.h"
#include "LanguageQueryVisitor.h"
#include "SourceText.h"

#include <unordered_set>

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

        const AstExpr* accessChainExpr = nullptr;
    };

    class CompletionTypeDecider : public LanguageQueryVisitor<CompletionTypeDecider>
    {
    private:
        // True if the cursor is in a declarator
        bool inDeclarator = false;
        // True if the cursor is in a function definition
        bool inFunctionDefinition = false;
        // True if the cursor is in a struct definition
        bool inStructDefinition = false;
        //
        const AstExpr* accessChainExpr = nullptr;

        TextPosition cursorPosition;

    public:
        CompletionTypeDecider(const LanguageQueryProvider& provider, TextPosition cursorPosition)
            : LanguageQueryVisitor(provider), cursorPosition(cursorPosition)
        {
        }

        auto Execute() -> CompletionTypeResult
        {
            TraverseTranslationUnit();

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

        auto EnterAstNode(AstNode& node) -> AstVisitPolicy
        {
            return TraverseNodeContains(node, cursorPosition);
        }

        auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void
        {
        }
        auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
        {
            if (auto dotTokIndex = GetProvider().LookupDotTokenIndex(expr)) {
                if (GetProvider().ContainsPositionExtended(
                        AstSyntaxRange{*dotTokIndex, expr.GetSyntaxRange().GetEndID()}, cursorPosition)) {
                    // FIXME: this also includes "^.xxx", which is not a valid position.
                    accessChainExpr = expr.GetBaseExpr();
                }
            }
        }
        auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void
        {
            if (auto dotTokIndex = GetProvider().LookupDotTokenIndex(expr)) {
                if (GetProvider().ContainsPositionExtended(
                        AstSyntaxRange{*dotTokIndex, expr.GetSyntaxRange().GetEndID()}, cursorPosition)) {
                    // FIXME: this also includes "^.xxx", which is not a valid position.
                    accessChainExpr = expr.GetBaseExpr();
                }
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
        {
            if (decl.GetBody() && GetProvider().ContainsPosition(*decl.GetBody(), cursorPosition)) {
                inFunctionDefinition = true;
            }
        }

        auto VisitAstParamDecl(const AstParamDecl& decl) -> void
        {
            // FIXME: what about cursor?
            if (GetProvider().ContainsPositionExtended(decl, cursorPosition)) {
                inDeclarator = true;
            }
        }

        auto VisitAstStructDecl(const AstStructDecl& decl) -> void
        {
            if (decl.GetNameToken() && GetProvider().ContainsPositionExtended(*decl.GetNameToken(), cursorPosition)) {
                inDeclarator = true;
            }

            // FIXME: if cursor is outside the braces
            inStructDefinition = true;
        }

        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
        {
            // FIXME: test if cursor is in the instance name
            inStructDefinition = true;
        }

        auto VisitAstFieldDecl(const AstStructFieldDecl& decl) -> void
        {
            // FIXME: layout?
            for (const auto& declarator : decl.GetDeclarators()) {
                TestDeclarator(declarator);
            }
        }

        auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
        {
            for (const auto& declarator : decl.GetDeclarators()) {
                TestDeclarator(declarator);
            }
        }

    private:
        auto TestDeclarator(const Declarator& declarator) -> void
        {
            if (GetProvider().ContainsPositionExtended(declarator.nameToken, cursorPosition)) {
                inDeclarator = true;
            }
        }
    };

    template <typename F>
        requires std::invocable<F, const AstSyntaxToken&, lsp::CompletionItemKind>
    auto CollectCompletionFromDecl(F&& callback, const AstDecl& decl)
    {
        if (auto funcDecl = decl.As<AstFunctionDecl>()) {
            callback(funcDecl->GetNameToken(), lsp::CompletionItemKind::Function);
        }
        else if (auto paramDecl = decl.As<AstParamDecl>()) {
            if (paramDecl->GetDeclarator()) {
                callback(paramDecl->GetDeclarator()->nameToken, lsp::CompletionItemKind::Variable);
            }
        }
        else if (auto varDecl = decl.As<AstVariableDecl>()) {
            for (const auto& declarator : varDecl->GetDeclarators()) {
                callback(declarator.nameToken, lsp::CompletionItemKind::Variable);
            }

            if (auto structDecl = varDecl->GetQualType()->GetStructDecl()) {
                if (structDecl->GetNameToken()) {
                    callback(*structDecl->GetNameToken(), lsp::CompletionItemKind::Struct);
                }
            }
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
            if (blockDecl->GetDeclarator()) {
                callback(blockDecl->GetDeclarator()->nameToken, lsp::CompletionItemKind::Variable);
            }
            else {
                for (auto memberDecl : blockDecl->GetMembers()) {
                    for (const auto& declarator : memberDecl->GetDeclarators()) {
                        callback(declarator.nameToken, lsp::CompletionItemKind::Variable);
                    }
                }
            }
        }
        else {
            // Do nothing here.
            // NOTE we don't have struct decl in our AST. Instead, those are hidden in the variable decl.
        }
    }

    class CompletionCollector : public LanguageQueryVisitor<CompletionCollector>
    {
    private:
        std::vector<lsp::CompletionItem>& output;
        CompletionTypeResult completionType;
        TextPosition cursorPosition;

        std::unordered_map<AtomString, size_t> itemIndexMap;

    public:
        CompletionCollector(std::vector<lsp::CompletionItem>& output, const LanguageQueryProvider& provider,
                            CompletionTypeResult completionType, TextPosition cursorPosition)
            : LanguageQueryVisitor(provider), output(output), completionType(completionType),
              cursorPosition(cursorPosition)
        {
        }

        auto Execute() -> void
        {
            TraverseTranslationUnit();
        }

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy
        {
            if (node.Is<AstTranslationUnit>()) {
                return AstVisitPolicy::Traverse;
            }
            else if (auto decl = node.As<AstDecl>()) {
                if (decl->Is<AstFunctionDecl>() && GetProvider().ContainsPosition(*decl, cursorPosition)) {
                    return AstVisitPolicy::Traverse;
                }
                else {
                    return AstVisitPolicy::Visit;
                }
            }
            else if (auto stmt = node.As<AstStmt>()) {
                if (stmt->Is<AstCompoundStmt>() && GetProvider().PrecedesPosition(*stmt, cursorPosition)) {
                    return AstVisitPolicy::Leave;
                }

                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstNode(const AstNode& node) -> void
        {
            if (auto decl = node.As<AstDecl>()) {
                CollectCompletionFromDecl([this](const AstSyntaxToken& token,
                                                 lsp::CompletionItemKind kind) { TryAddCompletionItem(token, kind); },
                                          *decl);
            }
        }

    private:
        auto TryAddCompletionItem(const AstSyntaxToken& declTok, lsp::CompletionItemKind kind) -> void
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
    };

    auto GetDefaultLibraryCompletionList() -> ArrayView<lsp::CompletionItem>
    {
        static const auto cachedCompletionItems = []() {
            std::vector<lsp::CompletionItem> result;
            std::unordered_set<AtomString> seenIds;

            // Builtins
            // FIXME: support user preamble
            for (const AstDecl* decl : GetStdlibModule()->GetSystemPreambleArtifacts().GetAst()->GetGlobalDecls()) {
                CollectCompletionFromDecl(
                    [&](const AstSyntaxToken& declTok, lsp::CompletionItemKind kind) {
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

    auto ComputeCompletion(const LanguageQueryProvider& provider, lsp::Position lspPosition)
        -> std::vector<lsp::CompletionItem>
    {
        const auto& compilerObject = provider.GetCompilerResult();

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
            else if (auto structDesc = type->GetStructDesc()) {
                for (const auto& memberDesc : structDesc->members) {
                    result.push_back({lsp::CompletionItem{
                        .label = memberDesc.name,
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