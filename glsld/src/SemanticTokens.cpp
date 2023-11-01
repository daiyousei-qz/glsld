#include "SemanticTokens.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    auto CreateSemanticTokenInfo(std::vector<SemanticTokenInfo>& tokenBuffer, const LanguageQueryProvider& provider,
                                 SyntaxToken token, SemanticTokenType type,
                                 SemanticTokenModifier modifier = SemanticTokenModifier::None) -> void
    {
        // FIXME: how to handle multi-line token
        //            a\
        //            b
        if (auto tokRange = provider.GetSpelledTextRangeInMainFile(token)) {
            if (!tokRange->IsEmpty()) {
                tokenBuffer.push_back(SemanticTokenInfo{
                    .line      = tokRange->start.line,
                    .character = tokRange->start.character,
                    .length    = tokRange->end.character - tokRange->start.character,
                    .type      = type,
                    .modifier  = modifier,
                });
            }
        }
    }

    // Collect semantic tokens from token stream, including keywords, numbers, etc.
    auto CollectLexSemanticTokens(const LanguageQueryProvider& provider, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        const auto& lexContext = provider.GetLexContext();
        for (SyntaxTokenIndex tokIndex = lexContext.GetTUTokenIndexOffset(); tokIndex < lexContext.GetTotalTokenCount();
             ++tokIndex) {
            auto tok = lexContext.GetTUToken(tokIndex);

            std::optional<SemanticTokenType> type;
            if (IsKeywordToken(tok.klass)) {
                if (!GetGlslBuiltinType(tok.klass)) {
                    // Type keyword would be handled by traversing Ast
                    type = SemanticTokenType::Keyword;
                }
            }
            else if (tok.klass == TokenKlass::IntegerConstant || tok.klass == TokenKlass::FloatConstant) {
                type = SemanticTokenType::Number;
            }

            if (type) {
                CreateSemanticTokenInfo(tokenBuffer, provider, tok, *type);
            }
        }
    }

    auto CollectPreprocessSemanticTokens(const PreprocessInfoCache& ppInfoCache,
                                         std::vector<SemanticTokenInfo>& tokenBuffer) -> void
    {
        for (const auto& ppToken : ppInfoCache.GetHeaderNames()) {
            tokenBuffer.push_back(SemanticTokenInfo{
                .line      = ppToken.spelledRange.start.line,
                .character = ppToken.spelledRange.start.character,
                .length    = ppToken.spelledRange.end.character - ppToken.spelledRange.start.character,
                .type      = SemanticTokenType::String,
                .modifier  = SemanticTokenModifier::None,
            });
        }

        for (const auto& ppToken : ppInfoCache.GetMacroUses()) {
            tokenBuffer.push_back(SemanticTokenInfo{
                .line      = ppToken.spelledRange.start.line,
                .character = ppToken.spelledRange.start.character,
                .length    = ppToken.spelledRange.end.character - ppToken.spelledRange.start.character,
                .type      = SemanticTokenType::Macro,
                .modifier  = SemanticTokenModifier::None,
            });
        }
    }

    // Collect semantic tokens from AST, including types, structs, functions, etc.
    auto CollectAstSemanticTokens(const LanguageQueryProvider& provider, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        class AstSemanticTokenCollector : public LanguageQueryVisitor<AstSemanticTokenCollector>
        {
        private:
            std::vector<SemanticTokenInfo>& output;

        public:
            AstSemanticTokenCollector(const LanguageQueryProvider& provider, std::vector<SemanticTokenInfo>& output)
                : LanguageQueryVisitor(provider), output(output)
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
                // FIXME: const variable should be readonly
                if (expr.GetAccessName().IsIdentifier()) {
                    if (expr.IsParameter()) {
                        TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Parameter);
                    }
                    else {
                        TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Variable);
                    }
                }
            }
            auto VisitAstFieldAccessExpr(const AstFieldAccessExpr& expr) -> void
            {
                if (expr.GetAccessName().IsIdentifier()) {
                    TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Variable);
                }
            }
            auto VisitSwizzleAccessExpr(const AstSwizzleAccessExpr& expr) -> void
            {
                if (expr.GetAccessName().IsIdentifier()) {
                    // Note we color the identifier regardless of whether it is a valid swizzle.
                    TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Variable);
                }
            }
            auto VisitAstFunctionCallExpr(const AstFunctionCallExpr& expr) -> void
            {
                if (expr.GetFunctionName().IsIdentifier()) {
                    TryAddSementicToken(expr.GetFunctionName(), SemanticTokenType::Function);
                }
            }

            auto VisitAstFieldDecl(const AstFieldDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifier |= SemanticTokenModifier::Readonly;
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.klass == TokenKlass::Identifier) {
                        TryAddSementicToken(declarator.declTok, SemanticTokenType::Variable, modifier);
                    }
                }
            }
            auto VisitAstStructDecl(const AstStructDecl& decl) -> void
            {
                if (decl.GetDeclTok() && decl.GetDeclTok()->IsIdentifier()) {
                    TryAddSementicToken(*decl.GetDeclTok(), SemanticTokenType::Struct,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstInterfaceBlockDecl(const AstInterfaceBlockDecl& decl) -> void
            {
                // Interface block name
                if (decl.GetDeclTok().IsIdentifier()) {
                    TryAddSementicToken(decl.GetDeclTok(), SemanticTokenType::Type, SemanticTokenModifier::Declaration);
                }

                // Interface block decl
                if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
                    TryAddSementicToken(decl.GetDeclarator()->declTok, SemanticTokenType::Variable,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstFunctionDecl(const AstFunctionDecl& decl) -> void
            {
                if (decl.GetDeclTok().IsIdentifier()) {
                    TryAddSementicToken(decl.GetDeclTok(), SemanticTokenType::Function,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstParamDecl(const AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
                    SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                    if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                        modifier |= SemanticTokenModifier::Readonly;
                    }
                    TryAddSementicToken(decl.GetDeclarator()->declTok, SemanticTokenType::Parameter, modifier);
                }
            }
            auto VisitAstVariableDecl(const AstVariableDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;

                if (auto quals = decl.GetQualType()->GetQualifiers(); quals && quals->GetQualGroup().qConst) {
                    modifier |= SemanticTokenModifier::Readonly;
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.IsIdentifier()) {
                        TryAddSementicToken(declarator.declTok, SemanticTokenType::Variable, modifier);
                    }
                }
            }

        private:
            auto TryAddSementicToken(SyntaxToken token, SemanticTokenType type,
                                     SemanticTokenModifier modifier = SemanticTokenModifier::None) -> void
            {
                if (token.IsValid()) {
                    CreateSemanticTokenInfo(output, GetProvider(), token, type, modifier);
                }
            }
        };

        AstSemanticTokenCollector{provider, tokenBuffer}.Execute();
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

    auto ComputeSemanticTokens(const LanguageQueryProvider& provider) -> lsp::SemanticTokens
    {
        std::vector<SemanticTokenInfo> tokenBuffer;
        CollectLexSemanticTokens(provider, tokenBuffer);
        CollectPreprocessSemanticTokens(provider.GetPreprocessInfoCache(), tokenBuffer);
        CollectAstSemanticTokens(provider, tokenBuffer);

        std::ranges::sort(tokenBuffer, [](const SemanticTokenInfo& lhs, const SemanticTokenInfo& rhs) {
            return std::tie(lhs.line, lhs.character) < std::tie(rhs.line, rhs.character);
        });

        return ToLspSemanticTokens(tokenBuffer);
    }

    auto ComputeSemanticTokensDelta(const LanguageQueryProvider& provider) -> lsp::SemanticTokensDelta
    {
        GLSLD_NO_IMPL();
    }
} // namespace glsld