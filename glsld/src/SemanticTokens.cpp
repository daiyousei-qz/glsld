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
        int column;
        int length;
        SemanticTokenType type;
        SemanticTokenModifier modifier;
    };

    auto CreateSemanticTokenInfo(const LexContext& lexContext, const SyntaxRange& range, SemanticTokenType type,
                                 SemanticTokenModifier modifier = SemanticTokenModifier::None) -> SemanticTokenInfo
    {
        auto tokBeginLoc = lexContext.LookupSyntaxLocation(range.begin);
        auto tokEndLoc   = lexContext.LookupSyntaxLocation(range.end);
        GLSLD_ASSERT(tokBeginLoc.file == tokEndLoc.file);
        GLSLD_ASSERT(tokBeginLoc.line == tokEndLoc.line);

        return SemanticTokenInfo{
            .line     = tokBeginLoc.line,
            .column   = tokBeginLoc.column,
            .length   = tokEndLoc.column - tokBeginLoc.column,
            .type     = type,
            .modifier = modifier,
        };
    }

    auto CollectLexSemanticTokens(GlsldCompiler& compiler, std::vector<SemanticTokenInfo>& tokenBuffer) -> void
    {
        auto lexContext = compiler.GetLexContext();
        for (const auto& tok : lexContext.GetAllToken()) {
            std::optional<SemanticTokenType> type;

            if (IsKeywordToken(tok.klass)) {
                if (GetBuiltinType(tok)) {
                    type = SemanticTokenType::Type;
                }
                else {
                    type = SemanticTokenType::Keyword;
                }
            }
            else if (tok.klass == TokenKlass::IntegerConstant || tok.klass == TokenKlass::FloatConstant) {
                type = SemanticTokenType::Number;
            }

            if (type) {
                tokenBuffer.push_back(CreateSemanticTokenInfo(lexContext, tok.range, *type));
            }
        }
    }

    auto CollectAstSemanticTokens(GlsldCompiler& compiler, std::vector<SemanticTokenInfo>& tokenBuffer) -> void
    {
        struct AstSemanticTokenCollector : public AstVisitor<AstSemanticTokenCollector>
        {
            GlsldCompiler& compiler;
            std::vector<SemanticTokenInfo>& tokenBuffer;

            AstSemanticTokenCollector(GlsldCompiler& compiler, std::vector<SemanticTokenInfo>& tokenBuffer)
                : compiler(compiler), tokenBuffer(tokenBuffer)
            {
            }

            auto VisitAstFunctionDecl(AstFunctionDecl& decl)
            {
                if (decl.GetName().klass == TokenKlass::Identifier) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compiler.GetLexContext(), decl.GetName().range,
                                                                  SemanticTokenType::Function,
                                                                  SemanticTokenModifier::Declaration));
                }
            }

            auto VisitAstParamDecl(AstParamDecl& decl)
            {
                if (decl.GetName().klass == TokenKlass::Identifier) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compiler.GetLexContext(), decl.GetName().range,
                                                                  SemanticTokenType::Parameter,
                                                                  SemanticTokenModifier::Declaration));
                }
            }

            auto VisitAstVariableDecl(AstVariableDecl& decl)
            {
                for (const auto& declarator : decl.GetDeclarators()) {
                    if (declarator.declTok.klass == TokenKlass::Identifier) {
                        tokenBuffer.push_back(
                            CreateSemanticTokenInfo(compiler.GetLexContext(), declarator.declTok.range,
                                                    SemanticTokenType::Variable, SemanticTokenModifier::Declaration));
                    }
                }
            }

            auto VisitAstInvokeExpr(AstInvokeExpr& expr)
            {
                if (expr.GetInvocationType() == InvocationType::FunctionCall) {
                    if (auto funcId = expr.GetInvokedExpr()->As<AstNameAccessExpr>()) {
                        if (funcId->GetAccessName().klass == TokenKlass::Identifier) {
                            tokenBuffer.push_back(CreateSemanticTokenInfo(
                                compiler.GetLexContext(), funcId->GetAccessName().range, SemanticTokenType::Function));
                        }
                    }
                }
            }

            auto VisitAstNameAccessExpr(AstNameAccessExpr& expr)
            {
                if (expr.GetAccessName().klass == TokenKlass::Identifier) {
                    tokenBuffer.push_back(CreateSemanticTokenInfo(compiler.GetLexContext(), expr.GetAccessName().range,
                                                                  SemanticTokenType::Variable));
                }
            }
        };

        AstSemanticTokenCollector{compiler, tokenBuffer}.TraverseAst(compiler.GetAstContext());
    }

    auto ComputeSemanticTokens(GlsldCompiler& compiler) -> lsp::SemanticTokens
    {
        std::vector<SemanticTokenInfo> tokenBuffer;
        CollectLexSemanticTokens(compiler, tokenBuffer);
        CollectAstSemanticTokens(compiler, tokenBuffer);
        std::ranges::sort(tokenBuffer, [](const SemanticTokenInfo& lhs, const SemanticTokenInfo& rhs) {
            return std::tie(lhs.line, lhs.column) < std::tie(rhs.line, rhs.column);
        });

        lsp::SemanticTokens result;
        int lastTokLine   = -1;
        int lastTokColumn = -1;
        for (const auto& tokInfo : tokenBuffer) {
            if (lastTokLine == -1) {
                result.data.push_back(tokInfo.line);
                result.data.push_back(tokInfo.column);
            }
            else {
                if (lastTokLine == tokInfo.line) {
                    result.data.push_back(0);
                    result.data.push_back(tokInfo.column - lastTokColumn);
                }
                else {
                    result.data.push_back(tokInfo.line - lastTokLine);
                    result.data.push_back(tokInfo.column);
                }
            }

            result.data.push_back(tokInfo.length);
            result.data.push_back(GetTokenTypeIndex(tokInfo.type));
            result.data.push_back(GetTokenModifierIndex(tokInfo.modifier));

            lastTokLine   = tokInfo.line;
            lastTokColumn = tokInfo.column;
        }

        return result;
    }

    auto ComputeSemanticTokensDelta(GlsldCompiler& compiler) -> lsp::SemanticTokensDelta
    {
        GLSLD_NO_IMPL();
    }
} // namespace glsld