#pragma once
#include "DiagnosticContext.h"
#include "LexContext.h"
#include "ParsedAst.h"
#include "Tokenizer.h"
#include "Typing.h"

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
        ParseResult(T&& result) : result(std::forward<T>(result)), success(true)
        {
        }
        ParseResult(T&& result, bool success) : result(std::forward<T>(result)), success(success)
        {
        }

        operator bool()
        {
            return success;
        }

        T result;
        bool success;
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

        // Consume a ';' if available, otherwise issue an error and return
        // This is used to semi-infer a required ';'
        auto ParsePermissiveSemicolon() -> void
        {
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expecting ';'");
                // However, we don't do error recovery as if the ';' is inferred by the parser.
            }
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
            auto beginTokIndex = 0;
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                // empty decl
                // TODO: report a warning
                return;
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

            // FIXME: if can declare interface block?
            if (TryTestToken(TokenKlass::LBrace, 1) || TryTestToken(TokenKlass::LBrace)) {
                // interface blocks
                // for example, uniform UBO { ... }

                if (!TryTestToken(TokenKlass::Identifier)) {
                    // need a block name
                }
                auto blockBody = ParseStructBody();
                auto blockName = ParseVariableDeclarators();
                GLSLD_NO_IMPL();
            }
            else {
                auto type = ParseType(quals);
                if (TryTestToken(TokenKlass::LParen, 1)) {
                    // function decl
                    ast.AddFunction(ParseFunctionDecl(beginTokIndex, type));
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

#pragma region Parsing Decl

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

        // EXPECT: 'K_layout'
        // PARSE: layout_qual
        //        layout_qual := 'layout' '(' ')'
        //        layout_qual := 'layout' '(' layout_spec [',' layout_spec]... ')'
        //        layout_spec := 'ID'
        //        layout_spec := 'ID' '=' assignment_expr
        auto ParseLayoutQualifier() -> void
        {
        }

        // Try to parse a sequence of qualifiers
        // PARSE: qual_seq
        //      - qual_seq := ???
        auto ParseTypeQualifiers() -> AstTypeQualifierSeq*
        {
            auto beginTokIndex = GetTokenIndex();
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
            return CreateRangedAstNode<AstTypeQualifierSeq>(beginTokIndex, result);
        }

        // EXPECT: 'ID'
        // PARSE: declarator_list
        //      - declarator_list := declarator [',' declarator]...
        //      - declarator := 'ID' [array_spec] ['=' initializer]
        //      - initializer := ?
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

                result.push_back(VariableDeclarator{.declTok = id, .arraySize = arraySpec, .init = init});
                if (TryTestToken(TokenKlass::Comma)) {
                    ConsumeToken();
                }
                else {
                    break;
                }
            }

            return result;
        }

        // EXPECT: '{'
        // PARSE: struct_body
        //      - struct_body := '{' member_decl... '}'
        //      - member_decl := ?
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
        // PARSE: struct_definition
        //      - struct_definition := 'struct' ['ID'] struct_body
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

        // EXPECT: '['
        // PARSE: '[' [expr] ']'
        auto ParseArraySpec() -> AstArraySpec*
        {
            auto beginTokIndex = GetTokenIndex();
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

            return CreateRangedAstNode<AstArraySpec>(beginTokIndex, std::move(sizes));
        }

        // PARSE: type_spec
        //      - type_spec := struct_definition
        //      - type_spec := 'ID'
        //      - type_spec := 'K_???'
        auto ParseType(AstTypeQualifierSeq* quals) -> AstQualType*
        {
            if (TryTestToken(TokenKlass::K_struct)) {
                return ParseStructDefinition();
            }

            auto beginTokIndex = GetTokenIndex();
            if (TryTestToken(TokenKlass::Identifier) || GetBuiltinType(PeekToken()).has_value()) {
                auto tok = PeekToken();
                ConsumeToken();
                return CreateRangedAstNode<AstQualType>(beginTokIndex, quals, tok);
            }

            // TODO: handle error
            return nullptr;
        }

        // PARSE: qualified_type_spec
        //        qualified_type_spec := [qual_seq] type_spec
        auto ParseQualType() -> ParseResultType
        {
            auto qualifiers = ParseTypeQualifiers();
            return ParseType(qualifiers);
        }

        // EXPECT: '('
        // PARSE: func_param_list
        //      - func_param_list := '(' ')'
        //      - func_param_list := '(' 'K_void' ')'
        //      - func_param_list := '(' func_param_decl [',' func_param_decl]...  ')'
        //      - func_param_decl := qualified_type_spec 'ID'
        //
        // ERROR: skip after ) or before ;
        // WHAT'S AN ERROR SECTION?
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
                auto beginTokIndex = GetTokenIndex();
                auto [type, id]    = Seq(&ParseContext::ParseQualType, &ParseContext::ParseDeclId);

                if (type && id) {
                    result.push_back(CreateRangedAstNode<AstParamDecl>(beginTokIndex, *type, *id));
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

        // EXPECT: 'ID'
        // PARSE: func_decl
        //      - func_decl := 'ID' func_param_list ';'
        //      - func_decl := 'ID' func_param_list compound_stmt
        auto ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> AstFunctionDecl*
        {
            // assert, never fail
            auto id = *ParseDeclId();

            //
            auto params = ParseFunctionParamList();

            if (TryTestToken(TokenKlass::Semicolon)) {
                ConsumeToken();
                return CreateRangedAstNode<AstFunctionDecl>(beginTokIndex, returnType, id, std::move(params));
            }
            else if (TryTestToken(TokenKlass::LBrace)) {
                auto body = ParseCompoundStmt();
                return CreateRangedAstNode<AstFunctionDecl>(beginTokIndex, returnType, id, std::move(params), body);
            }

            // error recovery, return a function anyway
            GLSLD_NO_IMPL();
        }

        // EXPECT: 'K_precision'
        // PARSE: precision_decl
        //      - precision_decl := ???
        auto ParseTypePrecisionDecl() -> AstDecl*
        {
            GLSLD_NO_IMPL();
        }

#pragma endregion

#pragma region Parsing Expr

        // TODO: is associativity in grammar correct?
        // PARSE: expr
        //        expr := assignment_expr [',' assignment_expr]
        auto ParseExpr() -> AstExpr*;

        // PARSE: assignment_expr
        //      - assignment_expr := unary_expr '?=' assignment_expr
        //      - assignment_expr := binary_or_conditional_expr
        auto ParseAssignmentExpr() -> AstExpr*;

        // PARSE: binary_or_conditional_expr
        //      - binary_or_conditional_expr := binary_expr
        //      - binary_or_conditional_expr := binary_expr '?' expr ':' assignment_expr
        //
        // `firstTerm` is the first terminal in the condition
        auto ParseBinaryOrConditionalExpr(size_t beginTokIndex, AstExpr* firstTerm) -> AstExpr*;

        // PARSE: binary_expr
        //      - binary_expr := ??
        //
        // `firstTerm` for this is a unary expression, which might already be parsed
        auto ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*;

        // PARSE: unary_expr
        //        unary_expr := ['unary_op']... postfix_expr
        auto ParseUnaryExpr() -> AstExpr*;

        // PARSE: postfix_expr
        //      - postfix_expr := primary_expr [postfix]...
        //      - postfix := 'incdec'
        //      - postfix := '.' 'ID'
        //      - postfix := array_spec
        //      - postfix := func_arg_list
        auto ParsePostfixExpr() -> AstExpr*;

        // PARSE: primary_expr
        //      - primary_expr := 'ID'
        //      - primary_expr := 'constant'
        //      - primary_expr := '(' expr ')'
        auto ParsePrimaryExpr() -> AstExpr*;

        // EXPECT: '('
        auto ParseParenWrappedExpr() -> AstExpr*;

        // EXPECT: '('
        // PARSE: func_arg_list
        //      - func_arg_list := '(' ')'
        //      - func_arg_list := '(' 'K_void' ')'
        //      - func_arg_list := '(' assignment_expr [',' assignment_expr]... ')'
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