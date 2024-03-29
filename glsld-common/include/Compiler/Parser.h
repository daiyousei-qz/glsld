#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Compiler/AstBuilder.h"
#include "Compiler/LexContext.h"
#include "Compiler/CompilerObject.h"
#include "Compiler/CompilerTrace.h"
#include "Compiler/SyntaxToken.h"

#include "DiagnosticStream.h"

#include <vector>

namespace glsld
{
    // FIXME: update RECOVERY info
    // A parser function is named in `ParseXXX`. It should consume a sequence of tokens and returns a AST node
    // or a list of them. The parser functions in two parsing states:
    // - Parsing state: Parser is seeing well-formed program.
    // - Recovery state: Parser is recovering from a erroneous program.
    // When a parser is called from Parsing, it could either finish successfully, exiting with Parsing, or
    // fail gracefully, exiting with Recovery.
    // When a parser is called from Recovery, it should do nothing and return with Recovery. A parser shouldn't
    // exit Recovery state by itself. The caller is responsible of that.
    //
    // EXPECT: The expected next token. Calling this parser with incorrect next token leads to undefined behavior.
    // PARSE: Grammar rules that are going to be parsed.
    // ACCEPT: Additional grammar rules to be accepted (for permissive parsing).
    // RECOVERY: The next token to be expected if this parser entered recovery mode
    class Parser
    {
    private:
        CompilerObject& compilerObject;

        AstBuilder astBuilder;

        enum class ParsingState
        {
            // Parser is currently parsing
            Parsing,

            // Parser just recovered from an error and need restart parsing at some point
            Recovery,
        };

        ParsingState state = ParsingState::Parsing;

        // If we are parsing an initializer list. This flag is used to determine the outermost brace depth of nested
        // initializer list.
        bool parsingInitializerList = false;
        size_t ilistBraceDepth      = 0;

        size_t parenDepth   = 0;
        size_t bracketDepth = 0;
        size_t braceDepth   = 0;

        class ParsingBalancedParenGuard
        {
        private:
            Parser& parser;
            size_t leftParenDepth;

        public:
            ParsingBalancedParenGuard(Parser& parser) : parser(parser)
            {
                GLSLD_ASSERT(parser.TryTestToken(TokenKlass::LParen));
                parser.ConsumeToken();
                leftParenDepth = parser.parenDepth;
            }

            ~ParsingBalancedParenGuard()
            {
                parser.ParseClosingParenHelper(leftParenDepth);
            }

            ParsingBalancedParenGuard(const ParsingBalancedParenGuard&)            = delete;
            ParsingBalancedParenGuard& operator=(const ParsingBalancedParenGuard&) = delete;

            auto GetLeftParenDepth() const noexcept -> size_t
            {
                return leftParenDepth;
            }
        };

        SyntaxToken currentTok = {};

    public:
        Parser(CompilerObject& compilerObject) : compilerObject(compilerObject), astBuilder(compilerObject)
        {
        }

        auto DoParse() -> void;

    private:
        // Parse the tokens in the token stream from the LexContext and register AST nodes into the AstContext.
        // This function should be called only once. After this function returns, this tokenizer object should no longer
        // be used.
        auto ParseTranslationUnit() -> const AstTranslationUnit*;

#pragma region Parsing Misc

        //
        // These are not registered parsers, but boilerplate functions
        //

        // Parse a ';' if available. Otherwise, we issue an error and return, which effectively infers
        // the required ';'
        //
        // PARSE: ';'
        //
        // ACCEPT: null
        auto ParseOrInferSemicolonHelper() -> void;

        // Parse a ',' inside a parenthesis pair. Returns true if a ',' is parsed, meaning we should continue parsing
        // the next item.
        //
        // PARSE: ','
        //
        // ACCEPT: ??? ',' or ??? ^')'
        auto ParseCommaSeperatorHelper(size_t leftParenDepth) -> bool;

        // Parse a closing ')' with the assumption that a balanced '(' has been parsed.
        //
        // PARSE: ')'
        //
        // ACCEPT: ??? ')' or null
        //
        // RECOVERY: ^'EOF' or '}' or ';'
        auto ParseClosingParenHelper(size_t leftParenDepth) -> void;

        // Parse a closing ']' with the assumption that a balanced '[' has been parsed.
        //
        // PARSE: ']'
        //
        // ACCEPT: ??? ']' or null
        //
        // RECOVERY: ^'EOF' or '}' or ';'
        auto ParseClosingBracketHelper(size_t leftBracketDepth) -> void;

        // Try to parse an identifier token as a symbol name if available, otherwise returns an invalid token.
        //
        // PARSE: 'ID'
        //
        // ACCEPT: null
        auto ParseDeclIdHelper() -> SyntaxToken;

        // Try to parse an expression wrapped in parenthesis. If we don't see a '(', enter recovery mode and return
        // error expr.
        //
        // PARSE: paren_wrapped_expr
        //
        // ACCEPT: null
        //
        // RECOVERY: unknown
        auto ParseParenWrappedExprOrErrorHelper() -> AstExpr*;
#pragma endregion

#pragma region Parsing QualType

        // Parse a layout specifier and save layout items into the parameter vector.
        //
        // EXPECT: 'K_layout'
        //
        // PARSE: layout_qual
        //      - layout_qual := 'layout' '(' ')'
        //      - layout_qual := 'layout' '(' layout_spec [',' layout_spec]... ')'
        //      - layout_spec := 'ID'
        //      - layout_spec := 'ID' '=' assignment_expr
        //
        // ACCEPT: 'K_layout' '(' ??? ')'
        //
        // RECOVERY: unknown
        auto ParseLayoutQualifier(std::vector<LayoutItem>& items) -> void;

        // Try to parse a sequence of qualifiers. Returns nullptr if no qualifier is parsed.
        //
        // PARSE: qual_seq
        //      - qual_seq := [qualifier]...
        //      - qualifier := layout_qual
        //      - qualifier := 'K_???'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseTypeQualifierSeq() -> AstTypeQualifierSeq*;

        // EXPECT: '{'
        // PARSE: struct_body
        //      - struct_body := '{' member_decl... '}'
        //      - member_decl := ?
        //
        // ACCEPT: '{' ??? '}'
        //
        // RECOVERY: ^'EOF' or ^';'
        // FIXME: member_decl???
        auto ParseStructBody() -> std::vector<AstFieldDecl*>;

        // EXPECT: K_struct
        //
        // PARSE: struct_definition
        //      - struct_definition := 'struct' ['ID'] struct_body
        //
        // RECOVERY:
        auto ParseStructDefinition() -> AstStructDecl*;

        // EXPECT: '['
        //
        // PARSE: array_spec
        //      - array_spec := ('[' [expr] ']')...
        //
        // RECOVERY: ^'EOF' or ^'}' or ^';'
        auto ParseArraySpec() -> AstArraySpec*;

        // Parse a qualified type specifier, which could be either:
        // 1. a struct definition
        // 2. an identifier that's a type name
        // 3. a keyword that's a built-in type name
        // NOTE qualifier is previously parsed and passed in as a parameter.
        //
        // PARSE: type_spec
        //      - type_spec := struct_definition
        //      - type_spec := 'ID'
        //      - type_spec := 'K_???'
        //
        // RECOVERY: unknown
        auto ParseTypeSpec(AstTypeQualifierSeq* quals) -> AstQualType*;

        // Parse a qualfied type specifier.
        //
        // PARSE: qualified_type_spec
        //      - qualified_type_spec := [qual_seq] type_spec
        //
        // RECOVERY: unknown
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
        // - precision settings
        //   - precision precision-qual type;
        // - other settings
        //   - type-qual;
        //   - type-qual id;
        //   - type-qual id, ...;
        // - block {in/out/uniform/buffer}
        //   - qualifiers id { ... };
        //   - qualifiers id { ... } id;
        //   - qualifiers id { ... } id[N];
        auto ParseDeclaration() -> AstDecl*;

        // - global (in/out/uniform)
        //   - type-qual? type-spec;
        //   - type-qual? type-spec id = init;
        //   - type-qual? type-spec id[N] = init;
        //   - type-qual? type-spec id = init, ...;
        // - struct
        //   - struct id? { ... };
        //   - struct id? { ... } id;
        //   - struct id? { ... } id[N];
        // - function
        //   - type-qual? type-spec id(...);
        //   - type-qual? type-spec id(...) { ... }
        auto ParseDeclarationWithTypeSpec(AstQualType* type) -> AstDecl*;

        // PARSE: decl
        //
        // RECOVERY: ^'EOF'
        auto ParseDeclAndTryRecover() -> AstDecl*;

        // Parse an initializer which is either an initializer list or an assignment expression.
        //
        // PARSE: initializer
        //      - initializer := assignment_expr
        //      - initializer := initializer_list
        //
        // FIXME: recovery?
        auto ParseInitializer() -> AstInitializer*;

        // PARSE: initializer_list
        //      - initializer_list := '{' initializer [',' initializer]... '}'
        auto ParseInitializerList() -> AstInitializerList*;

        // EXPECT: 'ID'
        // PARSE: declarator
        //      - declarator := 'ID' [array_spec]
        //      - declarator := 'ID' [array_spec] [ '=' initializer ]
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclarator() -> Declarator;

        // EXPECT: 'ID'
        // PARSE: declarator_list
        //      - declarator_list := declarator [',' declarator]...
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclaratorList() -> std::vector<Declarator>;

        // EXPECT: '('
        //
        // PARSE: func_param_list
        //      - func_param_list := '(' ')'
        //      - func_param_list := '(' 'K_void' ')'
        //      - func_param_list := '(' func_param_decl [',' func_param_decl]...  ')'
        //      - func_param_decl := type-qual? type_spec 'ID' [array_sped]
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
        // RECOVERY: unknown
        auto ParseExpr() -> AstExpr*;

        // Parse an expression without comma operator. If no token is consumed, enter recovery mode to advance parsing.
        //
        // PARSE: assignment_expr
        //
        // RECOVERY: unknown
        auto ParseExprNoComma() -> AstExpr*;

        // Parse a comma expression.
        //
        // PARSE: expr
        //      - expr := assignment_expr [',' assignment_expr]...
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
        //      - unary_expr := ['unary_op']... postfix_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseUnaryExpr() -> AstExpr*;

        // Parse an postfix expression.
        //
        // PARSE: postfix_expr
        //      - postfix_expr := primary_expr
        //      - postfix_expr := function_call
        //      - postfix_expr := constructor_call
        //      - postfix_expr := postfix_expr 'incdec'
        //      - postfix_expr := postfix_expr '.' 'ID'
        //      - postfix_expr := postfix_expr array_spec
        //      - function_call := postfix_expr func_arg_list
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParsePostfixExpr() -> AstExpr*;

        // Parse an constructor call expression.
        //
        // EXPECT: 'K_struct' or 'K_???' or 'ID' (aka. type_spec)
        //
        // PARSE: constructor_call
        //      - constructor_call := type_spec func_arg_list
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseConstructorCallExpr() -> AstExpr*;

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

        // Parse an expression wrapped in parentheses pair.
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
        auto ParseFunctionArgumentList() -> std::vector<AstExpr*>;

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
        // RECOVERY: unknown
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
        //
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
        // RECOVERY: unknown
        auto ParseDoWhileStmt() -> AstStmt*;

        // EXPECT: 'K_while'
        //
        // PARSE: while_stmt
        //      - while_stmt := 'K_while' paren_wrapped_expr stmt
        //
        // RECOVERY: unknown
        auto ParseWhileStmt() -> AstStmt*;

        // EXPECT: 'K_case' or 'K_default'
        //
        // PARSE: label_stmt
        //      - label_stmt := 'K_case' expr [':']
        //      - label_stmt := 'K_default' [':']
        //
        // RECOVERY: unknown
        auto ParseLabelStmt() -> AstStmt*;

        // EXPECT: 'K_switch'
        //
        // PARSE: switch_stmt
        //      - switch_stmt := 'K_switch' paren_wrapped_expr switch_body
        //      - switch_body := '{' ??? '}'
        //
        // RECOVERY: unknown
        auto ParseSwitchStmt() -> AstStmt*;

        // Parse jump statement.
        //
        // EXPECT: 'K_break' or 'K_continue' or 'K_discard' or 'K_return'
        //
        // PARSE: jump_stmt
        //      - jump_stmt := 'K_break' [';']
        //      - jump_stmt := 'K_continue' [';']
        //      - jump_stmt := 'K_discard' [';']
        //      - jump_stmt := 'K_return' [expr] [';']
        //
        // RECOVERY: unknown
        auto ParseJumpStmt() -> AstStmt*;

        // PARSE: expr_stmt
        //      - expr_stmt := expr ';'
        //
        // ACCEPT: expr
        //
        // RECOVERY: unknown
        auto ParseExprStmt() -> AstStmt*;

        // PARSE: decl_stmt
        //      - decl_stmt := declaration
        //
        // RECOVERY: unknown
        auto ParseDeclStmt() -> AstStmt*;

        // PARSE: decl_or_expr_stmt
        //      - decl_or_expr_stmt := expr_stmt
        //      - decl_or_expr_stmt := declaration
        //
        // RECOVERY: unknown
        // FIXME: what's in recovery mode?
        auto ParseDeclOrExprStmt() -> AstStmt*;

#pragma endregion

        enum class RecoveryMode
        {
            // Assuming a preceding '(', skip until we see:
            // 1. ')' that ends the leading '('
            // 2. ',' that in exactly this level of parenthesis
            // 2. ';' in this scope
            // 3. '}' that ends this scope (N/A in the global scope)
            // 4. 'EOF'
            Comma,

            // Assuming a preceding '(', skip until we see:
            // 1. ')' that ends the leading '('
            // 2. ';' in this scope
            // 3. '}' that ends this scope (N/A in the global scope)
            // 4. 'EOF'
            Paren,

            // Assuming a preceding '[', skip until we see:
            // 1. ']' that ends the leading '['
            // 2. ';' in this scope
            // 3. '}' that ends this scope (N/A in the global scope)
            // 4. 'EOF'
            Bracket,

            // Assuming a preceding '{', skip until we see:
            // 1. '}' that ends the leading '{'
            // 2. 'EOF'
            Brace,

            // Assuming a preceding '{', skip until we see:
            // 1. '}' that ends the leading '{'
            // 2. ';' in this scope
            // 2. 'EOF'
            IListBrace,

            // Assuming NO preceding '(' and '[', skip until we see:
            // 1. ';' in this scope
            // 2. '}' that ends this scope (N/A in the global scope)
            // 3. 'EOF'
            Semi,
        };

        auto InParsingMode() const noexcept -> bool
        {
            return state == ParsingState::Parsing;
        }

        auto InRecoveryMode() const noexcept -> bool
        {
            return state == ParsingState::Recovery;
        }

        auto EnterRecoveryMode() noexcept -> void
        {
            state = ParsingState::Recovery;
        }

        auto ExitRecoveryMode() noexcept -> void
        {
            GLSLD_ASSERT(state == ParsingState::Recovery);
            state = ParsingState::Parsing;
        }

        // Recover from a parsing error by skipping tokens until we reach a recovery point.
        auto RecoverFromError(RecoveryMode mode) -> void;

        auto PeekToken() const noexcept -> const SyntaxToken&
        {
            return currentTok;
        }

        auto PeekToken(size_t lookahead) const noexcept -> SyntaxToken
        {
            return compilerObject.GetLexContext().GetTUTokenSafe(currentTok.index + lookahead);
        }

        auto GetTokenIndex() const noexcept -> SyntaxTokenIndex
        {
            return currentTok.index;
        }

        // FIXME: need to restore brace depth tracker
        auto RestoreTokenIndex(size_t index) -> void
        {
            currentTok = compilerObject.GetLexContext().GetTUToken(index);
        }

        auto TryTestToken(TokenKlass klass) -> bool
        {
            return PeekToken().klass == klass;
        }

        auto TryTestToken(TokenKlass klass1, TokenKlass klass2) -> bool
        {
            return PeekToken().klass == klass1 || PeekToken().klass == klass2;
        }

        auto TryTestToken(TokenKlass klass1, TokenKlass klass2, TokenKlass klass3) -> bool
        {
            return PeekToken().klass == klass1 || PeekToken().klass == klass2 || PeekToken().klass == klass3;
        }

        auto TryTestToken(TokenKlass klass, int lookahead) -> bool
        {
            const auto& tok = PeekToken(lookahead);
            return tok.klass == klass;
        }
        auto TryTestToken(TokenKlass klass1, TokenKlass klass2, int lookahead) -> bool
        {
            const auto& tok = PeekToken(lookahead);
            return tok.klass == klass1 || tok.klass == klass2;
        }
        auto TryTestToken(TokenKlass klass1, TokenKlass klass2, TokenKlass klass3, int lookahead) -> bool
        {
            const auto& tok = PeekToken(lookahead);
            return tok.klass == klass1 || tok.klass == klass2 || tok.klass == klass3;
        }

        auto Eof() -> bool
        {
            return TryTestToken(TokenKlass::Eof);
        }

        auto ConsumeToken() -> void;

        auto TryConsumeToken(TokenKlass klass) -> bool
        {
            if (TryTestToken(klass)) {
                ConsumeToken();
                return true;
            }
            else {
                return false;
            }
        }

        auto CreateErrorExpr() -> AstErrorExpr*
        {
            return astBuilder.BuildErrorExpr(CreateAstSyntaxRange());
        }

        auto CreateErrorExpr(SyntaxTokenIndex beginTokIndex) -> AstErrorExpr*
        {
            return astBuilder.BuildErrorExpr(CreateAstSyntaxRange(beginTokIndex));
        }

        auto CreateErrorStmt() -> AstErrorStmt*
        {
            auto tokIndex = GetTokenIndex();
            return astBuilder.BuildErrorStmt(CreateAstSyntaxRange());
        }

        auto CreateAstSyntaxRange() -> AstSyntaxRange
        {
            return AstSyntaxRange{GetTokenIndex()};
        }
        auto CreateAstSyntaxRange(SyntaxTokenIndex beginTokIndex) -> AstSyntaxRange
        {
            return AstSyntaxRange{beginTokIndex, GetTokenIndex()};
        }

        // Report an error at the given token.
        auto ReportError(SyntaxTokenIndex tokIndex, std::string message) -> void
        {
            compilerObject.GetDiagnosticStream().ReportError(AstSyntaxRange{tokIndex}, std::move(message));
        }

        // Report an error at the current token.
        auto ReportError(std::string message) -> void
        {
            ReportError(GetTokenIndex(), std::move(message));
        }
    };

} // namespace glsld
