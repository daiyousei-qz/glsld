#include "SemanticTokens.h"
#include "LanguageQueryVisitor.h"

namespace glsld
{
    auto CreateSemanticTokenInfo(const LexContext& lexContext, SyntaxToken token, SemanticTokenType type,
                                 SemanticTokenModifier modifier = SemanticTokenModifier::None) -> SemanticTokenInfo
    {
        // FIXME: how to handle multi-line token
        //            a\
        //            b
        auto tokRange = lexContext.LookupSpelledTextRange(token);
        return SemanticTokenInfo{
            .line      = tokRange.start.line,
            .character = tokRange.start.character,
            .length    = tokRange.end.character - tokRange.start.character,
            .type      = type,
            .modifier  = modifier,
        };
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

    // Collect semantic tokens from token stream, including keywords, numbers, etc.
    auto CollectLexSemanticTokens(const LanguageQueryProvider& provider, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        const auto& lexContext = provider.GetLexContext();
        for (const auto& tok : lexContext.GetAllTokenView()) {
            std::optional<SemanticTokenType> type;

            // Only collect tokens from the main file.
            if (!provider.InMainFile(tok)) {
                continue;
            }

            // Only collect tokens that have a valid range.
            auto textRange = lexContext.LookupSpelledTextRange(tok);
            if (textRange.IsEmpty()) {
                continue;
            }

            if (IsKeywordToken(tok.klass)) {
                if (!GetGlslBuiltinType(tok)) {
                    // Type keyword would be handled by traversing Ast
                    type = SemanticTokenType::Keyword;
                }
            }
            else if (tok.klass == TokenKlass::IntegerConstant || tok.klass == TokenKlass::FloatConstant) {
                type = SemanticTokenType::Number;
            }

            if (type) {
                tokenBuffer.push_back(CreateSemanticTokenInfo(lexContext, tok, *type));
            }
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
                TraverseAllGlobalDecl();
            }

            auto VisitAstQualType(AstQualType& type) -> void
            {
                if (type.GetTypeNameTok().klass == TokenKlass::Identifier ||
                    GetGlslBuiltinType(type.GetTypeNameTok())) {
                    TryAddSementicToken(type.GetTypeNameTok(), SemanticTokenType::Type);
                }
            }

            auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                if (auto quals = decl.GetType()->GetQualifiers(); quals && quals->GetQualfierGroup().qConst) {
                    modifier |= SemanticTokenModifier::Readonly;
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.klass == TokenKlass::Identifier) {
                        TryAddSementicToken(declarator.declTok, SemanticTokenType::Variable, modifier);
                    }
                }
            }
            auto VisitAstStructDecl(AstStructDecl& decl) -> void
            {
                if (decl.GetDeclToken() && !decl.GetDeclToken()->IsError()) {
                    TryAddSementicToken(*decl.GetDeclToken(), SemanticTokenType::Struct,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
            {
                // Interface block name
                if (decl.GetDeclToken().IsIdentifier()) {
                    TryAddSementicToken(decl.GetDeclToken(), SemanticTokenType::Type,
                                        SemanticTokenModifier::Declaration);
                }

                // Interface block decl
                if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
                    TryAddSementicToken(decl.GetDeclarator()->declTok, SemanticTokenType::Variable,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                if (decl.GetName().IsIdentifier()) {
                    TryAddSementicToken(decl.GetName(), SemanticTokenType::Function,
                                        SemanticTokenModifier::Declaration);
                }
            }
            auto VisitAstParamDecl(AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
                    SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                    if (auto quals = decl.GetType()->GetQualifiers(); quals && quals->GetQualfierGroup().qConst) {
                        modifier |= SemanticTokenModifier::Readonly;
                    }
                    TryAddSementicToken(decl.GetDeclarator()->declTok, SemanticTokenType::Parameter, modifier);
                }
            }
            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;

                if (auto quals = decl.GetType()->GetQualifiers(); quals && quals->GetQualfierGroup().qConst) {
                    modifier |= SemanticTokenModifier::Readonly;
                }

                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.IsIdentifier()) {
                        TryAddSementicToken(declarator.declTok, SemanticTokenType::Variable, modifier);
                    }
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
            {
                switch (expr.GetAccessType()) {
                case NameAccessType::Function:
                    TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Function);
                    break;
                case NameAccessType::Constructor:
                    TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Type);
                    break;
                case NameAccessType::Variable:
                case NameAccessType::Swizzle:
                    if (expr.GetAccessName().klass == TokenKlass::Identifier) {
                        TryAddSementicToken(expr.GetAccessName(), SemanticTokenType::Variable);
                    }
                    break;
                case NameAccessType::Unknown:
                    GLSLD_UNREACHABLE();
                }
            }

            auto TryAddSementicToken(SyntaxToken token, SemanticTokenType type,
                                     SemanticTokenModifier modifier = SemanticTokenModifier::None) -> void
            {
                if (GetProvider().InMainFile(token) && token.IsValid()) {
                    // FIXME: how to handle multi-line token
                    //            a\
                    //            b
                    auto range = GetProvider().GetLexContext().LookupSpelledTextRange(token);
                    output.push_back(SemanticTokenInfo{
                        .line      = range.start.line,
                        .character = range.start.character,
                        .length    = range.end.character - range.start.character,
                        .type      = type,
                        .modifier  = modifier,
                    });
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
        CollectPreprocessSemanticTokens(provider.GetPreprocessInfoCache(), tokenBuffer);
        CollectLexSemanticTokens(provider, tokenBuffer);
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