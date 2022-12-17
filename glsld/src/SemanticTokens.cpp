#include "LanguageService.h"
#include "AstVisitor.h"

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
    };

    enum class SemanticTokenModifier
    {
        None        = 0,
        Declaration = 1,
    };

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
                },
            .tokenModifiers =
                {
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
        auto tokRange = lexContext.LookupTextRange(token);
        return SemanticTokenInfo{
            .line      = tokRange.start.line,
            .character = tokRange.start.character,
            .length    = tokRange.end.character - tokRange.start.character,
            .type      = type,
            .modifier  = modifier,
        };
    }

    auto CollectLexSemanticTokens(const CompileResult& compileResult, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        const auto& lexContext = compileResult.GetLexContext();
        for (const auto& tok : lexContext.GetAllTokenView()) {
            std::optional<SemanticTokenType> type;

            if (IsKeywordToken(tok.klass)) {
                if (!GetBuiltinType(tok)) {
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

    auto CollectAstSemanticTokens(const CompileResult& compileResult, std::vector<SemanticTokenInfo>& tokenBuffer)
        -> void
    {
        struct AstSemanticTokenCollector : public AstVisitor<AstSemanticTokenCollector>
        {
            const CompileResult& compileResult;
            std::vector<SemanticTokenInfo>& tokenBuffer;

            AstSemanticTokenCollector(const CompileResult& compiler, std::vector<SemanticTokenInfo>& tokenBuffer)
                : compileResult(compiler), tokenBuffer(tokenBuffer)
            {
            }

            auto VisitAstQualType(AstQualType& type) -> void
            {
                if (type.GetTypeNameTok().klass == TokenKlass::Identifier || GetBuiltinType(type.GetTypeNameTok())) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), type.GetTypeNameTok(),
                                                                  SemanticTokenType::Type));
                }
            }

            auto VisitAstStructMemberDecl(AstStructMemberDecl& decl) -> void
            {
                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.klass == TokenKlass::Identifier) {
                        tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), declarator.declTok,
                                                                      SemanticTokenType::Variable,
                                                                      SemanticTokenModifier::Declaration));
                    }
                }
            }
            auto VisitAstStructDecl(AstStructDecl& decl) -> void
            {
                if (decl.GetDeclToken() && !decl.GetDeclToken()->IsError()) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), *decl.GetDeclToken(),
                                                                  SemanticTokenType::Type,
                                                                  SemanticTokenModifier::Declaration));
                }
            }
            auto VisitAstInterfaceBlockDecl(AstInterfaceBlockDecl& decl) -> void
            {
                // Interface block name
                if (decl.GetDeclToken().IsIdentifier()) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), decl.GetDeclToken(),
                                                                  SemanticTokenType::Type,
                                                                  SemanticTokenModifier::Declaration));
                }

                // Interface block decl
                if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
                    tokenBuffer.push_back(
                        CreateSemanticTokenInfo(compileResult.GetLexContext(), decl.GetDeclarator()->declTok,
                                                SemanticTokenType::Variable, SemanticTokenModifier::Declaration));
                }
            }
            auto VisitAstFunctionDecl(AstFunctionDecl& decl) -> void
            {
                if (decl.GetName().IsIdentifier()) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), decl.GetName(),
                                                                  SemanticTokenType::Function,
                                                                  SemanticTokenModifier::Declaration));
                }
            }
            auto VisitAstParamDecl(AstParamDecl& decl) -> void
            {
                if (decl.GetDeclarator() && decl.GetDeclarator()->declTok.IsIdentifier()) {
                    tokenBuffer.push_back(
                        CreateSemanticTokenInfo(compileResult.GetLexContext(), decl.GetDeclarator()->declTok,
                                                SemanticTokenType::Parameter, SemanticTokenModifier::Declaration));
                }
            }
            auto VisitAstVariableDecl(AstVariableDecl& decl) -> void
            {
                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.IsIdentifier()) {
                        tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), declarator.declTok,
                                                                      SemanticTokenType::Variable,
                                                                      SemanticTokenModifier::Declaration));
                    }
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr) -> void
            {
                switch (expr.GetAccessType()) {
                case NameAccessType::Function:
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), expr.GetAccessName(),
                                                                  SemanticTokenType::Function));
                    break;
                case NameAccessType::Constructor:
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compileResult.GetLexContext(), expr.GetAccessName(),
                                                                  SemanticTokenType::Type));
                    break;
                case NameAccessType::Variable:
                case NameAccessType::Swizzle:
                    if (expr.GetAccessName().klass == TokenKlass::Identifier) {
                        tokenBuffer.push_back(CreateSemanticTokenInfo(
                            compileResult.GetLexContext(), expr.GetAccessName(), SemanticTokenType::Variable));
                    }
                    break;
                case NameAccessType::Unknown:
                default:
                    GLSLD_UNREACHABLE();
                }
            }
        };

        AstSemanticTokenCollector{compileResult, tokenBuffer}.TraverseAst(compileResult.GetAstContext());
    }

    auto ComputeSemanticTokens(const CompileResult& compileResult) -> lsp::SemanticTokens
    {
        std::vector<SemanticTokenInfo> tokenBuffer;
        CollectLexSemanticTokens(compileResult, tokenBuffer);
        CollectAstSemanticTokens(compileResult, tokenBuffer);
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

    auto ComputeSemanticTokensDelta(CompiledDependency& compiler) -> lsp::SemanticTokensDelta
    {
        GLSLD_NO_IMPL();
    }
} // namespace glsld