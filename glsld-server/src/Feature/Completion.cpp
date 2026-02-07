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
        // If we may complete with a type name
        bool allowTypeName = false;

        // If we may complete with a function name
        bool allowFunctionName = false;

        // If we may complete with a variable name
        bool allowVariableName = false;

        // Access chain that triggers the completion, if any
        // For example, if a completion is triggered by "expr.^", this field holds the AST node for "expr".
        const AstExpr* accessChainBaseExpr = nullptr;

        // The token that is being replaced by the completion
        // If completion is an insertion, this token is invalid.
        AstSyntaxToken pendingReplacementToken;
    };

    class CompletionTypeDecider : public LanguageQueryVisitor<CompletionTypeDecider, CompletionTypeResult>
    {
    private:
        // True if we should disable all completions at the location
        bool disableAllCompletion = false;
        // True if the cursor is in a function definition
        bool inFunctionDefinition = false;
        // True if the cursor is in a struct definition
        bool inStructDefinition = false;
        // True if the cursor is in a function call
        bool inFunctionCall = false;

        const AstExpr* accessChainBaseExpr = nullptr;

        AstSyntaxToken pendingReplacementToken;

        // The cursor position where completion is requested
        TextPosition cursorPosition;

        auto CheckTokenForReplacement(const AstSyntaxToken& token) -> bool
        {
            if (!token.IsValid()) {
                return false;
            }

            // A token "tok" contains three text positions: "^t^o^k"
            // However, we want to provide a replacement completion at "t^o^k^". Thus, we need to apply an offset.
            auto range = GetInfo().LookupExpandedTextRange(token);
            range.start.character += 1;
            range.end.character += 1;
            return range.Contains(cursorPosition);
        }

        auto TestDeclaratorNameToken(const AstSyntaxToken& nameToken) -> void
        {
            if (GetInfo().ContainsPositionExtended(nameToken, cursorPosition)) {
                disableAllCompletion = true;
            }
        }

    public:
        CompletionTypeDecider(const LanguageQueryInfo& info, TextPosition cursorPosition)
            : LanguageQueryVisitor(info), cursorPosition(cursorPosition)
        {
        }

        auto Finish() -> CompletionTypeResult GLSLD_AST_VISITOR_OVERRIDE
        {
            CompletionTypeResult result;
            if (disableAllCompletion) {
                result.allowTypeName     = false;
                result.allowFunctionName = false;
                result.allowVariableName = false;
            }
            else if (accessChainBaseExpr) {
                result.allowTypeName       = false;
                result.allowFunctionName   = false;
                result.allowVariableName   = false;
                result.accessChainBaseExpr = accessChainBaseExpr;
            }
            else if (inFunctionDefinition) {
                result.allowTypeName     = true;
                result.allowFunctionName = true;
                result.allowVariableName = true;
            }
            else if (inStructDefinition) {
                result.allowTypeName     = true;
                result.allowFunctionName = false;
                result.allowVariableName = false;
            }

            result.pendingReplacementToken = pendingReplacementToken;
            return result;
        }

        auto EnterAstNode(const AstNode& node) -> AstVisitPolicy GLSLD_AST_VISITOR_OVERRIDE
        {
            return TraverseNodeContains(node, cursorPosition);
        }

        auto VisitAstLiteralExpr(const AstLiteralExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            disableAllCompletion = true;
        }

        auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (CheckTokenForReplacement(expr.GetNameToken())) {
                // For name access, we only consider the replacement case, e.g. "fo^o"
                // Insertion case doesn't make sense because the AST node would not be available without the identifier.
                pendingReplacementToken = expr.GetNameToken();
            }
        }

        auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (CheckTokenForReplacement(expr.GetNameToken())) {
                // Case 1: cursor is on the field name for replacement, e.g. "a.he^llo"
                accessChainBaseExpr     = expr.GetBaseExpr();
                pendingReplacementToken = expr.GetNameToken();
            }
            else if (auto dotTokIndex = GetInfo().LookupDotTokenIndex(expr)) {
                // Case 2: cursor is after the dot for insertion, e.g. "a.^" or "a. ^ whatever"
                auto dotToken = GetInfo().LookupToken(*dotTokIndex);
                if (!GetInfo().ContainsPosition(AstSyntaxRange{*dotTokIndex}, cursorPosition) &&
                    GetInfo().ContainsPositionExtended(AstSyntaxRange{*dotTokIndex}, cursorPosition)) {
                    accessChainBaseExpr = expr.GetBaseExpr();
                }
            }

            // Case 3: cursor is on the base expression, e.g. "a^.hello"
            // We do not handle this case here because it should be handled while visiting the base expression itself.
        }

        auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (CheckTokenForReplacement(expr.GetNameToken())) {
                // Case 1: cursor is on the field name for replacement, e.g. "a.xy^z"
                accessChainBaseExpr     = expr.GetBaseExpr();
                pendingReplacementToken = expr.GetNameToken();
            }
            else if (auto dotTokIndex = GetInfo().LookupDotTokenIndex(expr)) {
                // Case 2: cursor is after the dot for insertion, e.g. "a.^" or "a. ^ whatever"
                auto dotToken = GetInfo().LookupToken(*dotTokIndex);
                if (!GetInfo().ContainsPosition(AstSyntaxRange{*dotTokIndex}, cursorPosition) &&
                    GetInfo().ContainsPositionExtended(AstSyntaxRange{*dotTokIndex}, cursorPosition)) {
                    accessChainBaseExpr = expr.GetBaseExpr();
                }
            }

            // Case 3: cursor is on the base expression, e.g. "a^.xyz"
            // We do not handle this case here because it should be handled while visiting the base expression itself.
        }

        auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (CheckTokenForReplacement(expr.GetNameToken())) {
                // Case 1: cursor is on the function name for replacement, e.g. "fo^o()"
                pendingReplacementToken = expr.GetNameToken();
            }
            else if (!GetInfo().ContainsPosition(expr.GetNameToken(), cursorPosition) &&
                     GetInfo().ContainsPositionExtended(expr.GetNameToken(), cursorPosition)) {
                // Case 2: `foo  ^()` should not result in completion
                disableAllCompletion = true;
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
            // TODO: allow user type name while completing parameter.
            // FIXME: what about cursor?
            if (GetInfo().ContainsPositionExtended(decl, cursorPosition)) {
                disableAllCompletion = true;
            }
        }

        auto VisitAstStructDecl(const AstStructDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            if (decl.GetNameToken()) {
                TestDeclaratorNameToken(*decl.GetNameToken());
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
            TestDeclaratorNameToken(decl.GetNameToken());
        }

        auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
        {
            TestDeclaratorNameToken(decl.GetNameToken());
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
                if (!completionType.allowFunctionName) {
                    return;
                }
                else {
                    break;
                }
            case lsp::CompletionItemKind::Variable:
                if (!completionType.allowVariableName) {
                    return;
                }
                else {
                    break;
                }
            case lsp::CompletionItemKind::Struct:
                if (!completionType.allowTypeName) {
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

    auto ComputeKeywordCompletionItems() -> std::vector<lsp::CompletionItem>
    {
        std::vector<lsp::CompletionItem> result;
        result.reserve(GetAllKeywords().size());
        for (auto [_, keywordText] : GetAllKeywords()) {
            result.push_back(lsp::CompletionItem{
                .label = std::string{keywordText},
                .kind  = lsp::CompletionItemKind::Keyword,
            });
        }

        return result;
    }

    auto ComputePreambleCompletionItems(const PrecompiledPreamble& preamble) -> std::vector<lsp::CompletionItem>
    {
        std::vector<lsp::CompletionItem> result;
        std::unordered_set<AtomString> seenIds;
        for (const AstDecl* decl : preamble.GetSystemPreambleArtifacts().GetAst()->GetGlobalDecls()) {
            CollectCompletionFromDecl(
                [&](const AstSyntaxToken& declTok, lsp::CompletionItemKind kind) {
                    if (seenIds.find(declTok.text) == seenIds.end()) {
                        seenIds.insert(declTok.text);
                        result.push_back(lsp::CompletionItem{
                            .label = declTok.text.Str(),
                            .kind  = kind,
                        });
                    }
                },
                *decl);
        }

        return result;
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

    auto HandleCompletion(const CompletionConfig& config, const LanguageQueryInfo& queryInfo, CompletionState& state,
                          const lsp::CompletionParams& params) -> lsp::CompletionList
    {
        if (!config.enable) {
            return {};
        }

        const auto& compilerResult = queryInfo.GetCompilerResult();

        auto cursorPosition = FromLspPosition(params.position);

        bool isIncomplete = false;
        std::vector<lsp::CompletionItem> result;
        auto completionType = TraverseAst(CompletionTypeDecider{queryInfo, cursorPosition}, queryInfo.GetUserFileAst());
        if (completionType.accessChainBaseExpr) {
            // We are completing `expr.^`
            auto baseType = completionType.accessChainBaseExpr->GetDeducedType();
            if (baseType->IsArray() || baseType->IsVector() || baseType->IsMatrix()) {
                // .length operator
                // FIXME: we should insert `length()` instead of `length` here
                result.push_back({lsp::CompletionItem{
                    .label = "length",
                    .kind  = lsp::CompletionItemKind::Method,
                }});

                if (auto vectorDesc = baseType->GetVectorDesc(); vectorDesc) {
                    // swizzle operators
                    // TODO: we should support swizzle on scalar as well?
                    auto pendingReplacementText = completionType.pendingReplacementToken.text.StrView();
                    if (pendingReplacementText.Empty()) {
                        // No pending text, suggest all possible swizzle components
                        for (char ch : StringView{"xyzwrgbastpq"}) {
                            result.push_back({lsp::CompletionItem{
                                .label = std::string{ch},
                                .kind  = lsp::CompletionItemKind::Field,
                            }});
                        }
                    }
                    else if (auto swizzleDesc = SwizzleDesc::Parse(pendingReplacementText); swizzleDesc.IsValid()) {
                        // As what's already there is a valid swizzle, we either accept it as is, or extend it.
                        result.push_back({lsp::CompletionItem{
                            .label = pendingReplacementText.Str(),
                            .kind  = lsp::CompletionItemKind::Field,
                        }});

                        if (pendingReplacementText.Size() < 4) {
                            for (char newChar : swizzleDesc.GetCharSetSeq().Take(vectorDesc->vectorSize)) {
                                result.push_back({lsp::CompletionItem{
                                    .label = pendingReplacementText.Str() + newChar,
                                    .kind  = lsp::CompletionItemKind::Field,
                                }});
                            }
                        }
                    }

                    // Mark as incomplete to avoid flooding the list
                    isIncomplete = true;
                }
            }
            else if (auto structDesc = baseType->GetStructDesc(); structDesc) {
                for (const auto& memberDesc : structDesc->members) {
                    result.push_back({lsp::CompletionItem{
                        .label = memberDesc.name,
                        .kind  = lsp::CompletionItemKind::Field,
                    }});
                }
            }
        }
        else {
            if (compilerResult.GetPreamble() != state.preamble) {
                // Recompute the cached completion items if the preamble has changed
                state.preamble                = compilerResult.GetPreamble();
                state.preambleCompletionItems = ComputePreambleCompletionItems(*state.preamble);
            }

            result = ComputeKeywordCompletionItems();
            if (completionType.allowTypeName || completionType.allowFunctionName || completionType.allowVariableName) {
                // Copy the completion items from the language and standard library
                std::ranges::copy_if(state.preambleCompletionItems, std::back_inserter(result),
                                     [&](const lsp::CompletionItem& item) -> bool {
                                         switch (item.kind) {
                                         case lsp::CompletionItemKind::Struct:
                                             return completionType.allowTypeName;
                                         case lsp::CompletionItemKind::Function:
                                             return completionType.allowFunctionName;
                                         case lsp::CompletionItemKind::Variable:
                                             return completionType.allowVariableName;
                                         default:
                                             return false;
                                         }
                                     });

                // Add the completion items from the AST
                TraverseAst(CompletionCollector{result, queryInfo, completionType, cursorPosition},
                            queryInfo.GetUserFileAst());
            }

            // FIXME: add the completion items from the preprocessor, aka. macros
        }

        return lsp::CompletionList{
            .isIncomplete = isIncomplete,
            .items        = std::move(result),
        };
    }
} // namespace glsld