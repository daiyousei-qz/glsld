#include "Compiler/SyntaxToken.h"
#include "Feature/SemanticTokens.h"
#include "Server/LanguageQueryVisitor.h"
#include "Support/EnumReflection.h"

#include <string>

namespace glsld
{
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

            auto VisitAstQualType(const AstQualType& type) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (type.GetTypeNameTok().klass == TokenKlass::Identifier ||
                    GetGlslBuiltinType(type.GetTypeNameTok().klass)) {
                    TryAddSementicToken(type.GetTypeNameTok(), SemanticTokenType::Type);
                }
            }

            auto VisitAstNameAccessExpr(const AstNameAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
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
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable);
                }
            }
            auto VisitAstSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    // Note we color the identifier regardless of whether it is a valid swizzle.
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable);
                }
            }
            auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Function);
                }
            }
            auto VisitAstStructFieldDeclaratorDecl(const AstStructFieldDeclaratorDecl& decl)
                -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                SemanticTokenModifierBits modifiers;
                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifiers |= SemanticTokenModifier::Readonly;
                }

                if (decl.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(decl.GetNameToken(), SemanticTokenType::Variable, modifiers);
                }
            }
            auto VisitAstStructDecl(const AstStructDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (decl.GetNameToken() && decl.GetNameToken()->IsIdentifier()) {
                    TryAddSementicToken(*decl.GetNameToken(), SemanticTokenType::Struct,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstBlockFieldDecl(const AstBlockFieldDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                SemanticTokenModifierBits modifiers = SemanticTokenModifier::Declaration;
                // TODO: say uniform block member is readonly

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator->GetNameToken().klass == TokenKlass::Identifier) {
                        TryAddSementicToken(declarator->GetNameToken(), SemanticTokenType::Variable, modifiers);
                    }
                }
            }
            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
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
            auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (decl.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(decl.GetNameToken(), SemanticTokenType::Function,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstParamDecl(const AstParamDecl& decl) -> void GLSLD_AST_VISITOR_OVERRIDE
            {
                if (decl.GetDeclarator() && decl.GetDeclarator()->nameToken.IsIdentifier()) {
                    SemanticTokenModifierBits modifiers = SemanticTokenModifier::Declaration;
                    if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                        modifiers |= SemanticTokenModifier::Readonly;
                    }
                    TryAddSementicToken(decl.GetDeclarator()->nameToken, SemanticTokenType::Parameter, modifiers);
                }
            }
            auto VisitAstVariableDeclaratorDecl(const AstVariableDeclaratorDecl& decl)
                -> void GLSLD_AST_VISITOR_OVERRIDE
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

        TraverseAst(AstSemanticTokenCollector{info, tokenBuffer}, info.GetUserFileAst());
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

    static auto NextSemanticTokensResultId(SemanticTokensState& state) -> std::string
    {
        return std::to_string(state.nextResultId.fetch_add(1, std::memory_order_relaxed) + 1);
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
                              SemanticTokensState& state, const lsp::SemanticTokensParams& params)
        -> lsp::SemanticTokens
    {
        auto result    = ToLspSemanticTokens(CollectSemanticTokens(config, info));
        result.resultId = NextSemanticTokensResultId(state);
        state.resultId  = result.resultId;
        state.data      = result.data;
        return result;
    }

    auto HandleSemanticTokensDelta(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                                   SemanticTokensState& state, const lsp::SemanticTokensDeltaParams& params)
        -> lsp::SemanticTokensDelta
    {
        auto tokens = ToLspSemanticTokens(CollectSemanticTokens(config, info));

        lsp::SemanticTokensDelta result;
        result.resultId = NextSemanticTokensResultId(state);

        std::size_t start  = 0;
        std::size_t oldEnd = state.data.size();
        std::size_t newEnd = tokens.data.size();

        if (params.previousResultId != state.resultId) {
            if (!state.data.empty() || !tokens.data.empty()) {
                result.edits.push_back(lsp::SemanticTokensEdit{
                    .start       = 0,
                    .deleteCount = static_cast<lsp::uinteger>(state.data.size()),
                    .data        = tokens.data,
                });
            }

            state.resultId = result.resultId;
            state.data     = std::move(tokens.data);
            return result;
        }

        while (start < oldEnd && start < newEnd && state.data[start] == tokens.data[start]) {
            ++start;
        }

        while (oldEnd > start && newEnd > start && state.data[oldEnd - 1] == tokens.data[newEnd - 1]) {
            --oldEnd;
            --newEnd;
        }

        if (start != oldEnd || start != newEnd) {
            auto startIt = tokens.data.begin() + static_cast<std::vector<lsp::uinteger>::difference_type>(start);
            auto endIt   = tokens.data.begin() + static_cast<std::vector<lsp::uinteger>::difference_type>(newEnd);
            result.edits.push_back(lsp::SemanticTokensEdit{
                .start       = static_cast<lsp::uinteger>(start),
                .deleteCount = static_cast<lsp::uinteger>(oldEnd - start),
                .data        = std::vector<lsp::uinteger>(startIt, endIt),
            });
        }

        state.resultId = result.resultId;
        state.data     = std::move(tokens.data);

        return result;
    }
} // namespace glsld
