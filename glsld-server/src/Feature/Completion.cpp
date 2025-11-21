#include "Feature/Completion.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/SourceText.h"

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

    class CompletionTypeDecider : public LanguageQueryVisitor<CompletionTypeDecider, CompletionTypeResult>
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
        CompletionTypeDecider(const LanguageQueryInfo& info, TextPosition cursorPosition)
            : LanguageQueryVisitor(info), cursorPosition(cursorPosition)
        {
        }

        auto Finish() -> CompletionTypeResult GLSLD_AST_VISITOR_OVERRIDE
        {
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

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy GLSLD_AST_VISITOR_OVERRIDE
        {
            return TraverseNodeContains(node, cursorPosition);
        }

        auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
        }
        auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (auto dotTokIndex = GetInfo().LookupDotTokenIndex(expr)) {
                if (GetInfo().ContainsPositionExtended(AstSyntaxRange{*dotTokIndex, expr.GetSyntaxRange().GetEndID()},
                                                       cursorPosition)) {
                    // FIXME: this also includes "^.xxx", which is not a valid position.
                    accessChainExpr = expr.GetBaseExpr();
                }
            }
        }
        auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (auto dotTokIndex = GetInfo().LookupDotTokenIndex(expr)) {
                if (GetInfo().ContainsPositionExtended(AstSyntaxRange{*dotTokIndex, expr.GetSyntaxRange().GetEndID()},
                                                       cursorPosition)) {
                    // FIXME: this also includes "^.xxx", which is not a valid position.
                    accessChainExpr = expr.GetBaseExpr();
                }
            }
        }

        auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (decl.GetBody() && GetInfo().ContainsPosition(*decl.GetBody(), cursorPosition)) {
                inFunctionDefinition = true;
            }
        }

        auto VisitAstParamDecl(const AstParamDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            // FIXME: what about cursor?
            if (GetInfo().ContainsPositionExtended(decl, cursorPosition)) {
                inDeclarator = true;
            }
        }

        auto VisitAstStructDecl(const AstStructDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (decl.GetNameToken() && GetInfo().ContainsPositionExtended(*decl.GetNameToken(), cursorPosition)) {
                inDeclarator = true;
            }

            // FIXME: if cursor is outside the braces
            inStructDefinition = true;
        }

        auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            // FIXME: test if cursor is in the instance name
            inStructDefinition = true;
        }

        auto VisitAstStructFieldDeclaratorDecl(const AstStructFieldDeclaratorDecl& decl)
            -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            // FIXME: layout?
            TestDeclarator(Declarator{decl.GetNameToken(), decl.GetArraySpec(), decl.GetInitializer()});
        }

        auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            TestDeclarator(Declarator{decl.GetNameToken(), decl.GetArraySpec(), decl.GetInitializer()});
        }

    private:
        auto TestDeclarator(const Declarator& declarator) -> void
        {
            if (GetInfo().ContainsPositionExtended(declarator.nameToken, cursorPosition)) {
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
            if (auto structDecl = varDecl->GetQualType()->GetStructDecl()) {
                if (structDecl->GetNameToken()) {
                    callback(*structDecl->GetNameToken(), lsp::CompletionItemKind::Struct);
                }
            }

            for (const auto& declaratorDecl : varDecl->GetDeclarators()) {
                callback(declaratorDecl->GetNameToken(), lsp::CompletionItemKind::Variable);
            }
        }
        else if (auto blockDecl = decl.As<AstInterfaceBlockDecl>()) {
            if (blockDecl->GetDeclarator()) {
                callback(blockDecl->GetDeclarator()->nameToken, lsp::CompletionItemKind::Variable);
            }
            else {
                for (auto memberDecl : blockDecl->GetMembers()) {
                    for (const auto& declarator : memberDecl->GetDeclarators()) {
                        callback(declarator->GetNameToken(), lsp::CompletionItemKind::Variable);
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
        CompletionCollector(std::vector<lsp::CompletionItem>& output, const LanguageQueryInfo& info,
                            CompletionTypeResult completionType, TextPosition cursorPosition)
            : LanguageQueryVisitor(info), output(output), completionType(completionType), cursorPosition(cursorPosition)
        {
        }

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy GLSLD_AST_VISITOR_OVERRIDE
        {
            if (node.Is<AstTranslationUnit>()) {
                return AstVisitPolicy::Traverse;
            }
            else if (auto decl = node.As<AstDecl>()) {
                if (decl->Is<AstFunctionDecl>() && GetInfo().ContainsPosition(*decl, cursorPosition)) {
                    return AstVisitPolicy::Traverse;
                }
                else {
                    return AstVisitPolicy::Visit;
                }
            }
            else if (auto stmt = node.As<AstStmt>()) {
                if (stmt->Is<AstCompoundStmt>() && GetInfo().PrecedesPosition(*stmt, cursorPosition)) {
                    return AstVisitPolicy::Leave;
                }

                return AstVisitPolicy::Traverse;
            }
            else {
                return AstVisitPolicy::Leave;
            }
        }

        auto VisitAstNode(const AstNode& node) -> void GLSLD_AST_VISITOR_OVERRIDE
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

    auto ComputeCompletionPreambleInfo(const PrecompiledPreamble& preamble) -> std::unique_ptr<CompletionPreambleInfo>
    {
        std::vector<lsp::CompletionItem> builtinCompletionItems;

        // Builtins
        std::unordered_set<AtomString> seenIds;
        for (const AstDecl* decl : preamble.GetSystemPreambleArtifacts().GetAst()->GetGlobalDecls()) {
            CollectCompletionFromDecl(
                [&](const AstSyntaxToken& declTok, lsp::CompletionItemKind kind) {
                    if (seenIds.find(declTok.text) == seenIds.end()) {
                        seenIds.insert(declTok.text);
                        builtinCompletionItems.push_back(lsp::CompletionItem{
                            .label = declTok.text.Str(),
                            .kind  = kind,
                        });
                    }
                },
                *decl);
        }

        // Keywords
        for (auto [keywordKlass, keywordText] : GetAllKeywords()) {
            builtinCompletionItems.push_back(lsp::CompletionItem{
                .label = std::string{keywordText},
                .kind  = lsp::CompletionItemKind::Keyword,
            });
        }

        return std::make_unique<CompletionPreambleInfo>(std::move(builtinCompletionItems));
    }

    auto GetCompletionOptions(const CompletionConfig& config) -> std::optional<lsp::CompletionOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::CompletionOptions{
            .triggerCharacters = {"."},
        };
    }

    auto HandleCompletion(const CompletionConfig& config, const CompletionPreambleInfo& preambleInfo,
                          const LanguageQueryInfo& queryInfo, const lsp::CompletionParams& params)
        -> std::vector<lsp::CompletionItem>
    {
        if (!config.enable) {
            return {};
        }

        const auto& compilerResult = queryInfo.GetCompilerResult();

        auto cursorPosition = FromLspPosition(params.position);

        std::vector<lsp::CompletionItem> result;
        auto completionType = TraverseAst(CompletionTypeDecider{queryInfo, cursorPosition}, queryInfo.GetUserFileAst());
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
            std::ranges::copy_if(preambleInfo.builtinCompletionItems, std::back_inserter(result),
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
                TraverseAst(CompletionCollector{result, queryInfo, completionType, cursorPosition},
                            queryInfo.GetUserFileAst());
            }

            // FIXME: add the completion items from the preprocessor, aka. macros
        }

        return result;
    }
} // namespace glsld