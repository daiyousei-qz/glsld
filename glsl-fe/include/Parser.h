#pragma once
#include "DiagnosticContext.h"
#include "LexContext.h"
#include "AstContext.h"
#include "Tokenizer.h"
#include "Typing.h"
#include "ParserTrace.h"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace glsld
{
    template <typename T>
    class ParseResult
    {
    public:
        ParseResult(T result) : result(std::move(result)), success(true)
        {
        }
        ParseResult(bool success, T result) : result(std::move(result)), success(success)
        {
        }

        auto Success() -> bool
        {
            return success;
        }

        auto Get() -> const T&
        {
            return result;
        }

        auto Move() -> T&&
        {
            return std::move(result);
        }

        operator bool()
        {
            return success;
        }

    private:
        T result;

        // true if parser accept the input
        // false otherwise, meaning parser is in recovery mode
        // NOTE even if the parser is in recovery, it would still provide a best-effort result.
        bool success;
    };

    template <typename T>
    using ParseSuccuss = ParseResult<T>;

    class Parser
    {
    public:
        Parser(LexContext* lexCtx, AstContext* astCtx, DiagnosticContext* diagCtx)
            : lexer(lexCtx), astCtx(astCtx), diagCtx(diagCtx)
        {
            RestoreTokenIndex(0);
        }

        auto Eof() -> bool
        {
            return PeekToken().klass == TokenKlass::Eof;
        }

        auto DoParseTranslationUnit() -> void
        {
            TRACE_PARSER();

            while (!Eof()) {
                auto declResult = ParseDeclaration();
                if (!declResult.Success()) {
                    RecoverFromBadDecl();
                }

                astCtx->AddGlobalDecl(declResult.Get());
                // FIXME: register decl
            }
        }

#pragma region Parsing Misc

        // Consume a ';' if available, otherwise issue an error and return
        // This is used to semi-infer a required ';'
        //
        // PARSE: ';'
        //
        // ACCEPT: null
        auto ParsePermissiveSemicolon() -> void
        {
            if (!TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expecting ';'");
                // However, we don't do error recovery as if the ';' is inferred by the parser.
            }
        }

        // Parse an identifier token as a symbol name if available, otherwise returns an error token
        //
        // PARSE: 'ID'
        //
        // ACCEPT: null
        auto ParseDeclId() -> SyntaxToken
        {
            if (PeekToken().klass == TokenKlass::Identifier) {
                auto result = PeekToken();
                ConsumeToken();
                return result;
            }
            else {
                // FIXME: which token should we return? RECOVERY?
                return SyntaxToken{};
            }
        }

#pragma endregion

#pragma region Parsing QualType

        // EXPECT: 'K_layout'
        // PARSE: layout_qual
        //        layout_qual := 'layout' '(' ')'
        //        layout_qual := 'layout' '(' layout_spec [',' layout_spec]... ')'
        //        layout_spec := 'ID'
        //        layout_spec := 'ID' '=' assignment_expr
        // ACCEPT: 'K_layout' '(' ??? ')'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseLayoutQualifier() -> void;

        // Try to parse a sequence of qualifiers
        // PARSE: qual_seq
        //      - qual_seq := ['qual_keyword']...
        //
        // FIXME: layout qual
        auto ParseTypeQualifiers() -> AstTypeQualifierSeq*;

        // EXPECT: '{'
        // PARSE: struct_body
        //      - struct_body := '{' member_decl... '}'
        //      - member_decl := ?
        //
        // ACCEPT: '{' ??? '}'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseStructBody() -> ParseResult<std::vector<AstStructMemberDecl*>>;

        // EXPECT: K_struct
        //
        // PARSE: struct_definition
        //      - struct_definition := 'struct' ['ID'] struct_body
        //
        // RECOVERY:
        auto ParseStructDefinition() -> ParseResult<AstStructDecl*>;

        // EXPECT: '['
        // PARSE: array_spec
        //      - array_spec := ('[' [expr] ']')...
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseArraySpec() -> ParseResult<AstArraySpec*>;

        // PARSE: type_spec
        //      - type_spec := struct_definition
        //      - type_spec := 'ID'
        //      - type_spec := 'K_???'
        //
        // RECOVERY:
        auto ParseType(AstTypeQualifierSeq* quals) -> ParseResult<AstQualType*>;

        // PARSE: qualified_type_spec
        //        qualified_type_spec := [qual_seq] type_spec
        auto ParseQualType() -> ParseResult<AstQualType*>;

#pragma endregion

#pragma region Parsing Decl

        // PARSE: declaration
        //      - declaration := ';'
        //      - declaration := type_decl
        //      - declaration := variable_decl
        //      - declaration := function_decl
        //
        // ACCEPT: ??? ';'
        // FIXME: fill declaration BNF
        //
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
        auto ParseDeclaration() -> ParseResult<AstDecl*>;

        auto ParseDeclNoRecovery() -> ParseSuccuss<AstDecl*>;

        // EXPECT: 'ID'
        // PARSE: declarator_list
        //      - declarator_list := declarator [',' declarator]...
        //      - declarator := 'ID' [array_spec] ['=' initializer]
        //      - initializer := ?
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseVariableDeclarators() -> ParseResult<std::vector<VariableDeclarator>>;

        // EXPECT: '('
        //
        // PARSE: func_param_list
        //      - func_param_list := '(' ')'
        //      - func_param_list := '(' 'K_void' ')'
        //      - func_param_list := '(' func_param_decl [',' func_param_decl]...  ')'
        //      - func_param_decl := qualified_type_spec 'ID'
        //
        // RECOVERY: ^'EOF' or ^';'
        // WHAT'S AN ERROR SECTION?
        auto ParseFunctionParamList() -> ParseResult<std::vector<AstParamDecl*>>;

        // EXPECT: 'ID' or '('
        //
        // PARSE: func_decl
        //      - func_decl := 'ID' func_param_list ';'
        //      - func_decl := 'ID' func_param_list compound_stmt
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> ParseResult<AstDecl*>;

        // EXPECT: ';' or 'ID'
        //
        // PARSE: type_or_variable_decl
        //      - type_or_variable_decl := ';'
        //      - type_or_variable_decl := declarator [',' declarator]... ';'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseTypeOrVariableDecl(size_t beginTokIndex, AstQualType* variableType) -> ParseResult<AstDecl*>;

        // EXPECT: 'ID' or '{'
        // RECOVERY: ^'EOF' or ^';'
        auto ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> ParseResult<AstDecl*>;

        // EXPECT: 'K_precision'
        // PARSE: precision_decl
        //      - precision_decl := ???
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParsePrecisionDecl() -> ParseResult<AstDecl*>;

#pragma endregion

#pragma region Parsing Expr

        // Entry function to parse any expression AST.
        // This function always return a valid AstExpr*. In presence of a parsing error, an AstErrorExpr* will be used
        // to replace the expression failed to be constructed.
        // TODO: is associativity in grammar correct?
        // PARSE: expr
        //        expr := assignment_expr [',' assignment_expr]
        //
        // RECOVERY: ^';' or ^'}' or ^'EOF'
        auto ParseExpr() -> ParseResult<AstExpr*>;

        // PARSE: assignment_expr
        //      - assignment_expr := unary_expr '?=' assignment_expr
        //      - assignment_expr := binary_or_conditional_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseAssignmentExpr() -> ParseResult<AstExpr*>;

        // PARSE: binary_or_conditional_expr
        //      - binary_or_conditional_expr := binary_expr
        //      - binary_or_conditional_expr := binary_expr '?' expr ':' assignment_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        //
        // `firstTerm` is the first terminal in the condition
        auto ParseBinaryOrConditionalExpr(size_t beginTokIndex, ParseResult<AstExpr*> firstTerm)
            -> ParseResult<AstExpr*>;

        // PARSE: binary_expr
        //      - binary_expr := unary_expr
        //      - binary_expr := binary_expr 'binary_op' binary_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        //
        // `firstTerm` for this is a unary expression, which might already be parsed
        auto ParseBinaryExpr(size_t beginTokIndex, ParseResult<AstExpr*> firstTerm, int minPrecedence)
            -> ParseResult<AstExpr*>;

        // PARSE: unary_expr
        //        unary_expr := ['unary_op']... postfix_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseUnaryExpr() -> ParseResult<AstExpr*>;

        // PARSE: postfix_expr
        //      - postfix_expr := primary_expr
        //      - postfix_expr := postfix_expr 'incdec'
        //      - postfix_expr := postfix_expr '.' 'ID'
        //      - postfix_expr := postfix_expr array_spec
        //      - postfix_expr := function_call
        //      - function_call := function_identifier func_arg_list
        //      - function_identifier := postfix_expr
        //      - function_identifier := type_spec
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParsePostfixExpr() -> ParseResult<AstExpr*>;

        // PARSE: primary_expr
        //      - primary_expr := 'ID'
        //      - primary_expr := 'constant'
        //      - primary_expr := paren_wrapped_expr
        //
        // ACCEPT: null
        auto ParsePrimaryExpr() -> ParseResult<AstExpr*>;

        // EXPECT: '('
        //
        // PARSE: paren_wrapped_expr
        //      - paren_wrapped_expr := '(' expr ')'
        //
        // ACCEPT: '('  ??? ')'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseParenWrappedExpr() -> ParseResult<AstExpr*>;

        auto ParseConstructorCall() -> ParseResult<AstExpr*>
        {
            TRACE_PARSER();

            auto beginTokIndex = GetTokenIndex();

            // Parse type
            auto typeResult = ParseType(nullptr);
            if (!typeResult.Success() || !TryTestToken(TokenKlass::LParen)) {
                return {false, CreateErrorExpr()};
            }
            auto ctorExpr = CreateAstNode<AstConstructorExpr>(beginTokIndex, typeResult.Get());

            // Parse argument list
            auto argListResult = ParseFunctionArgumentList();
            return {argListResult.Success(), CreateAstNode<AstInvokeExpr>(beginTokIndex, InvocationType::FunctionCall,
                                                                          ctorExpr, argListResult.Move())};
        }

        // EXPECT: '('
        //
        // PARSE: func_arg_list
        //      - func_arg_list := '(' ')'
        //      - func_arg_list := '(' 'K_void' ')'
        //      - func_arg_list := '(' assignment_expr [',' assignment_expr]... ')'
        //
        // ACCEPT: '(' ??? ')'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseFunctionArgumentList() -> ParseResult<std::vector<AstExpr*>>
        {
            TRACE_PARSER();
            ConsumeTokenAssert(TokenKlass::LParen);

            if (TryConsumeToken(TokenKlass::RParen)) {
                return {{}};
            }

            if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
                ConsumeToken();
                ConsumeToken();
                return {{}};
            }

            std::vector<AstExpr*> result;
            while (!Eof()) {
                auto arg = ParseAssignmentExpr();
                if (arg.Success()) {
                    result.push_back(arg.Move());
                }
                else {
                    break;
                }

                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }

            // consume ")"
            // FIXME: handle closed paren in unified way?
            bool closed = TryConsumeToken(TokenKlass::RParen);
            if (!closed) {
                ReportError("expect ')'");

                RecoverFromError(RecoveryMode::Paren);
                closed = TryConsumeToken(TokenKlass::RParen);
            }

            return {closed, std::move(result)};
        }

        // EXPECT: '['
        //
        // PARSE: index_list
        //      - index_list := '[' expr ']'
        //
        // ACCEPT: '[' ??? ']'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseIndexingAccess() -> void
        {
            TRACE_PARSER();
        }

#pragma endregion

#pragma region Parsing Stmt

        // PARSE: stmt
        //      - stmt := compound_stmt
        //      - stmt := selection_stmt
        //      - stmt := for_stmt
        //      - stmt := while_stmt
        //      - stmt := switch_stmt
        //      - stmt := jump_stmt
        //      - stmt := decl_stmt
        //      - stmt := expr_stmt
        auto ParseStmt() -> ParseResult<AstStmt*>;

        auto ParseStmtNoRecovery() -> ParseSuccuss<AstStmt*>;

        // EXPECT: '{'
        //
        // PARSE: compound_stmt
        //      - compound_stmt := '{' [stmt]... '}'
        //
        // ACCEPT: '{' ??? '}'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseCompoundStmt() -> ParseResult<AstStmt*>;

        // EXPECT: 'K_if'
        //
        // PARSE: selection_stmt
        //      - selection_stmt := 'K_if' paren_wrapped_expr stmt
        //      - selection_stmt := 'K_if' paren_wrapped_expr stmt 'K_else' selection_stmt
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        // FIXME: check with if-else nesting
        auto ParseSelectionStmt() -> ParseResult<AstStmt*>;

        // EXPECT: 'K_for'
        //
        // PARSE: for_stmt
        //      - for_stmt := 'K_for' for_pred stmt
        //      - for_pred := '(' stmt stmt stmt ')'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        // FIXME: only simple stmt is allowed in for loop predicate?
        auto ParseForStmt() -> ParseResult<AstStmt*>;

        // EXPECT: 'K_while'
        //
        // PARSE: while_stmt
        //      - while_stmt := 'K_while' paren_wrapped_expr stmt
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseWhileStmt() -> ParseResult<AstStmt*>;

        // EXPECT: 'K_switch'
        //
        // PARSE: switch_stmt
        //      - switch_stmt := 'K_switch' paren_wrapped_expr switch_body
        //      - switch_body := '{' ??? '}'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseSwitchStmt() -> ParseResult<AstStmt*>;

        // EXPECT: 'K_break' or 'K_continue' or 'K_discard' or 'K_return'
        //
        // PARSE: jump_stmt
        //      - jump_stmt := 'K_break' [';']
        //      - jump_stmt := 'K_continue' [';']
        //      - jump_stmt := 'K_discard' [';']
        //      - jump_stmt := 'K_return' [expr] [';']
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseJumpStmt() -> ParseResult<AstStmt*>;

        // PARSE: expr_stmt
        //      - expr_stmt := expr [';']
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseExprStmt() -> ParseResult<AstStmt*>;

        // PARSE: decl_or_expr_stmt
        //      - decl_or_expr_stmt := expr_stmt
        //      - decl_or_expr_stmt := declaration
        //
        // RECOVERY: ???
        // FIXME: what's in recovery mode?
        auto ParseDeclOrExprStmt() -> ParseResult<AstStmt*>;

#pragma endregion

        enum class RecoveryMode
        {
            // Assuming a non-zero depth of paren, skip until we see
            // 1) ')' that ends the leading '('
            // 1) ';' in this scope
            // 2) '}' that ends this scope (N/A in the global scope)
            Paren,

            // Assuming a non-zero depth of bracket, skip until we see
            // 1) ']' that ends the leading '['
            // 1) ';' in this scope
            // 2) '}' that ends this scope (N/A in the global scope)
            Bracket,

            // Skip until we see
            // 1) '}' that ends the leading '{', aka. end of this scope.
            Brace,

            // Skip until we see
            // 1) ';' in this scope
            // 2) '}' that ends this scope (N/A in the global scope)
            Semi,
        };

        // FIXME: implement correctly (initializer could also use braces, NOTE ';' in initializer isn't ending a
        // statement)
        auto RecoverFromError(RecoveryMode mode) -> void
        {
            // FIXME: is this a parser?
            TRACE_PARSER();

            auto desiredToken = static_cast<TokenKlass>(mode);

            size_t initParenDepth   = parenDepth;
            size_t initBracketDepth = bracketDepth;
            size_t initBraceDepth   = braceDepth;

            if (mode == RecoveryMode::Paren) {
                GLSLD_ASSERT(initParenDepth != 0);
            }
            if (mode == RecoveryMode::Bracket) {
                GLSLD_ASSERT(initBracketDepth != 0);
            }
            if (mode == RecoveryMode::Brace) {
                GLSLD_ASSERT(initBraceDepth != 0);
            }

            // We may close a pair of parenthsis without the closing delimitor
            auto removeDepthIfUnclosed = [&]() {
                switch (mode) {
                case RecoveryMode::Paren:
                    if (!TryTestToken(TokenKlass::RParen)) {
                        GLSLD_ASSERT(initParenDepth != 0);
                        parenDepth -= 1;
                    }
                    break;
                case RecoveryMode::Bracket:
                    if (!TryTestToken(TokenKlass::RBracket)) {
                        GLSLD_ASSERT(initBracketDepth != 0);
                        bracketDepth -= 1;
                    }
                    break;
                case RecoveryMode::Brace:
                    if (!TryTestToken(TokenKlass::RBrace)) {
                        GLSLD_ASSERT(initBraceDepth != 0);
                        braceDepth -= 1;
                    }
                    break;
                default:
                    break;
                }
            };

            while (PeekToken().klass != TokenKlass::Eof) {
                switch (PeekToken().klass) {
                case TokenKlass::RParen:
                    if (parenDepth == 0) {
                        // skip an isolated ')'
                        break;
                    }
                    if (mode == RecoveryMode::Paren && parenDepth == initParenDepth) {
                        return;
                    }
                    break;
                case TokenKlass::RBracket:
                    if (bracketDepth == 0) {
                        // skip an isolated ']'
                        break;
                    }
                    if (mode == RecoveryMode::Bracket && bracketDepth == initBracketDepth) {
                        return;
                    }
                    break;
                case TokenKlass::RBrace:
                    if (braceDepth == 0) {
                        // skip an isolated '}'
                        break;
                    }
                    if ((mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket || mode == RecoveryMode::Brace ||
                         mode == RecoveryMode::Semi) &&
                        braceDepth == initBraceDepth) {
                        return removeDepthIfUnclosed();
                    }
                    break;
                case TokenKlass::Semicolon:
                    if ((mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket || mode == RecoveryMode::Semi) &&
                        braceDepth == initBraceDepth) {
                        return removeDepthIfUnclosed();
                    }
                    break;
                default:
                    break;
                }

                // skip token?
                ConsumeToken();
            }
        }

        // FIXME: this is identical to decl recovery
        // Starting at an arbitrary location, consumes tokens until it might be a valid location to parse the next
        // declaration or statement.
        auto RecoverFromBadStmt() -> void
        {
            RecoverFromError(RecoveryMode::Semi);
            TryConsumeToken(TokenKlass::Semicolon);
        }

        // Starting at an arbitrary location, consumes tokens until it might be a valid location to parse the next
        // declaration or statement.
        auto RecoverFromBadDecl() -> void
        {
            RecoverFromError(RecoveryMode::Semi);
            TryConsumeToken(TokenKlass::Semicolon);
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

        // FIXME: need to restore brace depth tracker
        auto RestoreTokenIndex(size_t index) -> void
        {
            currentTokIndex = index;
            currentTok      = lexer->GetToken(currentTokIndex);
        }

        auto ConsumeToken() -> void
        {
            if (!Eof()) {
                TRACE_TOKEN_CONSUMED(PeekToken());

                switch (PeekToken().klass) {
                case TokenKlass::LParen:
                    parenDepth += 1;
                    break;
                case TokenKlass::LBracket:
                    bracketDepth += 1;
                    break;
                case TokenKlass::LBrace:
                    braceDepth += 1;
                    break;
                case TokenKlass::RParen:
                    if (parenDepth > 0) {
                        parenDepth -= 1;
                    }
                    break;
                case TokenKlass::RBracket:
                    if (bracketDepth > 0) {
                        bracketDepth -= 1;
                    }
                    break;
                case TokenKlass::RBrace:
                    if (braceDepth > 0) {
                        braceDepth -= 1;
                    }
                    break;
                default:
                    break;
                }

                RestoreTokenIndex(currentTokIndex + 1);
            }
        }

        auto ConsumeTokenAssert(TokenKlass klass) -> void
        {
            GLSLD_ASSERT(PeekToken().klass == klass);
            ConsumeToken();
        }

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

        auto CreateErrorExpr() -> AstErrorExpr*
        {
            auto tokIndex = GetTokenIndex();
            return CreateRangedAstNode<AstErrorExpr>(tokIndex, tokIndex);
        }

        auto CreateErrorStmt() -> AstErrorStmt*
        {
            auto tokIndex = GetTokenIndex();
            return CreateRangedAstNode<AstErrorStmt>(tokIndex, tokIndex);
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<AstNodeBase, T>
        auto CreateRangedAstNode(size_t beginTokIndex, size_t endTokIndex, Args&&... args) -> T*
        {
            return astCtx->CreateAstNode<T>(lexer->GetSyntaxRange(beginTokIndex, endTokIndex),
                                            std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<AstNodeBase, T>
        auto CreateAstNode(size_t beginTokIndex, Args&&... args) -> T*
        {
            return astCtx->CreateAstNode<T>(lexer->GetSyntaxRange(beginTokIndex, GetTokenIndex()),
                                            std::forward<Args>(args)...);
        }

    private:
        size_t parenDepth      = 0;
        size_t bracketDepth    = 0;
        size_t braceDepth      = 0;
        size_t scopeBraceDepth = 0;

        LexContext* lexer      = nullptr;
        size_t currentTokIndex = 0;
        SyntaxToken currentTok = {};

        AstContext* astCtx;
        DiagnosticContext* diagCtx;
    };

} // namespace glsld
