#include "Compiler/SyntaxToken.h"
#include "Feature/SemanticTokens.h"
#include "Server/LanguageQueryVisitor.h"

namespace glsld
{
    auto CreateSemanticTokenInfo(std::vector<SemanticTokenInfo>& tokenBuffer, const LanguageQueryInfo& info,
                                 SyntaxTokenID tokID, SemanticTokenType type,
                                 SemanticTokenModifier modifier = SemanticTokenModifier::None) -> void
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
                    .modifier  = modifier,
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
                    .type     = SemanticTokenType::String,
                    .modifier = SemanticTokenModifier::None,
                });
            }
            else if (auto macroSymbol = ppSymbol.GetMacroInfo(); macroSymbol) {
                tokenBuffer.push_back(SemanticTokenInfo{
                    .line      = macroSymbol->macroName.spelledRange.start.line,
                    .character = macroSymbol->macroName.spelledRange.start.character,
                    .length    = macroSymbol->macroName.spelledRange.end.character -
                              macroSymbol->macroName.spelledRange.start.character,
                    .type     = SemanticTokenType::Macro,
                    .modifier = SemanticTokenModifier::None,
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
                    auto modifier = SemanticTokenModifier::None;
                    if (expr.IsConstNameAccess()) {
                        modifier |= SemanticTokenModifier::Readonly;
                    }

                    if (expr.IsParameter()) {
                        TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Parameter, modifier);
                    }
                    else {
                        TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable, modifier);
                    }
                }
            }
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
            {
                if (expr.GetNameToken().IsIdentifier()) {
                    TryAddSementicToken(expr.GetNameToken(), SemanticTokenType::Variable);
                }
            }
            auto VisitSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void
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
            auto VisitAstStructFieldDecl(const AstStructFieldDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifier |= SemanticTokenModifier::Readonly;
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.nameToken.klass == TokenKlass::Identifier) {
                        TryAddSementicToken(declarator.nameToken, SemanticTokenType::Variable, modifier);
                    }
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
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                // TODO: say uniform block member is readonly

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.nameToken.klass == TokenKlass::Identifier) {
                        TryAddSementicToken(declarator.nameToken, SemanticTokenType::Variable, modifier);
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
                SemanticTokenModifier instanceModifier = SemanticTokenModifier::Declaration;
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
                    SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                    if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                        modifier |= SemanticTokenModifier::Readonly;
                    }
                    TryAddSementicToken(decl.GetDeclarator()->nameToken, SemanticTokenType::Parameter, modifier);
                }
            }
            auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;

                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifier |= SemanticTokenModifier::Readonly;
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.nameToken.IsIdentifier()) {
                        TryAddSementicToken(declarator.nameToken, SemanticTokenType::Variable, modifier);
                    }
                }
            }

        private:
            auto TryAddSementicToken(AstSyntaxToken token, SemanticTokenType type,
                                     SemanticTokenModifier modifier = SemanticTokenModifier::None) -> void
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
            result.data.push_back(GetTokenModifierMask(tokInfo.modifier));

            lastTokLine   = tokInfo.line;
            lastTokColumn = tokInfo.character;
        }

        return result;
    }

    auto CollectSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info)
        -> std::vector<SemanticTokenInfo>
    {
        std::vector<SemanticTokenInfo> tokenBuffer;
        CollectLexSemanticTokens(info, tokenBuffer);
        CollectPreprocessSemanticTokens(info.GetPreprocessInfo(), tokenBuffer);
        CollectAstSemanticTokens(info, tokenBuffer);

        std::ranges::sort(tokenBuffer, [](const SemanticTokenInfo& lhs, const SemanticTokenInfo& rhs) {
            return std::tie(lhs.line, lhs.character) < std::tie(rhs.line, rhs.character);
        });
        return tokenBuffer;
    }

    auto GetSemanticTokensOptions(const SemanticTokenConfig& config) -> lsp::SemanticTokensOptions
    {
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
                    .delta = false,
                },
        };
    }

    auto HandleSemanticTokens(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                              const lsp::SemanticTokensParams& params) -> lsp::SemanticTokens
    {
        return ToLspSemanticTokens(CollectSemanticTokens(config, info));
    }

    auto HandleSemanticTokensDelta(const SemanticTokenConfig& config, const LanguageQueryInfo& info,
                                   const lsp::SemanticTokensDeltaParams& params) -> lsp::SemanticTokensDelta
    {
        GLSLD_NO_IMPL();
    }
} // namespace glsld