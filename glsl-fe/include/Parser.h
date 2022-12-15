#pragma once
#include "DiagnosticContext.h"
#include "LexContext.h"
#include "AstContext.h"
#include "Tokenizer.h"
#include "Typing.h"
#include "ParserTrace.h"
#include "AstVisitor.h"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace glsld
{
    // FIXME: update RECOVERY info
    class Parser
    {
    public:
        Parser(LexContext* lexCtx, AstContext* astCtx, DiagnosticContext* diagCtx)
            : lexContext(lexCtx), astContext(astCtx), diagContext(diagCtx)
        {
            RestoreTokenIndex(0);
        }

        auto DoParseTranslationUnit() -> void
        {
            TRACE_PARSER();

            while (!Eof()) {
                astContext->AddGlobalDecl(ParseDeclAndTryRecover());
            }
        }

#pragma region Parsing Misc

        //
        // These are not parsers, but boilerplate functions
        //

        // In parsing mode, consume a ';' if available. Otherwise we issue an error and return, whici is used to
        // infer a required ';'.
        //
        // In recovery mode, we do nothing and leave the ';' for recovery.
        //
        // PARSE: ';'
        //
        // ACCEPT: null
        auto ParsePermissiveSemicolon() -> void;

        // PARSE: ')'
        //
        // RECOVERY: ^'EOF' or '}' or ';'
        auto ParseClosingParen() -> void;

        // PARSE: ']'
        //
        // RECOVERY: ^'EOF' or '}' or ';'
        auto ParseClosingBracket() -> void;

        // Try to parse an identifier token as a symbol name if available, otherwise returns an error token
        //
        // PARSE: 'ID'
        //
        // ACCEPT: null
        auto ParseDeclId() -> SyntaxToken;

        // Try to parse an expression wrapped in parenthesis. If we don't see a '(', enter revery mode and return error
        // expr.
        //
        // PARSE: paren_wrapped_expr
        //
        // ACCEPT: null
        //
        // RECOVERY: ?
        auto ParseParenWrappedExprOrError() -> AstExpr*;

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
        auto ParseLayoutQualifier() -> std::vector<LayoutItem>;

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
        auto ParseStructBody() -> std::vector<AstStructMemberDecl*>;

        // EXPECT: K_struct
        //
        // PARSE: struct_definition
        //      - struct_definition := 'struct' ['ID'] struct_body
        //
        // RECOVERY:
        auto ParseStructDefinition() -> AstStructDecl*;

        // EXPECT: '['
        // PARSE: array_spec
        //      - array_spec := ('[' [expr] ']')...
        //
        // RECOVERY: ^'EOF' or ^'}' or ^';'
        auto ParseArraySpec() -> AstArraySpec*;

        // PARSE: type_spec
        //      - type_spec := struct_definition
        //      - type_spec := 'ID'
        //      - type_spec := 'K_???'
        //
        // RECOVERY:
        auto ParseType(AstTypeQualifierSeq* quals) -> AstQualType*;

        // PARSE: qualified_type_spec
        //        qualified_type_spec := [qual_seq] type_spec
        auto ParseQualType() -> AstQualType*;

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
        auto ParseDeclaration() -> AstDecl*;

        // PARSE: decl
        //
        // RECOVERY: ^'EOF'
        auto ParseDeclAndTryRecover() -> AstDecl*;

        // Parse an initializer list.
        //
        // PARSE: initializer
        //      - initializer := assignment_expr
        //      - initializer := '{' initializer [',' initializer]... '}'
        //
        // FIXME: recovery?
        auto ParseInitializer() -> AstNodeBase*;

        // EXPECT: 'ID'
        // PARSE: declarator
        //      - declarator := 'ID' [array_spec]
        //      - declarator := 'ID' [array_spec] [ '=' initializer ]
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclarator() -> VariableDeclarator;

        // EXPECT: 'ID'
        // PARSE: declarator_list
        //      - declarator_list := declarator [',' declarator]...
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclaratorList() -> std::vector<VariableDeclarator>;

        // EXPECT: '('
        //
        // PARSE: func_param_list
        //      - func_param_list := '(' ')'
        //      - func_param_list := '(' 'K_void' ')'
        //      - func_param_list := '(' func_param_decl [',' func_param_decl]...  ')'
        //      - func_param_decl := qualified_type_spec 'ID' [array_sped]
        //
        // RECOVERY: ^'EOF' or ^';'
        // WHAT'S AN ERROR SECTION?
        auto ParseFunctionParamList() -> std::vector<AstParamDecl*>;

        // EXPECT: 'ID' or '('
        //
        // PARSE: func_decl
        //      - func_decl := 'ID' func_param_list ';'
        //      - func_decl := 'ID' func_param_list compound_stmt
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> AstDecl*;

        // EXPECT: ';' or 'ID'
        //
        // PARSE: type_or_variable_decl
        //      - type_or_variable_decl := ';'
        //      - type_or_variable_decl := declarator [',' declarator]... ';'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseTypeOrVariableDecl(size_t beginTokIndex, AstQualType* variableType) -> AstDecl*;

        // EXPECT: 'ID' '{' or '{'
        //
        // PARSE: interface_block
        //      - interface_block := 'ID' '{' [declartion]... '}' [declarator] ';'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> AstDecl*;

        // EXPECT: 'K_precision'
        // PARSE: precision_decl
        //      - precision_decl := ???
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParsePrecisionDecl() -> AstDecl*;

#pragma endregion

#pragma region Parsing Expr

        // Parse an expression. If no token is consumed, enter recovery mode to advance parsing.
        //
        // PARSE: expr
        //
        // RECOVERY: ?
        auto ParseExpr() -> AstExpr*;

        // Parse an expression without comma operator. If no token is consumed, enter recovery mode to advance parsing.
        //
        // PARSE: assignment_expr
        //
        // RECOVERY: ?
        auto ParseExprNoComma() -> AstExpr*;

        // Parse a comma expression.
        //
        // PARSE: expr
        //        expr := assignment_expr [',' assignment_expr]...
        //
        // RECOVERY: ^';' or ^'}' or ^'EOF'
        auto ParseCommaExpr() -> AstExpr*;

        // Parse a top-level expreesion without comma operator.
        //
        // PARSE: assignment_expr
        //      - assignment_expr := unary_expr '?=' assignment_expr
        //      - assignment_expr := binary_or_conditional_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseAssignmentExpr() -> AstExpr*;

        // PARSE: binary_or_conditional_expr
        //      - binary_or_conditional_expr := binary_expr
        //      - binary_or_conditional_expr := binary_expr '?' expr ':' assignment_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        //
        // `firstTerm` is the first terminal in the condition
        auto ParseBinaryOrConditionalExpr(size_t beginTokIndex, AstExpr* firstTerm) -> AstExpr*;

        // PARSE: binary_expr
        //      - binary_expr := unary_expr
        //      - binary_expr := binary_expr 'binary_op' binary_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        //
        // `firstTerm` for this is a unary expression, which might already be parsed
        auto ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*;

        // Parse an unary expression.
        //
        // PARSE: unary_expr
        //        unary_expr := ['unary_op']... postfix_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseUnaryExpr() -> AstExpr*;

        // Parse an postfix expression.
        //
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
        auto ParsePostfixExpr() -> AstExpr*;

        // Parse an primary expression.
        //
        // PARSE: primary_expr
        //      - primary_expr := 'ID'
        //      - primary_expr := 'constant'
        //      - primary_expr := paren_wrapped_expr
        //
        // ACCEPT: null
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParsePrimaryExpr() -> AstExpr*;

        // Parse an expression wrapped in parenthesis.
        //
        // EXPECT: '('
        //
        // PARSE: paren_wrapped_expr
        //      - paren_wrapped_expr := '(' expr ')'
        //
        // ACCEPT: '(' ??? ')'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseParenWrappedExpr() -> AstExpr*;

        // Parse a function argument list.
        //
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
        auto ParseFunctionArgumentList() -> std::vector<AstExpr*>
        {
            TRACE_PARSER();
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
                if (InRecoveryMode()) {
                    break;
                }

                result.push_back(arg);
                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }

            ParseClosingParen();
            return result;
        }

#pragma endregion

#pragma region Parsing Stmt

        // PARSE: stmt
        //      - stmt := compound_stmt
        //      - stmt := selection_stmt
        //      - stmt := for_stmt
        //      - stmt := dowhile_stmt
        //      - stmt := while_stmt
        //      - stmt := switch_stmt
        //      - stmt := jump_stmt
        //      - stmt := expr_stmt
        //      - stmt := declaration
        //
        // RECOVERY: ?
        auto ParseStmt() -> AstStmt*;

        // PARSE: stmt
        //
        // RECOVERY: ^'EOF'
        auto ParseStmtAndTryRecover() -> AstStmt*;

        // EXPECT: '{'
        //
        // PARSE: compound_stmt
        //      - compound_stmt := '{' [stmt]... '}'
        //
        // ACCEPT: '{' ??? '}'
        //
        // RECOVERY: ^'EOF'
        auto ParseCompoundStmt() -> AstStmt*;

        // EXPECT: 'K_if'
        //
        // PARSE: selection_stmt
        //      - selection_stmt := 'K_if' paren_wrapped_expr stmt
        //      - selection_stmt := 'K_if' paren_wrapped_expr stmt 'K_else' selection_stmt
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        // FIXME: check with if-else nesting
        auto ParseSelectionStmt() -> AstStmt*;

        // EXPECT: 'K_for'
        //
        // PARSE: for_stmt
        //      - for_stmt := 'K_for' for_pred stmt
        //      - for_pred := '(' stmt stmt stmt ')'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        // FIXME: only simple stmt is allowed in for loop predicate?
        auto ParseForStmt() -> AstStmt*;

        // EXPECT: 'K_do'
        //
        // PARSE: dowhile_stmt
        //      - dowhile_stmt := 'K_do' stmt 'K_while' paren_wrapped_expr ';'
        //
        // RECOVERY: ?
        auto ParseDoWhileStmt() -> AstStmt*;

        // EXPECT: 'K_while'
        //
        // PARSE: while_stmt
        //      - while_stmt := 'K_while' paren_wrapped_expr stmt
        //
        // RECOVERY: ?
        auto ParseWhileStmt() -> AstStmt*;

        // EXPECT: 'K_case' or 'K_default'
        //
        // PARSE: label_stmt
        //      - label_stmt := 'K_case' expr [':']
        //      - label_stmt := 'K_default' [':']
        //
        // RECOVERY: ?
        auto ParseLabelStmt() -> AstStmt*;

        // EXPECT: 'K_switch'
        //
        // PARSE: switch_stmt
        //      - switch_stmt := 'K_switch' paren_wrapped_expr switch_body
        //      - switch_body := '{' ??? '}'
        //
        // RECOVERY: ?
        auto ParseSwitchStmt() -> AstStmt*;

        // EXPECT: 'K_break' or 'K_continue' or 'K_discard' or 'K_return'
        //
        // PARSE: jump_stmt
        //      - jump_stmt := 'K_break' [';']
        //      - jump_stmt := 'K_continue' [';']
        //      - jump_stmt := 'K_discard' [';']
        //      - jump_stmt := 'K_return' [expr] [';']
        //
        // RECOVERY: ?
        auto ParseJumpStmt() -> AstStmt*;

        // PARSE: expr_stmt
        //      - expr_stmt := expr [';']
        //
        // RECOVERY: ?
        auto ParseExprStmt() -> AstStmt*;

        // PARSE: decl_or_expr_stmt
        //      - decl_or_expr_stmt := expr_stmt
        //      - decl_or_expr_stmt := declaration
        //
        // RECOVERY: ?
        // FIXME: what's in recovery mode?
        auto ParseDeclOrExprStmt() -> AstStmt*;

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

        auto Eof() -> bool
        {
            return TryTestToken(TokenKlass::Eof);
        }

        auto InParsingMode() -> bool
        {
            return state == ParsingState::Parsing;
        }

        auto InRecoveryMode() -> bool
        {
            return state == ParsingState::Recovery;
        }

        auto EnterRecoveryMode() -> void
        {
            state = ParsingState::Recovery;
        }

        auto ExitRecoveryMode() -> void
        {
            GLSLD_ASSERT(state == ParsingState::Recovery);
            state = ParsingState::Parsing;
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

        auto PeekToken() -> const SyntaxToken&
        {
            return currentTok;
        }

        auto PeekToken(size_t lookahead) -> SyntaxToken
        {
            return lexContext->GetToken(currentTok.index + lookahead);
        }

        auto GetTokenIndex() -> SyntaxTokenIndex
        {
            return currentTok.index;
        }

        // FIXME: need to restore brace depth tracker
        auto RestoreTokenIndex(size_t index) -> void
        {
            currentTok = lexContext->GetToken(index);
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

                RestoreTokenIndex(currentTok.index + 1);
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
        auto CreateRangedAstNode(SyntaxTokenIndex beginTokIndex, SyntaxTokenIndex endTokIndex, Args&&... args) -> T*
        {
            return astContext->CreateAstNode<T>(SyntaxTokenRange{beginTokIndex, endTokIndex},
                                                std::forward<Args>(args)...);
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<AstNodeBase, T>
        auto CreateAstNode(SyntaxTokenIndex beginTokIndex, Args&&... args) -> T*
        {
            return astContext->CreateAstNode<T>(SyntaxTokenRange{beginTokIndex, GetTokenIndex()},
                                                std::forward<Args>(args)...);
        }

        auto ReportError(SyntaxTokenIndex tokIndex, std::string message) -> void
        {
            // FIXME: report error
            // auto synRange = lexContext->GetSyntaxRange(tokIndex);
            // auto beginLoc = lexContext->LookupSyntaxLocation(synRange.begin);
            // diagContext->ReportError(DiagnosticLocation{beginLoc.line, beginLoc.column}, std::move(message));
        }
        auto ReportError(std::string message) -> void
        {
            // ReportError(GetTokenIndex(), std::move(message));
        }

    private:
        enum class ParsingState
        {
            // Parser is parsing
            Parsing,

            // Parser just recovered from an error and need restart parsing at some point
            Recovery,
        };

        ParsingState state = ParsingState::Parsing;

        size_t parenDepth      = 0;
        size_t bracketDepth    = 0;
        size_t braceDepth      = 0;
        size_t scopeBraceDepth = 0;

        LexContext* lexContext = nullptr;
        SyntaxToken currentTok = {};

        AstContext* astContext;
        DiagnosticContext* diagContext;
    };

} // namespace glsld
