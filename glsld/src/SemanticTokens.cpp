#include "LanguageService.h"
#include "ModuleVisitor.h"

namespace glsld
{
    enum class SemanticTokenType
    {
        Type      = 0,
        Struct    = 1,
        Parameter = 2,
        Variable  = 3,
        Function  = 4,
        Keyword   = 5,
        Comment   = 6,
        Number    = 7,
        Macro     = 8,
        String    = 9,
    };

    enum class SemanticTokenModifier
    {
        None        = 0,
        Readonly    = 1 << 0,
        Declaration = 1 << 1,
    };

    constexpr auto operator|(SemanticTokenModifier lhs, SemanticTokenModifier rhs) -> SemanticTokenModifier
    {
        using EnumInt = std::underlying_type_t<SemanticTokenModifier>;
        return static_cast<SemanticTokenModifier>(static_cast<EnumInt>(lhs) | static_cast<EnumInt>(rhs));
    }
    constexpr auto operator|=(SemanticTokenModifier& lhs, SemanticTokenModifier rhs) -> SemanticTokenModifier&
    {
        lhs = lhs | rhs;
        return lhs;
    }

    auto GetTokenTypeIndex(SemanticTokenType type) -> int
    {
        return static_cast<int>(type);
    }

    auto GetTokenModifierIndex(SemanticTokenModifier modifier) -> int
    {
        return static_cast<int>(modifier);
    }

    auto GetTokenLegend() -> lsp::SemanticTokensLegend
    {
        return lsp::SemanticTokensLegend{
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
        };
    }

    struct SemanticTokenInfo
    {
        int line;
        int character;
        int length;
        SemanticTokenType type;
        SemanticTokenModifier modifier;
    };

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

    auto CollectPreprocessSemanticTokens(const PPInfoCache& ppInfoCache, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
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

    auto CollectLexSemanticTokens(const CompilerObject& compilerObject, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        const auto& lexContext = compilerObject.GetLexContext();
        for (const auto& tok : lexContext.GetAllTokenView()) {
            std::optional<SemanticTokenType> type;

            // Only collect tokens from the main file.
            if (lexContext.LookupSpelledFile(tok) != compilerObject.GetSourceContext().GetMainFile()->GetID()) {
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

    auto CollectAstSemanticTokens(const CompilerObject& compilerObject, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        struct AstSemanticTokenCollector : public AstVisitor<AstSemanticTokenCollector>
        {
            const CompilerObject& compilerObject;
            std::vector<SemanticTokenInfo>& tokenBuffer;

            AstSemanticTokenCollector(const CompilerObject& compiler, std::vector<SemanticTokenInfo>& tokenBuffer)
                : compilerObject(compiler), tokenBuffer(tokenBuffer)
            {
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
                if (decl.GetType()->GetQualifiers()->GetQualfierGroup().qConst) {
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
                    if (decl.GetType()->GetQualifiers()->GetQualfierGroup().qConst) {
                        modifier |= SemanticTokenModifier::Readonly;
                    }
                    TryAddSementicToken(decl.GetDeclarator()->declTok, SemanticTokenType::Parameter, modifier);
                }
            }
            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                SemanticTokenModifier modifier = SemanticTokenModifier::Declaration;
                if (decl.GetType()->GetQualifiers()->GetQualfierGroup().qConst) {
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
                default:
                    GLSLD_UNREACHABLE();
                }
            }

            auto TryAddSementicToken(SyntaxToken token, SemanticTokenType type,
                                     SemanticTokenModifier modifier = SemanticTokenModifier::None) -> void
            {
                if (compilerObject.GetLexContext().LookupSpelledFile(token) ==
                    compilerObject.GetSourceContext().GetMainFile()->GetID()) {
                    tokenBuffer.push_back(
                        CreateSemanticTokenInfo(compilerObject.GetLexContext(), token, type, modifier));
                }
            }
        };

        AstSemanticTokenCollector{compilerObject, tokenBuffer}.TraverseAst(compilerObject.GetAstContext());
    }

    auto ComputeSemanticTokens(const IntellisenseProvider& provider) -> lsp::SemanticTokens
    {
        std::vector<SemanticTokenInfo> tokenBuffer;
        CollectPreprocessSemanticTokens(provider.GetPPInfoCache(), tokenBuffer);
        CollectLexSemanticTokens(provider.GetCompilerObject(), tokenBuffer);
        CollectAstSemanticTokens(provider.GetCompilerObject(), tokenBuffer);
        std::ranges::sort(tokenBuffer, [](const SemanticTokenInfo& lhs, const SemanticTokenInfo& rhs) {
            return std::tie(lhs.line, lhs.character) < std::tie(rhs.line, rhs.character);
        });

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
            result.data.push_back(GetTokenModifierIndex(tokInfo.modifier));

            lastTokLine   = tokInfo.line;
            lastTokColumn = tokInfo.character;
        }

        return result;
    }

    // auto ComputeSemanticTokensDelta(const IntellisenseProvider& provider) -> lsp::SemanticTokensDelta
    // {
    //     GLSLD_NO_IMPL();
    // }
} // namespace glsld