#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Basic/Common.h"
#include "Compiler/AstBuilder.h"
#include "Compiler/CompilerInvocationState.h"
#include "Compiler/SyntaxToken.h"
#include "Compiler/DiagnosticStream.h"

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
    // RECOVERY: If specified, this parser would do backtracking and parse additional error rules
    class Parser
    {
    private:
        CompilerInvocationState& compiler;

        DiagnosticReportor diagReporter;

        AstBuilder astBuilder;

        TranslationUnitID tuID;

        ArrayView<RawSyntaxTokenEntry> tokens;

        const RawSyntaxTokenEntry* currentTok = nullptr;

        enum class ParsingState
        {
            // Parser is currently parsing
            Parsing,

            // Parser just recovered from an error and need restart parsing at some point
            Recovery,
        };

        ParsingState state = ParsingState::Parsing;

        struct SystemSettings
        {
            // True if __glsld_syscmd_require_version__ is unsatisfied.
            bool requiredVersionDisabled = false;

            // True if __glsld_syscmd_require_stage__ is unsatisfied.
            bool requiredStageDisabled = false;

            // True if __glsld_syscmd_require_extension__ is unsatisfied.
            bool requiredExtensionDisabled = false;
        };

        std::optional<SystemSettings> systemSettings;

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

    public:
        Parser(CompilerInvocationState& compiler, const LexedTranslationUnit& tu)
            : compiler(compiler), astBuilder(compiler), diagReporter(compiler.GetDiagnosticStream()), tuID(tu.GetID()),
              tokens(tu.GetTokens())
        {
            currentTok = tokens.data();
        }

        auto DoParse() -> void;

    private:
        auto HandleSystemCommand(StringView cmd, ArrayView<SyntaxToken> args) -> void;

        // Parse a sequence of system commands in the global scope.
        // System commands are `__glsld_syscmd_XXX__ <args>... ;` in the system preamble, which are used to
        // configure the special compiler behavior for the standard library.
        auto ParseSystemCommands() -> void;

        // Parse the tokens in the token stream and register AST nodes into the CompilerInvocationState.
        // This function should be called only once. After this function returns, this tokenizer object should no longer
        // be used.
        //
        // PARSE: translation_unit
        //      - translation_unit := declaration...
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

        // Parse a ',' inside a parenthesis pair. Returns true if a ',' is successfully parsed.
        //
        // PARSE: ','
        //
        // RECOVERY: comma_recovery
        auto ParseCommaInParenHelper(size_t leftParenDepth) -> bool;

        // Parse a closing ')' with the assumption that a balanced '(' has been parsed.
        //
        // PARSE: ')'
        //
        // RECOVERY: paren_recovery ')'
        auto ParseClosingParenHelper(size_t leftParenDepth) -> void;

        // Parse a closing ']' with the assumption that a balanced '[' has been parsed.
        //
        // PARSE: ']'
        //
        // RECOVERY: bracket_recovery ']'
        auto ParseClosingBracketHelper(size_t leftBracketDepth) -> void;

        // Try to parse an identifier token as a symbol name if available, otherwise returns an invalid token.
        //
        // PARSE: 'ID'
        //
        // ACCEPT: null
        auto ParseOptionalDeclIdHelper() -> SyntaxToken;

        auto ParseDeclIdHelper() -> SyntaxToken;

        // Try to parse an expression wrapped in parenthesis. If we don't see a '(', enter recovery mode and return
        // error expr.
        //
        // PARSE: paren_wrapped_expr
        //
        // ACCEPT: null
        auto ParseParenWrappedExprOrErrorHelper() -> AstExpr*;
#pragma endregion

#pragma region Parsing QualType

        // Parse a layout specifier and save layout items into the parameter vector.
        //
        // EXPECT: 'K_layout'
        //
        // PARSE: layout_qual
        //      - layout_qual := 'K_layout' '(' ')'
        //      - layout_qual := 'K_layout' '(' layout_spec [',' layout_spec]... ')'
        //      - layout_spec := 'ID'
        //      - layout_spec := 'ID' '=' assignment_expr
        //
        // ACCEPT:
        //       - 'K_layout'
        //       - 'K_layout' '(' paren_recovery ')'
        auto ParseLayoutQualifier(std::vector<LayoutItem>& items) -> void;

        // Try to parse a sequence of qualifiers. Returns nullptr if no qualifier is parsed.
        //
        // PARSE: qual_seq
        //      - qual_seq := [qualifier]...
        //      - qualifier := layout_qual
        //      - qualifier := 'K_???'
        auto ParseTypeQualifierSeq() -> AstTypeQualifierSeq*;

        // EXPECT: '{'
        //
        // PARSE: struct_body
        //      - struct_body := '{' member_decl... '}'
        //      - member_decl := ?
        //
        // ACCEPT: '{' ??? '}'
        //
        // RECOVERY: ^'EOF' or ^';'
        // FIXME: member_decl???
        auto ParseStructBody() -> std::vector<AstFieldDecl*>;

        // EXPECT: 'K_struct'
        //
        // PARSE: struct_definition
        //      - struct_definition := 'K_struct' ['ID'] struct_body
        //
        // ACCEPT:
        //      - 'K_struct'
        //      - 'K_struct' ['ID']
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

        // PARSE: declaration
        //
        // RECOVERY: ^'EOF'
        auto ParseDeclAndTryRecover(AstQualType* typeSpec, bool atGlobalScope) -> AstDecl*;

        // PARSE: stmt
        //
        // RECOVERY: ^'EOF'
        auto ParseStmtAndTryRecover() -> AstStmt*;

#pragma endregion

#pragma region Parsing Decl

        // Parse a declaration. For disambiguation, the type specifier may has already been parsed.
        //
        // PARSE: declaration
        //      - declaration := ';'
        //      - declaration := precision_decl
        //      - declaration := qual_seq ';' (TODO)
        //      - declaration := qual_seq id [ ',' id ]... ';' (TODO)
        //      - declaration := qual_seq interface_block_decl
        //      - declaration := qual_seq? type_spec func_decl
        //      - declaration := qual_seq? type_spec type_or_variable_decl
        //
        // ACCEPT: ??? ';'
        //
        // RECOVERY: ^'EOF'
        auto ParseDeclaration(bool atGlobalScope) -> AstDecl*;

        // We have already parsed a fully qualified type specifier. Now we need to parse the rest of the declaration.
        //
        // PARSE: declaration_with_type_spec
        //      - declaration_with_type_spec := func_decl
        //      - declaration_with_type_spec := type_or_variable_decl
        //      - declaration_with_type_spec := ';'
        //
        // RECOVERY: ^'EOF'
        auto ParseDeclarationWithTypeSpec(AstQualType* type) -> AstDecl*;

        // Parse an initializer which is either an initializer list or an assignment expression.
        //
        // PARSE: initializer
        //      - initializer := assignment_expr
        //      - initializer := initializer_list
        //
        // FIXME: recovery?
        auto ParseInitializer(const Type* type) -> AstInitializer*;

        // Parse an initializer list.
        //
        // PARSE: initializer_list
        //      - initializer_list := '{' initializer [',' initializer]... '}'
        auto ParseInitializerList(const Type* type) -> AstInitializerList*;

        // Parse a declarator.
        //
        // EXPECT: 'ID'
        // PARSE: declarator
        //      - declarator := 'ID' [array_spec]
        //      - declarator := 'ID' [array_spec] [ '=' initializer ]
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclarator(const Type* type) -> Declarator;

        // Parse a list of declarator.
        //
        // EXPECT: 'ID'
        // PARSE: declarator_list
        //      - declarator_list := declarator [',' declarator]...
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclaratorList(const Type* type) -> std::vector<Declarator>;

        // Parse a declarator that cannot have an initializer.
        //
        // EXPECT: 'ID'
        // PARSE: declarator_no_init
        //      - declarator_no_init := 'ID' [array_spec]
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclaratorNoInit() -> Declarator;

        // Parse a list of declarator that cannot have an initializer.
        //
        // EXPECT: 'ID'
        // PARSE: declarator_list_no_init
        //      - declarator_list_no_init := declarator_no_init [',' declarator_no_init]...
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseDeclaratorListNoInit() -> std::vector<Declarator>;

        // Parse the function parameter list.
        //
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
        auto ParseFunctionDecl(SyntaxTokenID beginTokIndex, AstQualType* returnType) -> AstDecl*;

        // EXPECT: ';' or 'ID'
        //
        // PARSE: type_or_variable_decl
        //      - type_or_variable_decl := ';'
        //      - type_or_variable_decl := declarator [',' declarator]... ';'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseTypeOrVariableDecl(SyntaxTokenID beginTokIndex, AstQualType* variableType) -> AstDecl*;

        // EXPECT: 'ID' '{' or '{'
        //
        // PARSE: interface_block_decl
        //      - interface_block_decl := 'ID' '{' [declartion]... '}' [declarator] ';'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseInterfaceBlockDecl(SyntaxTokenID beginTokIndex, AstTypeQualifierSeq* quals) -> AstDecl*;

        // EXPECT: 'K_precision'
        //
        // PARSE: precision_decl
        //      - precision_decl := 'K_precision' precision-qual type ';'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParsePrecisionDecl() -> AstDecl*;

#pragma endregion

#pragma region Parsing Expr

        // Parse an expression. If no token is consumed, enter recovery mode to advance parsing.
        //
        // PARSE: expr
        //      - expr := comma_expr
        //
        // ACCEPT: null
        //
        // RECOVERY: yes
        auto ParseExpr() -> AstExpr*;

        // Parse an expression without comma operator. If no token is consumed, enter recovery mode to advance parsing.
        //
        // PARSE: assignment_expr
        //
        // RECOVERY: yes
        auto ParseExprNoComma() -> AstExpr*;

        // Parse a comma expression.
        //
        // PARSE: comma_expr
        //      - comma_expr := assignment_expr [',' assignment_expr]...
        auto ParseCommaExpr() -> AstExpr*;

        // Parse an assignment expreesion.
        //
        // PARSE: assignment_expr
        //      - assignment_expr := unary_expr '?=' assignment_expr
        //      - assignment_expr := conditional_expr
        auto ParseAssignmentExpr() -> AstExpr*;

        // Parse an assignment expression with lhs unary expression already parsed.
        //
        // PARSE: assignment_expr
        //      - assignment_expr := unary_expr '?=' assignment_expr
        //      - assignment_expr := conditional_expr
        auto ParseAssignmentExprWithLhs(AstExpr* lhs) -> AstExpr*;

        // Parse a conditional expression with a part of the first binary expression already parsed.
        //
        // PARSE: conditional_expr
        //      - conditional_expr := binary_expr
        //      - conditional_expr := binary_expr '?' comma_expr ':' assignment_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseConditionalExpr(SyntaxTokenID beginTokIndex, AstExpr* firstTerm) -> AstExpr*;

        // PARSE: binary_expr
        //      - binary_expr := unary_expr
        //      - binary_expr := binary_expr 'binary_op' binary_expr
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        //
        // `firstTerm` for this is a unary expression, which might already be parsed
        auto ParseBinaryExpr(SyntaxTokenID beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*;

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

        // Parse an constructor call expression, assuming the type specifier has already been parsed.
        //
        // EXPECT: 'K_struct' or 'K_???' or 'ID' (aka. type_spec)
        //
        // PARSE: constructor_call
        //      - constructor_call := type_spec func_arg_list
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseConstructorCallExpr(AstQualType* typeSpec) -> AstExpr*;

        // Parse an primary expression.
        //
        // PARSE: primary_expr
        //      - primary_expr := 'ID'
        //      - primary_expr := 'constant'
        //      - primary_expr := paren_wrapped_expr
        //
        // ACCEPT: null
        auto ParsePrimaryExpr() -> AstExpr*;

        // Parse an expression wrapped in parentheses pair.
        //
        // EXPECT: '('
        //
        // PARSE: paren_wrapped_expr
        //      - paren_wrapped_expr := '(' expr ')'
        //
        // ACCEPT: '(' paren_recovery ')'
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

        // Parse an expression statement. For disambiguation, the type specifier part may has already been parsed for a
        // constructor call.
        //
        // PARSE: expr_stmt
        //      - expr_stmt := expr ';'
        //
        // ACCEPT: expr
        //
        // RECOVERY: unknown
        auto ParseExprStmt(AstQualType* typeSpec) -> AstStmt*;

        // Parse a declaration statement. For disambiguation, the type specifier part may has already been parsed for a
        // struct/variable declaration.
        //
        // PARSE: decl_stmt
        //      - decl_stmt := declaration
        //
        // RECOVERY: unknown
        auto ParseDeclStmt(AstQualType* typeSpec) -> AstStmt*;

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

        auto PeekToken() const noexcept -> const RawSyntaxTokenEntry&
        {
            GLSLD_ASSERT(currentTok != nullptr);
            return *currentTok;
        }

        auto PeekToken(size_t lookahead) const noexcept -> const RawSyntaxTokenEntry&
        {
            GLSLD_ASSERT(currentTok != nullptr);
            if (currentTok + lookahead < tokens.data() + tokens.size()) {
                return currentTok[lookahead];
            }
            else {
                return tokens.back();
            }
        }

        auto GetCurrentTokenID() const noexcept -> SyntaxTokenID
        {
            GLSLD_ASSERT(currentTok != nullptr);
            return SyntaxTokenID{tuID, static_cast<uint32_t>(currentTok - tokens.data())};
        }

        auto GetCurrentToken() const noexcept -> SyntaxToken
        {
            return SyntaxToken{
                .index = GetCurrentTokenID(),
                .klass = PeekToken().klass,
                .text  = PeekToken().text,
            };
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

        // Tests if the current token is an EOF token.
        // Notably, this doesn't mean we are running out of tokens. We may have multiple files in the token stream.
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
            return astBuilder.BuildErrorExpr(CreateAstSyntaxRange(GetCurrentTokenID()));
        }

        auto CreateErrorExpr(SyntaxTokenID beginTokIndex) -> AstErrorExpr*
        {
            return astBuilder.BuildErrorExpr(CreateAstSyntaxRange(beginTokIndex));
        }

        auto CreateErrorStmt() -> AstErrorStmt*
        {
            return astBuilder.BuildErrorStmt(CreateAstSyntaxRange(GetCurrentTokenID()));
        }

        auto CreateAstSyntaxRange(SyntaxTokenID beginTokIndex) -> AstSyntaxRange
        {
            return AstSyntaxRange{beginTokIndex, GetCurrentTokenID()};
        }

        // Report an error at the given token.
        auto ReportError(SyntaxTokenID tokIndex, std::string message) -> void
        {
            diagReporter.ReportError(AstSyntaxRange{tokIndex}, std::move(message));
        }

        // Report an error at the current token.
        auto ReportError(std::string message) -> void
        {
            ReportError(GetCurrentTokenID(), std::move(message));
        }
    };

} // namespace glsld
