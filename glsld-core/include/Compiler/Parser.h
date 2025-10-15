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
    // EXPECT: The expected next token, aka. the first set of the grammar rule.
    // PARSE: Grammar rules that are going to be parsed.
    // ACCEPT: Additional grammar rules to be accepted (for permissive parsing).
    // RECOVERY: If specified, this parser may be in a unknown state and should try to recover.
    class Parser
    {
    private:
        CompilerInvocationState& compiler;

        DiagnosticReportor diagReporter;

        AstBuilder astBuilder;

        TranslationUnitID tuID;

        ArrayView<RawSyntaxToken> tokens;

        const RawSyntaxToken* currentTok = nullptr;

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

        class ParsingBalancedBracketGuard
        {
        private:
            Parser& parser;
            size_t leftBracketDepth;

        public:
            ParsingBalancedBracketGuard(Parser& parser) : parser(parser)
            {
                GLSLD_ASSERT(parser.TryTestToken(TokenKlass::LBracket));
                parser.ConsumeToken();
                leftBracketDepth = parser.bracketDepth;
            }

            ~ParsingBalancedBracketGuard()
            {
                parser.ParseClosingBracketHelper(leftBracketDepth);
            }

            ParsingBalancedBracketGuard(const ParsingBalancedBracketGuard&)            = delete;
            ParsingBalancedBracketGuard& operator=(const ParsingBalancedBracketGuard&) = delete;

            auto GetLeftBracketDepth() const noexcept -> size_t
            {
                return leftBracketDepth;
            }
        };

    public:
        Parser(CompilerInvocationState& compiler, TranslationUnitID tuID, ArrayView<RawSyntaxToken> tokens)
            : compiler(compiler), astBuilder(compiler), diagReporter(compiler.GetDiagnosticStream()), tuID(tuID),
              tokens(tokens)
        {
            GLSLD_ASSERT(!tokens.empty() && tokens.back().klass == TokenKlass::Eof);
            currentTok = tokens.data();
        }

        auto DoParse() -> void;

    private:
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
        // ACCEPT: comma_recovery [',']
        auto ParseCommaInParenHelper(size_t leftParenDepth) -> bool;

        // Parse a closing ')' with the assumption that a balanced '(' has been parsed.
        //
        // PARSE: ')'
        //
        // ACCEPT: paren_recovery [')']
        auto ParseClosingParenHelper(size_t leftParenDepth) -> void;

        // Parse a closing ']' with the assumption that a balanced '[' has been parsed.
        //
        // PARSE: ']'
        //
        // ACCEPT: bracket_recovery ']'
        auto ParseClosingBracketHelper(size_t leftBracketDepth) -> void;

        // Try to parse an identifier token as a symbol name if available, otherwise returns an invalid token.
        //
        // PARSE: 'ID'
        //
        // ACCEPT: null
        auto ParseOptionalDeclIdHelper() -> AstSyntaxToken;

        auto ParseDeclIdHelper() -> AstSyntaxToken;

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
        //      - layout_qual := 'K_layout'
        //      - layout_qual := 'K_layout' '(' paren_recovery [')']
        //      - layout_spec := 'ID' comma_recovery
        //      - layout_spec := comma_recovery
        auto ParseLayoutQualifier(std::vector<LayoutItem>& items) -> void;

        // Try to parse a sequence of qualifiers. Returns nullptr if no qualifier is parsed.
        //
        // PARSE: qual_seq
        //      - qual_seq := [qualifier]...
        //      - qualifier := layout_qual
        //      - qualifier := 'K_<qualifier>'
        auto ParseTypeQualifierSeq() -> AstTypeQualifierSeq*;

        // EXPECT: '{'
        //
        // PARSE: struct_body
        //      - struct_body := '{' [field_decl]... '}'
        //      - field_decl := ';'
        //      - field_decl := qualified_type_spec declarator_list_no_init ';'
        //
        // ACCEPT:
        //      - struct_body := '{' [field_decl]... brace_recovery ['}']
        //      - field_decl := qualified_type_spec declarator_list_no_init
        //      - field_decl := qualified_type_spec declarator_list_no_init semi_recovery [';']
        //
        auto ParseStructBody() -> std::vector<AstStructFieldDecl*>;

        // EXPECT: 'K_struct'
        //
        // PARSE: struct_definition
        //      - struct_definition := 'K_struct' ['ID'] struct_body
        //
        // ACCEPT:
        //      - struct_definition := 'K_struct'
        //      - struct_definition := 'K_struct' ['ID']
        auto ParseStructDefinition() -> AstStructDecl*;

        // EXPECT: '['
        //
        // PARSE: array_spec
        //      - array_spec := bracket_wrapped_expr...
        auto ParseArraySpec() -> AstArraySpec*;

        // Parse a type specifier and combine it with a previously parsed qualifier sequence, which could be either:
        // 1. a struct definition
        // 2. an identifier that's a type name
        // 3. a keyword that's a built-in type name
        // NOTE qualifier is previously parsed and passed in as a parameter.
        //
        // PARSE: type_spec
        //      - type_spec := struct_definition
        //      - type_spec := 'ID'
        //      - type_spec := 'K_<primitive_type>'
        auto ParseTypeSpec(AstTypeQualifierSeq* quals) -> AstQualType*;

        // Parse a qualfied type specifier.
        //
        // PARSE: qualified_type_spec
        //      - qualified_type_spec := [qual_seq] type_spec
        auto ParseQualType() -> AstQualType*;

        // PARSE: declaration
        //
        // ACCEPT: [declaration] semi_recovery [';']
        auto ParseDeclAndTryRecover(AstQualType* typeSpec, bool atGlobalScope) -> AstDecl*;

        // PARSE: stmt
        //
        // RECOVERY: ^'EOF'
        auto ParseStmtAndTryRecover() -> AstStmt*;

#pragma endregion

#pragma region Parsing Decl

        // Parse a declaration. Notably, a GLSL declaration could be associated with multiple AST nodes.
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
        auto ParseInitializer(const Type* type) -> AstInitializer*;

        // Parse an initializer list.
        //
        // PARSE: initializer_list
        //      - initializer_list := '{' initializer [',' initializer]... [,] '}'
        //
        // ACCEPT:
        //      - initializer_list := '{' initializer [',' initializer]... [,] brace_recovery ['}']
        auto ParseInitializerList(const Type* type) -> AstInitializerList*;

        // Parse a declarator.
        //
        // EXPECT: 'ID'
        //
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

        // EXPECT: '{'
        //
        // PARSE: block_body
        //      - block_body := '{' [field_decl]... '}'
        //      - field_decl := ';'
        //      - field_decl := qualified_type_spec declarator_list_no_init ';'
        //
        // ACCEPT:
        //      - block_body := '{' [field_decl]... brace_recovery ['}']
        //      - field_decl := qualified_type_spec declarator_list_no_init
        //      - field_decl := qualified_type_spec declarator_list_no_init semi_recovery [';']
        //
        auto ParseBlockBody() -> std::vector<AstBlockFieldDecl*>;

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

        // Parse an error-permissive expression, each term of which may be null.
        // If no token is consumed, enter recovery mode to advance parsing.
        //
        // PARSE: expr
        //      - expr := comma_expr
        //
        // RECOVERY: yes
        auto ParseExpr() -> AstExpr*;

        // Parse an error-permissive expression without comma operator, each term of which may be null.
        // If no token is consumed, enter recovery mode to advance parsing.
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
        auto ParseConditionalExpr(SyntaxTokenID beginTokIndex, AstExpr* firstTerm) -> AstExpr*;

        // PARSE: binary_expr
        //      - binary_expr := unary_expr
        //      - binary_expr := binary_expr 'binary_op' binary_expr
        //
        // `firstTerm` for this is a unary expression, which might already be parsed
        auto ParseBinaryExpr(SyntaxTokenID beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*;

        // Parse an unary expression.
        //
        // PARSE: unary_expr
        //      - unary_expr := ['unary_op']... postfix_expr
        auto ParseUnaryExpr() -> AstExpr*;

        // Parse an postfix expression.
        //
        // PARSE: postfix_expr
        //      - postfix_expr := primary_expr
        //      - postfix_expr := function_call
        //      - postfix_expr := constructor_call
        //      - postfix_expr := postfix_expr '++'
        //      - postfix_expr := postfix_expr '--'
        //      - postfix_expr := postfix_expr '.' 'ID'
        //      - postfix_expr := postfix_expr bracket_wrapped_expr
        //      - function_call := postfix_expr func_arg_list
        auto ParsePostfixExpr(AstExpr* parsedExpr) -> AstExpr*;

        // Parse an constructor call expression, assuming the type specifier has already been parsed.
        //
        // EXPECT: 'K_struct' or 'K_???' or 'ID' (aka. type_spec)
        //
        // PARSE: constructor_call
        //      - constructor_call := type_spec func_arg_list
        auto ParseConstructorCallExpr(AstQualType* typeSpec) -> AstExpr*;

        // Parse an primary expression.
        //
        // PARSE: primary_expr
        //      - primary_expr := 'ID'
        //      - primary_expr := 'constant'
        //      - primary_expr := paren_wrapped_expr
        //
        // ACCEPT:
        //      - primary_expr := null
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

        // Parse an expression wrapped in brackets pair. If `emptyIsError` is true, an empty bracket pair is considered
        // an AstErrorExpr. Otherwise, this returns nullptr.
        //
        // EXPRECT: '['
        //
        // PARSE: bracket_wrapped_expr
        //      - bracket_wrapped_expr := '[' expr ']'
        //
        // ACCEPT:
        //      - bracket_wrapped_expr := '[' expr bracket_recovery [']']
        //      - bracket_wrapped_expr := '[' bracket_recovery [']']
        auto ParseBracketWrappedExpr(bool emptyIsError) -> AstExpr*;

        // Parse a function argument list.
        //
        // EXPECT: '('
        //
        // PARSE: func_arg_list
        //      - func_arg_list := '(' ['K_void'] ')'
        //      - func_arg_list := '(' assignment_expr [',' assignment_expr]... ')'
        //
        // ACCEPT:
        //      - func_arg_list := '(' ['K_void'] paren_recovery [')']
        //      - func_arg_list := '(' assignment_expr [',' assignment_expr]... paren_recovery [')']
        auto ParseFunctionArgumentList() -> std::vector<AstExpr*>;

#pragma endregion

#pragma region Parsing Stmt

        // PARSE: stmt
        //      - stmt := ';'
        //      - stmt := compound_stmt
        //      - stmt := selection_stmt
        //      - stmt := for_stmt
        //      - stmt := dowhile_stmt
        //      - stmt := while_stmt
        //      - stmt := switch_stmt
        //      - stmt := jump_stmt
        //      - stmt := expr_stmt
        //      - stmt := decl_stmt
        auto ParseStmt() -> AstStmt*;

        // EXPECT: '{'
        //
        // PARSE: compound_stmt
        //      - compound_stmt := '{' [stmt]... '}'
        //
        // ACCEPT:
        //      - compound_stmt := '{' [stmt]... brace_recovery ['}']
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
        // ACCEPT:
        //      - dowhile_stmt := 'K_do' stmt 'K_while' paren_wrapped_expr
        auto ParseDoWhileStmt() -> AstStmt*;

        // EXPECT: 'K_while'
        //
        // PARSE: while_stmt
        //      - while_stmt := 'K_while' paren_wrapped_expr stmt
        auto ParseWhileStmt() -> AstStmt*;

        // EXPECT: 'K_case' or 'K_default'
        //
        // PARSE: label_stmt
        //      - label_stmt := 'K_case' expr ':'
        //      - label_stmt := 'K_default' ':'
        // ACCEPT:
        //      - label_stmt := 'K_case' expr
        //      - label_stmt := 'K_default'
        auto ParseLabelStmt() -> AstStmt*;

        // EXPECT: 'K_switch'
        //
        // PARSE: switch_stmt
        //      - switch_stmt := 'K_switch' paren_wrapped_expr switch_body
        //      - switch_body := '{' [stmt]... '}'
        //
        // ACCEPT:
        //      - switch_body := '{' [stmt]... brace_recovery ['}']
        auto ParseSwitchStmt() -> AstStmt*;

        // Parse jump statement.
        //
        // EXPECT: 'K_break' or 'K_continue' or 'K_discard' or 'K_return'
        //
        // PARSE: jump_stmt
        //      - jump_stmt := 'K_break' ';'
        //      - jump_stmt := 'K_continue' ';'
        //      - jump_stmt := 'K_discard' ';'
        //      - jump_stmt := 'K_return' [expr] ';'
        //
        // ACCEPT:
        //      - jump_stmt := 'K_break'
        //      - jump_stmt := 'K_continue'
        //      - jump_stmt := 'K_discard'
        //      - jump_stmt := 'K_return' expr
        auto ParseJumpStmt() -> AstStmt*;

        // Parse an expression statement. For disambiguation, the type specifier part may has already been parsed for a
        // constructor call.
        // At least one token must be consumed, including the type specifier part.
        //
        // PARSE: expr_stmt
        //      - expr_stmt := expr ';'
        //
        // ACCEPT:
        //      - expr_stmt := expr [';']
        auto ParseExprStmt(AstQualType* typeSpec) -> AstStmt*;

        // Parse a declaration statement. For disambiguation, the type specifier part may has already been parsed for a
        // struct/variable declaration.
        //
        // PARSE: decl_stmt
        //      - decl_stmt := declaration
        auto ParseDeclStmt(AstQualType* typeSpec) -> AstStmt*;

        // PARSE: decl_or_expr_stmt
        //      - decl_or_expr_stmt := expr_stmt
        //      - decl_or_expr_stmt := decl_stmt
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
        //
        // ACCEPT: comma_recovery := ...
        //         paren_recovery := ...
        //         bracket_recovery := ...
        //         brace_recovery := ...
        //         ilist_brace_recovery := ...
        //         semi_recovery := ...
        auto RecoverFromError(RecoveryMode mode) -> void;

        auto PeekToken() const noexcept -> const RawSyntaxToken&
        {
            GLSLD_ASSERT(currentTok != nullptr);
            return *currentTok;
        }

        auto PeekToken(size_t lookahead) const noexcept -> const RawSyntaxToken&
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

        auto GetCurrentToken() const noexcept -> AstSyntaxToken
        {
            return AstSyntaxToken{
                .id    = GetCurrentTokenID(),
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
