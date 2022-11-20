#pragma once
#include "DiagnosticContext.h"
#include "LexContext.h"
#include "ParsedAst.h"
#include "Tokenizer.h"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace glsld
{
    class ParseContext;

    template <typename T>
    struct ParseResult
    {
        std::optional<T*> result;
    };

    using ParseResultExpr = std::optional<AstExpr*>;
    using ParseResultStmt = std::optional<AstStmt*>;
    using ParseResultDecl = std::optional<AstDecl*>;
    using ParseResultType = std::optional<AstQualType*>;

    template <typename Parser>
    using ParserResultType = std::invoke_result_t<Parser, ParseContext>;
    class ParseContext
    {
    public:
        ParseContext(DiagnosticContext* diagCtx, LexContext* lexer) : diagCtx(diagCtx), lexer(lexer)
        {
            RestoreTokenIndex(0);
        }

        auto GetAst() -> const ParsedAst*
        {
            return &ast;
        }

        auto Eof() -> bool
        {
            return PeekToken().klass == TokenKlass::Eof;
        }

        template <typename Parser>
        auto SepBy(TokenKlass sep, Parser elemParser)
        {
            std::vector<int> result;
        }

        template <typename Parser1, typename Parser2>
        auto Seq(Parser1 parser1, Parser2 parser2) -> std::tuple<ParserResultType<Parser1>, ParserResultType<Parser2>>
        {
            auto result1 = std::invoke(parser1, this);
            if (!result1) {
                return {std::move(result1), std::nullopt};
            }

            auto result2 = std::invoke(parser2, this);
            return {std::move(result1), std::move(result2)};
        }

        auto ParsePermissiveSemicolon() -> void
        {
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expecting ';'");
                // However, we don't do error recovery as if the ';' is inferred by the parser.
            }
        }

        auto ParseDeclId() -> std::optional<AstDeclId*>
        {
            if (PeekToken().klass == TokenKlass::Identifier) {
                auto result = CreateAstNode<AstDeclId>({}, PeekToken().text);
                ConsumeToken();
                return result;
            }
            else {
                // error
                return nullptr;
            }
        }

        auto ParseTypeQualifiers() -> AstTypeQualifierSeq*
        {
            AstTypeQualifierSeq result;

            bool inQualSeq = true;
            while (inQualSeq) {
                // FIXME: implement correctly
                switch (PeekToken().klass) {
                    // precision qualifier
                case TokenKlass::K_highp:
                    result.SetHighp();
                    break;
                case TokenKlass::K_mediump:
                    result.SetMediump();
                    break;
                case TokenKlass::K_lowp:
                    result.SetLowp();
                    break;
                    // invariance qual?
                    // precise qual?
                    // memory qualifier
                case TokenKlass::K_coherent:
                case TokenKlass::K_volatile:
                case TokenKlass::K_restrict:
                case TokenKlass::K_readonly:
                case TokenKlass::K_writeonly:
                    break;
                    // layout qual
                case TokenKlass::K_layout:
                    break;
                    // storage qual (also, parameter qual)
                case TokenKlass::K_const:
                    result.SetConst();
                    break;
                case TokenKlass::K_in:
                    result.SetIn();
                    break;
                case TokenKlass::K_out:
                    result.SetOut();
                    break;
                case TokenKlass::K_inout:
                    result.SetInout();
                    break;
                case TokenKlass::K_attribute:
                    result.SetAttribute();
                    break;
                case TokenKlass::K_uniform:
                    result.SetUniform();
                    break;
                case TokenKlass::K_varying:
                    result.SetVarying();
                    break;
                case TokenKlass::K_buffer:
                    result.SetBuffer();
                    break;
                case TokenKlass::K_shared:
                    result.SetShared();
                    break;
                    // auxiliary storage qual
                case TokenKlass::K_centroid:
                    result.SetCentroid();
                    break;
                case TokenKlass::K_sample:
                    result.SetSample();
                    break;
                case TokenKlass::K_patch:
                    result.SetPatch();
                    break;
                default:
                    inQualSeq = false;
                    break;
                }

                if (inQualSeq) {
                    ConsumeToken();
                }
            }

            // TODO: return nullptr if not set
            return CreateAstNode<AstTypeQualifierSeq>({}, result);
        }

        // EXPECT: ID
        // PARSE: (ID[N] = init) ,...
        auto ParseVariableDeclarators() -> std::vector<VariableDeclarator>
        {
            std::vector<VariableDeclarator> result;

            while (!Eof()) {
                AstDeclId* id = *ParseDeclId();

                AstArraySpec* arraySpec = nullptr;
                if (TryTestToken(TokenKlass::LBracket)) {
                    arraySpec = ParseArraySpec();
                }

                AstExpr* init = nullptr;
                if (TryTestToken(TokenKlass::Assign)) {
                    // parse init
                    GLSLD_NO_IMPL();
                }

                result.push_back(VariableDeclarator{.id = id, .arraySize = arraySpec, .init = init});
                if (TryTestToken(TokenKlass::Comma)) {
                    ConsumeToken();
                }
                else {
                    break;
                }
            }

            return result;
        }

        // EXPECT: LBrace
        // PARSE: '{' member_decl... '}'
        auto ParseStructBody() -> std::vector<AstStructMemberDecl*>
        {
            ConsumeTokenAssert(TokenKlass::LBrace);

            if (TryTestToken(TokenKlass::RBrace)) {
                // empty struct body
                return {};
            }

            std::vector<AstStructMemberDecl*> result;
            while (true) {
                // TODO: parse layout qualifiers
                AstQualType* type = ParseType(nullptr);
                // TODO: support multiple declarators
                AstDeclId* id = *ParseDeclId();

                if (!TryTestToken(TokenKlass::Semicolon, TokenKlass::RBrace)) {
                    // error recovery
                }
                if (TryTestToken(TokenKlass::Semicolon)) {
                    ConsumeToken();
                }
                if (TryTestToken(TokenKlass::RBrace)) {
                    ConsumeToken();
                    break;
                }
            }
            return result;
        }

        // EXPECT: K_struct
        auto ParseStructDefinition() -> AstQualType*
        {
            ConsumeTokenAssert(TokenKlass::K_struct);
            AstDeclId* name = nullptr;
            if (TryTestToken(TokenKlass::Identifier)) {
                // TODO: parse name
            }

            if (TryTestToken(TokenKlass::LBrace)) {
                ParseStructBody();
            }

            // error recovery
            GLSLD_NO_IMPL();
        }

        // EXPECT: LBracket
        // PARSE: [N]...
        auto ParseArraySpec() -> AstArraySpec*
        {
            std::vector<AstExpr*> sizes;
            while (TryTestToken(TokenKlass::LBracket)) {
                ConsumeTokenAssert(TokenKlass::LBracket);

                if (TryTestToken(TokenKlass::RBracket)) {
                    sizes.push_back(nullptr);
                    continue;
                }

                AstExpr* dimSize = ParseExpr();
                sizes.push_back(dimSize);

                if (TryTestToken(TokenKlass::RBracket)) {
                    ConsumeToken();
                }
                else {
                    ReportError("expecting ]");
                }
            }

            return CreateAstNode<AstArraySpec>({}, std::move(sizes));
        }

        auto ParseType(AstTypeQualifierSeq* quals) -> AstQualType*
        {
            if (TryTestToken(TokenKlass::K_struct)) {
                return ParseStructDefinition();
            }

            switch (PeekToken().klass) {
            case TokenKlass::K_void:
                ConsumeToken();
                return CreateAstNode<AstQualType>({}, "void", quals);
            case TokenKlass::K_int:
                ConsumeToken();
                return CreateAstNode<AstQualType>({}, "int", quals);
            case TokenKlass::K_float:
                ConsumeToken();
                return CreateAstNode<AstQualType>({}, "float", quals);
            default:
                // error
                return nullptr;
            }
        }
        auto ParseQualType() -> ParseResultType
        {
            auto qualifiers = ParseTypeQualifiers();
            return ParseType(qualifiers);
        }

        // EXPECT: (
        // PARSE: (...)
        // ERROR: skip after ) or before ;
        auto ParseFunctionParamList() -> std::vector<AstParamDecl*>
        {
            std::vector<AstParamDecl*> result;

            ConsumeTokenAssert(TokenKlass::LParen);

            // empty parameter list
            if (TryConsumeToken(TokenKlass::RParen)) {
                return result;
            }

            // TODO: needed? spec doesn't include this grammar.
            if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
                ConsumeToken();
                ConsumeToken();
                return {};
            }

            // parse parameters
            while (PeekToken().klass != TokenKlass::Eof) {
                auto [type, id] = Seq(&ParseContext::ParseQualType, &ParseContext::ParseDeclId);

                if (type && id) {
                    result.push_back(CreateAstNode<AstParamDecl>({}, *type, *id));
                }

                if (TryTestToken(TokenKlass::Comma)) {
                    ConsumeToken();
                }
                else {
                    break;
                }
            }

            if (TryTestToken(TokenKlass::RParen)) {
                ConsumeToken();
            }
            else {
                RecoverFromError(RecoveryMode::Paren);
            }

            return result;
        }

        // EXPECT: {
        // PARSE: { ... }
        auto ParseFunctionBody() -> AstCompoundStmt*
        {
            return ParseCompoundStmt();
        }

        // EXPECT: ID
        // PARSE: ID(...) [{...}]
        auto ParseFunctionDecl(AstQualType* returnType) -> AstFunctionDecl*
        {
            // assert, never fail
            auto id = *ParseDeclId();

            //
            auto params = ParseFunctionParamList();

            if (TryTestToken(TokenKlass::Semicolon)) {
                ConsumeToken();
                return CreateAstNode<AstFunctionDecl>({}, returnType, id, std::move(params));
            }
            else if (TryTestToken(TokenKlass::LBrace)) {
                auto body = ParseFunctionBody();
                return CreateAstNode<AstFunctionDecl>({}, returnType, id, std::move(params), body);
            }

            // error recovery, return a function anyway
            GLSLD_NO_IMPL();
        }

        // EXPECT: K_precision
        auto ParseTypePrecisionDecl() -> AstDecl*
        {
            GLSLD_NO_IMPL();
        }

        // - global (in/out/uniform)
        //   - type-qual? type-spec;
        //   - type-qual? type-spec id = init;
        //   - type-qual? type-spec id[N] = init;
        //   - type-qual? type-spec id = init, ...;
        // - precision settings
        //   - precision precision-qual type;
        // - other settings
        //   - type-qual;
        //   - type-qual id;
        //   - type-qual id, ...;
        // - block {in/out/uniform/buffer}
        //   - type-qual id { ... };
        //   - type-qual id { ... } id;
        //   - type-qual id { ... } id[N];
        // - struct
        //   - struct id? { ... };
        //   - struct id? { ... } id;
        //   - struct id? { ... } id[N];
        // - function
        //   - type-qual? type-spec id(...);
        //   - type-qual? type-spec id(...) { ... }
        auto DoParseExternalDecl() -> void
        {
            if (TryTestToken(TokenKlass::Semicolon)) {
                // empty decl
                GLSLD_NO_IMPL();
            }

            if (TryTestToken(TokenKlass::K_precision)) {
                // precision decl
                GLSLD_NO_IMPL();
                // return ParseTypePrecisionDecl();
            }

            auto quals = ParseTypeQualifiers();
            if (TryTestToken(TokenKlass::Semicolon)) {
                // early return
                // for example, "layout(...) in;"
                GLSLD_NO_IMPL();
            }

            if (TryTestToken(TokenKlass::LBrace, 1) || TryTestToken(TokenKlass::LBrace)) {
                // interface blocks
                // for example, uniform UBO { ... }
                if (!TryTestToken(TokenKlass::Identifier)) {
                    // need a block name
                }
                GLSLD_NO_IMPL();
            }
            else {
                auto type = ParseType(quals);
                if (TryTestToken(TokenKlass::LParen, 1)) {
                    // function decl
                    ast.AddFunction(ParseFunctionDecl(type));
                }
                else {
                    // variable decl
                    auto decl = CreateAstNode<AstVariableDecl>({}, type, ParseVariableDeclarators());
                    ast.AddGlobalVariable(decl);
                }
            }

            // if error
            if (false) {
                RecoverFromError(RecoveryMode::GlobalSemi);
            }

            if (TryTestToken(TokenKlass::Semicolon)) {
                ConsumeToken();
            }
            else {
                // TODO: sometimes ; is not needed, like function definition
            }
        }

        auto DoParseTranslationUnit() -> void
        {
            while (!Eof()) {
                DoParseExternalDecl();
            }
        }

        // EXPECT: '('

        auto ParseFunctionArgumentList() -> std::vector<AstExpr*>
        {
            ConsumeTokenAssert(TokenKlass::LParen);

            if (TryConsumeToken(TokenKlass::RParen)) {
                return {};
            }

            if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
                ConsumeToken();
                ConsumeToken();
                return {};
            }

            std::vector<AstExpr*> result;
            while (!Eof()) {
                auto arg = ParseAssignmentExpr();
                result.push_back(arg);

                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }

            // FIXME: error handling
            if (!TryConsumeToken(TokenKlass::RParen)) {
                GLSLD_NO_IMPL();
            }

            return result;
        }

        auto ParseIndexingAccess() -> void
        {
        }

#pragma region Parsing Expr

        auto ParseExpr() -> AstExpr*;

        auto ParseAssignmentExpr() -> AstExpr*;

        auto ParseBinaryOrConditionalExpr(size_t beginTokIndex, AstExpr* firstTerm) -> AstExpr*;

        auto ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*;

        auto ParseUnaryExpr() -> AstExpr*;

        auto ParsePostfixExpr() -> AstExpr*;

        auto ParsePrimaryExpr() -> AstExpr*;

        auto ParseParenWrappedExpr() -> AstExpr*;

#pragma endregion

#pragma region Parsing Stmt

        auto ParseStmt() -> AstExpr*;

        auto ParseCompoundStmt() -> AstCompoundStmt*;

        auto ParseSelectionStmt() -> AstIfStmt*;

        auto ParseForStmt() -> AstForStmt*;

        auto ParseWhileStmt() -> AstWhileStmt*;

        auto ParseSwitchStmt() -> AstSwitchStmt*;

        auto ParseJumpStmt() -> AstExpr*;

        auto ParseDeclOrExprStmt() -> AstStmt*;

#pragma endregion

        enum class RecoveryMode
        {
            // Assuming a leading '('
            // Skip until we i) consumed next balanced ')' ii) stopped before next ';' in the same scope
            Paren = static_cast<int>(TokenKlass::RParen),

            // Assuming a leading '['
            // Skip until we i) consumed next balanced ']' ii) stopped before next ';' in the same scope
            Bracket = static_cast<int>(TokenKlass::RBracket),

            // Assuming a leading '{'
            // Skip until we i)  stopped before balanced '}' ii) stopped before next ';' in the parent scope
            Brace = static_cast<int>(TokenKlass::RBrace),

            // Assuming a leading '{'
            // Skip until we i) stopped before next ';' in the same scope ii) stopped before next balanced '}'
            LocalSemi = static_cast<int>(TokenKlass::Semicolon),

            // Skip until we consumed next ';' in the same scope
            GlobalSemi = static_cast<int>(TokenKlass::Semicolon),
        };

        // FIXME: implement correctly
        auto RecoverFromError(RecoveryMode mode) -> int
        {
            auto desiredToken = static_cast<TokenKlass>(mode);

            // skip tokens until
            // 1. next ';' in the same or outer scope
            // 2. next '}' that end the current scope
            std::vector<TokenKlass> pendingClose;
            while (PeekToken().klass != TokenKlass::Eof) {
                switch (PeekToken().klass) {
                case TokenKlass::LParen:
                    pendingClose.push_back(TokenKlass::RParen);
                    break;
                case TokenKlass::LBracket:
                    pendingClose.push_back(TokenKlass::RBracket);
                    break;
                case TokenKlass::LBrace:
                    pendingClose.push_back(TokenKlass::RBrace);
                    break;
                case TokenKlass::RParen:
                case TokenKlass::RBracket:
                case TokenKlass::RBrace:
                    if (pendingClose.empty() && PeekToken().klass == desiredToken) {
                        return 1;
                    }
                    break;
                case TokenKlass::Semicolon:
                    if (pendingClose.empty()) {
                        return desiredToken == TokenKlass::Semicolon ? 1 : 0;
                    }
                    break;
                default:
                    break;
                }
            }

            return 0;
        }

        auto TryTestToken(TokenKlass klass, int lookahead = 0) -> bool
        {
            const auto& tok = lookahead == 0 ? PeekToken() : PeekToken(lookahead);
            if (tok.klass == klass) {
                return true;
            }
            else {
                return false;
            }
        }
        auto TryTestToken(TokenKlass klass1, TokenKlass klass2, int lookahead = 0) -> bool
        {
            const auto& tok = lookahead == 0 ? PeekToken() : PeekToken(lookahead);
            if (tok.klass == klass1 || tok.klass == klass2) {
                return true;
            }
            else {
                return false;
            }
        }
        auto TryTestToken(TokenKlass klass1, TokenKlass klass2, TokenKlass klass3, int lookahead = 0) -> bool
        {
            const auto& tok = lookahead == 0 ? PeekToken() : PeekToken(lookahead);
            if (tok.klass == klass1 || tok.klass == klass2 || tok.klass == klass3) {
                return true;
            }
            else {
                return false;
            }
        }

        auto ReportError(size_t tokIndex, std::string message) -> void
        {
            diagCtx->ReportError(lexer->GetSyntaxRange(tokIndex), std::move(message));
        }
        auto ReportError(std::string message) -> void
        {
            ReportError(GetTokenIndex(), std::move(message));
        }

        auto PeekToken() -> const SyntaxToken&
        {
            return currentTok;
        }

        auto PeekToken(size_t lookahead) -> SyntaxToken
        {
            return lexer->GetToken(currentTokIndex + lookahead);
        }

        auto GetTokenIndex() -> size_t
        {
            return currentTokIndex;
        }

        auto RestoreTokenIndex(size_t index) -> void
        {
            currentTokIndex = index;
            currentTok      = lexer->GetToken(currentTokIndex);
        }

        auto ConsumeToken() -> void
        {
            RestoreTokenIndex(currentTokIndex + 1);
        }

        auto ConsumeTokenAssert(TokenKlass klass) -> void
        {
            GLSLD_ASSERT(PeekToken().klass == klass);
            ConsumeToken();
        }

        // TODO: error recovery
        auto TryConsumeToken(TokenKlass klass) -> bool
        {
            if (PeekToken().klass == klass) {
                ConsumeToken();
                return true;
            }
            else {
                return false;
            }
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<SyntaxNode, T>
        auto CreateRangedAstNode2(size_t beginTokIndex, size_t endTokIndex, Args&&... args) -> T*
        {
            auto result = new T{std::forward<Args>(args)...};
            result->UpdateRange(lexer->GetSyntaxRange(beginTokIndex, endTokIndex));
            return result;
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<SyntaxNode, T>
        auto CreateRangedAstNode(size_t beginTokIndex, Args&&... args) -> T*
        {
            auto result = new T{std::forward<Args>(args)...};
            result->UpdateRange(lexer->GetSyntaxRange(beginTokIndex, GetTokenIndex()));
            return result;
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<SyntaxNode, T>
        auto CreateAstNode(const SyntaxRange& range, Args&&... args) -> T*
        {
            auto result = new T{std::forward<Args>(args)...};
            result->UpdateRange(range);
            return result;
        }

    private:
        LexContext* lexer      = nullptr;
        size_t currentTokIndex = 0;
        SyntaxToken currentTok = {};

        ParsedAst ast;

        DiagnosticContext* diagCtx;
    };

} // namespace glsld