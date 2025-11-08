#include "Compiler/SyntaxToken.h"
#include "Feature/SemanticTokens.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/EnumReflection.h"

#include <algorithm>
#include <mutex>
#include <unordered_map>

namespace glsld
{
    // Cache for storing previous semantic tokens results
    static std::mutex semanticTokensCacheMutex;
    static std::unordered_map<std::string, std::vector<lsp::uinteger>> semanticTokensCache;
    static int nextResultId = 1;
    auto CreateSemanticTokenInfo(std::vector<SemanticTokenInfo>& tokenBuffer, const LanguageQueryInfo& info,
                                 SyntaxTokenID tokID, SemanticTokenType type, SemanticTokenModifierBits modifiers = {})
        -> void
    {
        // FIXME: how to handle multi-line token
        //            a\
        //            b
        if (auto tokTextRange = info.LookupSpelledTextRangeInMainFile(tokID)) {
            if (!tokTextRange->IsEmpty()) {
                tokenBuffer.push_back(SemanticTokenInfo{
                    .line      = tokTextRange->start.line,
                    .character = tokTextRange->start.character,
                    .length    = tokTextRange->end.character - tokTextRange->start.character,
                    .type      = type,
                    .modifiers = modifiers,
                });
            }
        }
    }

    // Collect semantic tokens from token stream, including keywords, numbers, etc.
    auto CollectLexSemanticTokens(const LanguageQueryInfo& info, std::vector<SemanticTokenInfo>& tokenBuffer) -> void
    {
        for (auto tokID : info.GetUserFileAst().GetSyntaxRange()) {
            auto tok = info.LookupToken(tokID);

            std::optional<SemanticTokenType> type;
            if (IsKeywordToken(tok->klass)) {
                if (!GetGlslBuiltinType(tok->klass)) {
                    // Type keyword would be handled by traversing Ast
                    type = SemanticTokenType::Keyword;
                }
            }
            else if (tok->klass == TokenKlass::IntegerConstant || tok->klass == TokenKlass::FloatConstant) {
                type = SemanticTokenType::Number;
            }

            if (type) {
                CreateSemanticTokenInfo(tokenBuffer, info, tokID, *type);
            }
        }
    }

    auto CollectPreprocessSemanticTokens(const PreprocessSymbolStore& ppInfoStore,
                                         std::vector<SemanticTokenInfo>& tokenBuffer) -> void
    {
        for (const auto& ppSymbol : ppInfoStore.GetAllOccurrences()) {
            if (auto headerName = ppSymbol.GetHeaderNameInfo(); headerName) {
                tokenBuffer.push_back(SemanticTokenInfo{
                    .line      = headerName->headerName.spelledRange.start.line,
                    .character = headerName->headerName.spelledRange.start.character,
                    .length    = headerName->headerName.spelledRange.end.character -
                              headerName->headerName.spelledRange.start.character,
                    .type      = SemanticTokenType::String,
                    .modifiers = {},
                });
            }
            else if (auto macroSymbol = ppSymbol.GetMacroInfo(); macroSymbol) {
                tokenBuffer.push_back(SemanticTokenInfo{
                    .line      = macroSymbol->macroName.spelledRange.start.line,
                    .character = macroSymbol->macroName.spelledRange.start.character,
                    .length    = macroSymbol->macroName.spelledRange.end.character -
                              macroSymbol->macroName.spelledRange.start.character,
                    .type      = SemanticTokenType::Macro,
                    .modifiers = {},
                });
            }
        }
    }

    // Collect semantic tokens from AST, including types, structs, functions, etc.
    auto CollectAstSemanticTokens(const LanguageQueryInfo& info, std::vector<SemanticTokenInfo>& tokenBuffer) -> void
    {
        class AstSemanticTokenCollector : public LanguageQueryVisitor<AstSemanticTokenCollector>
        {
        private:
            std::vector<SemanticTokenInfo>& output;

        public:
            AstSemanticTokenCollector(const LanguageQueryInfo& info, std::vector<SemanticTokenInfo>& output)
                : LanguageQueryVisitor(info), output(output)
            {
            }

            auto Execute() -> void
            {
                TraverseTranslationUnit();
            }

            auto VisitAstQualType(const AstQualType& type) -> void
            {
                if (type.GetTypeNameTok().klass == TokenKlass::Identifier ||
                    GetGlslBuiltinType(type.GetTypeNameTok().klass)) {
                    TryAddSementicToken(type.GetTypeNameTok(), SemanticTokenType::Type);
                }
            }

            auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    SemanticTokenModifierBits modifiers;
                    if (expr.IsConstNameAccess()) {
                        modifiers |= SemanticTokenModifier::Readonly;
                    }

                    if (expr.IsParameter()) {
                        TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Parameter, modifiers);
                    }
                    else {
                        TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable, modifiers);
                    }
                }
            }
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable);
                }
            }
            auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    // Note we color the identifier regardless of whether it is a valid swizzle.
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable);
                }
            }
            auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Function);
                }
            }
            auto VisitAstStructFieldDeclaratorDecl(const AstStructFieldDeclaratorDecl& decl) -> void
            {
                SemanticTokenModifierBits modifiers;
                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifiers |= SemanticTokenModifier::Readonly;
                }

                if (decl.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(decl.GetNameToken(), SemanticTokenType::Variable, modifiers);
                }
            }
            auto VisitAstStructDecl(const AstStructDecl& decl) -> void
            {
                if (decl.GetNameToken() && decl.GetNameToken()->IsIdentifier()) {
                    TryAddSementicToken(*decl.GetNameToken(), SemanticTokenType::Struct,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstBlockFieldDecl(const AstBlockFieldDecl& decl) -> void
            {
                SemanticTokenModifierBits modifiers = SemanticTokenModifier::Declaration;
                // TODO: say uniform block member is readonly

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator->GetNameToken().klass == TokenKlass::Identifier) {
                        TryAddSementicToken(declarator->GetNameToken(), SemanticTokenType::Variable, modifiers);
                    }
                }
            }
            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
            {
                // Interface block name
                if (decl.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(decl.GetNameToken(), SemanticTokenType::Type,
                                        SemanticTokenModifier::Declaration);
                }

                // Interface block decl
                SemanticTokenModifierBits instanceModifier = SemanticTokenModifier::Declaration;
                // TODO: say uniform block member is readonly
                if (decl.GetDeclarator() && decl.GetDeclarator()->nameToken.IsIdentifier()) {
                    TryAddSementicToken(decl.GetDeclarator()->nameToken, SemanticTokenType::Variable, instanceModifier);
                }
            }
            auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
            {
                if (decl.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(decl.GetNameToken(), SemanticTokenType::Function,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstParamDecl(const AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator() && decl.GetDeclarator()->nameToken.IsIdentifier()) {
                    SemanticTokenModifierBits modifiers = SemanticTokenModifier::Declaration;
                    if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                        modifiers |= SemanticTokenModifier::Readonly;
                    }
                    TryAddSementicToken(decl.GetDeclarator()->nameToken, SemanticTokenType::Parameter, modifiers);
                }
            }
            auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl) -> void
            {
                SemanticTokenModifierBits modifiers = SemanticTokenModifier::Declaration;

                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifiers |= SemanticTokenModifier::Readonly;
                }

                TryAddSementicToken(decl.GetNameToken(), SemanticTokenType::Variable, modifiers);
            }

        private:
            auto TryAddSementicToken(AstSyntaxToken token, SemanticTokenType type,
                                     SemanticTokenModifierBits modifier = {}) -> void
            {
                if (token.IsValid()) {
                    CreateSemanticTokenInfo(output, GetInfo(), token.id, type, modifier);
                }
            }
        };

        AstSemanticTokenCollector{info, tokenBuffer}.Execute();
    }

    auto ToLspSemanticTokens(ArrayView<SemanticTokenInfo> tokenBuffer) -> lsp::SemanticTokens
    {
        lsp::SemanticTokens result;
        int lastTokLine   = -1;
        int lastTokColumn = -1;
        for (const auto& tokInfo : tokenBuffer) {
            if (lastTokLine == -1) {
                result.data.push_back(tokInfo.line);
                result.data.push_back(tokInfo.character);
            }
            else {
                if (lastTokLine == tokInfo.line) {
                    result.data.push_back(0);
                    result.data.push_back(tokInfo.character - lastTokColumn);
                }
                else {
                    result.data.push_back(tokInfo.line - lastTokLine);
                    result.data.push_back(tokInfo.character);
                }
            }

            result.data.push_back(tokInfo.length);
            result.data.push_back(GetTokenTypeIndex(tokInfo.type));
            result.data.push_back(tokInfo.modifiers.GetBits());

            lastTokLine   = tokInfo.line;
            lastTokColumn = tokInfo.character;
        }

        return result;
    }

    auto CollectSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info)
        -> std::vector<SemanticTokenInfo>
    {
        if (!config.enable) {
            return {};
        }

        std::vector<SemanticTokenInfo> tokenBuffer;
        CollectLexSemanticTokens(info, tokenBuffer);
        CollectPreprocessSemanticTokens(info.GetPreprocessInfo(), tokenBuffer);
        CollectAstSemanticTokens(info, tokenBuffer);

        std::ranges::sort(tokenBuffer, [](const SemanticTokenInfo& lhs, const SemanticTokenInfo& rhs) {
            return std::tie(lhs.line, lhs.character) < std::tie(rhs.line, rhs.character);
        });
        return tokenBuffer;
    }

    // Compute edits to transform oldData into newData
    auto ComputeSemanticTokensEdits(const std::vector<lsp::uinteger>& oldData,
                                    const std::vector<lsp::uinteger>& newData) -> std::vector<lsp::SemanticTokensEdit>
    {
        std::vector<lsp::SemanticTokensEdit> edits;

        // Use a simple diff algorithm: find common prefix and suffix, then replace the middle
        size_t prefixLen = 0;
        size_t minLen    = std::min(oldData.size(), newData.size());

        // Find common prefix
        while (prefixLen < minLen && oldData[prefixLen] == newData[prefixLen]) {
            prefixLen++;
        }

        // Find common suffix
        size_t suffixLen = 0;
        while (suffixLen < minLen - prefixLen && 
               oldData[oldData.size() - 1 - suffixLen] == newData[newData.size() - 1 - suffixLen]) {
            suffixLen++;
        }

        // If everything is the same, no edits needed
        if (prefixLen + suffixLen >= minLen && oldData.size() == newData.size()) {
            return edits;
        }

        // Compute the edit
        size_t deleteStart = prefixLen;
        size_t deleteCount = oldData.size() - prefixLen - suffixLen;
        
        std::vector<lsp::uinteger> insertData;
        if (newData.size() > prefixLen + suffixLen) {
            insertData.insert(insertData.end(), 
                            newData.begin() + prefixLen, 
                            newData.end() - suffixLen);
        }

        // Only add an edit if there's actually a change
        if (deleteCount > 0 || !insertData.empty()) {
            edits.push_back(lsp::SemanticTokensEdit{
                .start       = static_cast<lsp::uinteger>(deleteStart),
                .deleteCount = static_cast<lsp::uinteger>(deleteCount),
                .data        = std::move(insertData),
            });
        }

        return edits;
    }

    auto GetSemanticTokensOptions(const SemanticTokenConfig& config) -> std::optional<lsp::SemanticTokensOptions>
    {
        if (!config.enable) {
            return std::nullopt;
        }

        return lsp::SemanticTokensOptions{
            .legend =
                lsp::SemanticTokensLegend{
                    .tokenTypes =
                        {
                            "type",
                            "struct",
                            "parameter",
                            "variable",
                            "function",
                            "keyword",
                            "comment",
                            "number",
                            "macro",
                            "string",
                        },
                    .tokenModifiers =
                        {
                            "readonly",
                            "declaration",
                        },
                },
            .full =
                lsp::SemanticTokensOptions::FullOptions{
                    .delta = true,
                },
        };
    }

    auto HandleSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                              const lsp::SemanticTokensParams& params) -> lsp::SemanticTokens
    {
        auto result = ToLspSemanticTokens(CollectSemanticTokens(config, info));
        
        // Generate a result ID and cache the data
        std::lock_guard<std::mutex> lock(semanticTokensCacheMutex);
        result.resultId = std::to_string(nextResultId++);
        semanticTokensCache[result.resultId] = result.data;
        
        return result;
    }

    auto HandleSemanticTokensDelta(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                                   const lsp::SemanticTokensDeltaParams& params) -> lsp::SemanticTokensDelta
    {
        // Get the new semantic tokens data
        auto newResult = ToLspSemanticTokens(CollectSemanticTokens(config, info));
        
        lsp::SemanticTokensDelta delta;
        
        // Look up the previous data
        std::lock_guard<std::mutex> lock(semanticTokensCacheMutex);
        auto it = semanticTokensCache.find(params.previousResultId);
        
        if (it != semanticTokensCache.end()) {
            // Compute the diff
            delta.edits = ComputeSemanticTokensEdits(it->second, newResult.data);
        } else {
            // If we don't have the previous result, return an edit that replaces everything
            delta.edits.push_back(lsp::SemanticTokensEdit{
                .start       = 0,
                .deleteCount = 0,  // No delete since we don't know the old size
                .data        = newResult.data,
            });
        }
        
        // Generate a new result ID and cache the new data
        delta.resultId = std::to_string(nextResultId++);
        semanticTokensCache[delta.resultId] = newResult.data;
        
        return delta;
    }
} // namespace glsld