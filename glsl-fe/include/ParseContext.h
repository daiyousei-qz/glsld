#pragma once
#include "DiagnosticContext.h"
#include "LexContext.h"
#include "ParsedAst.h"
#include "Tokenizer.h"
#include "Typing.h"
#include "ParserTrace.h"

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace glsld
{
    class ParseContext;

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
    auto ParseError(T&& result) -> ParseResult<T>
    {
        return ParseResult<T>{std::forward<T>(result), false};
    }

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
        auto ParseDeclId() -> ParseResult<SyntaxToken>
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
        auto DoParseTranslationUnit() -> void
        {
            TRACE_PARSER();

            while (!Eof()) {
                auto declResult = ParseDeclaration();
                if (!declResult.Success()) {
                    RecoverFromBadDecl();
                }

                ast.Add(declResult.Get());
                // FIXME: register decl
            }
        }

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
        auto ParseLayoutQualifier() -> void
        {
            TRACE_PARSER();
        }

        // Try to parse a sequence of qualifiers
        // PARSE: qual_seq
        //      - qual_seq := ['qual_keyword']...
        //
        // FIXME: layout qual
        auto ParseTypeQualifiers() -> AstTypeQualifierSeq*
        {
            TRACE_PARSER();

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
            return CreateAstNode<AstTypeQualifierSeq>(beginTokIndex, result);
        }

        // EXPECT: '{'
        // PARSE: struct_body
        //      - struct_body := '{' member_decl... '}'
        //      - member_decl := ?
        //
        // ACCEPT: '{' ??? '}'
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseStructBody() -> ParseResult<std::vector<AstStructMemberDecl*>>
        {
            TRACE_PARSER();

            ConsumeTokenAssert(TokenKlass::LBrace);

            if (TryTestToken(TokenKlass::RBrace)) {
                // empty struct body
                return {{}};
            }

            std::vector<AstStructMemberDecl*> result;
            while (true) {
                auto typeResult  = ParseQualType();
                auto declarators = ParseVariableDeclarators();

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
        //
        // PARSE: struct_definition
        //      - struct_definition := 'struct' ['ID'] struct_body
        //
        // RECOVERY:
        auto ParseStructDefinition() -> ParseResult<AstStructDecl*>
        {
            TRACE_PARSER();

            auto beginTokIndex = GetTokenIndex();
            ConsumeTokenAssert(TokenKlass::K_struct);

            // Parse the declared struct type name
            std::optional<SyntaxToken> declTok = std::nullopt;
            if (TryTestToken(TokenKlass::Identifier)) {
                declTok = PeekToken();
                ConsumeToken();
            }

            // Parse the struct body
            if (TryTestToken(TokenKlass::LBrace)) {
                auto structBodyResult = ParseStructBody();
                return {structBodyResult.Success(),
                        CreateAstNode<AstStructDecl>(beginTokIndex, declTok, structBodyResult.Move())};
            }

            // FIXME: could we do better on recovery?
            return {false, nullptr};
        }

        // EXPECT: '['
        // PARSE: array_spec
        //      - array_spec := ('[' [expr] ']')...
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseArraySpec() -> ParseResult<AstArraySpec*>
        {
            TRACE_PARSER();

            GLSLD_ASSERT(PeekToken().klass == TokenKlass::LBracket);
            auto beginTokIndex = GetTokenIndex();

            std::vector<AstExpr*> sizes;
            while (TryConsumeToken(TokenKlass::LBracket)) {

                // Parse unsized specifier
                if (TryConsumeToken(TokenKlass::RBracket)) {
                    sizes.push_back(nullptr);
                    continue;
                }

                // TODO: expr? assignment_expr?
                sizes.push_back(ParseExpr().Move());

                if (!TryConsumeToken(TokenKlass::RBracket)) {
                    ReportError("expecting ]");
                    // FIXME: end global decl?
                    RecoverFromError(RecoveryMode::Semi);
                    if (!TryConsumeToken(TokenKlass::RBracket)) {
                        return {false, CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes))};
                    }
                }
            }

            GLSLD_ASSERT(!sizes.empty());
            return CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes));
        }

        // PARSE: type_spec
        //      - type_spec := struct_definition
        //      - type_spec := 'ID'
        //      - type_spec := 'K_???'
        //
        // RECOVERY:
        auto ParseType(AstTypeQualifierSeq* quals) -> ParseResult<AstQualType*>
        {
            TRACE_PARSER();

            // FIXME: shouldn't this be the beginning of qualifiers?
            auto beginTokIndex = GetTokenIndex();

            // FIXME: parse array spec
            if (TryTestToken(TokenKlass::K_struct)) {
                auto structDefinitionResult = ParseStructDefinition();
                return {structDefinitionResult.Success(),
                        CreateAstNode<AstQualType>(beginTokIndex, quals, structDefinitionResult.Get())};
            }

            if (TryTestToken(TokenKlass::Identifier) || GetBuiltinType(PeekToken()).has_value()) {
                auto tok = PeekToken();
                ConsumeToken();
                return CreateAstNode<AstQualType>(beginTokIndex, quals, tok);
            }

            // TODO: handle error
            return nullptr;
        }

        // PARSE: qualified_type_spec
        //        qualified_type_spec := [qual_seq] type_spec
        auto ParseQualType() -> ParseResult<AstQualType*>
        {
            TRACE_PARSER();

            auto qualifiers = ParseTypeQualifiers();
            return ParseType(qualifiers);
        }

#pragma endregion

#pragma region Parsing Decl

        // ACCEPT: ??? ';'
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
        auto ParseDeclaration() -> ParseResult<AstDecl*>
        {
            TRACE_PARSER();

            auto beginTokIndex = 0;
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                // empty decl
                return CreateAstNode<AstEmptyDecl>(beginTokIndex);
            }

            if (TryTestToken(TokenKlass::K_precision)) {
                // precision decl
                // FIXME: implement this
                ReportError("precision decl not supported yet");
                return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
            }

            auto quals = ParseTypeQualifiers();
            if (TryTestToken(TokenKlass::Semicolon)) {
                // default qualifier decl
                // FIXME: implement this
                ReportError("default qualifier decl not supported yet");
                return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
            }
            else if (TryTestToken(TokenKlass::Identifier) &&
                     TryTestToken(TokenKlass::Semicolon, TokenKlass::Comma, 1)) {
                // qualifier overwrite decl
                // FIXME: implement this
                ReportError("qualifier overwrite decl not supported yet");
                return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
            }
            if (TryTestToken(TokenKlass::LBrace) || TryTestToken(TokenKlass::LBrace, 1)) {
                // interface blocks
                // for example, uniform UBO { ... }
                // FIXME: only enter here if qualifier can actually start a interface block
                return ParseInterfaceBlockDecl(beginTokIndex, quals);
            }
            else {
                // function/variable decl
                auto typeResult = ParseType(quals);
                if (!typeResult.Success()) {
                    // FIXME: what to do here?
                    ReportError("what to do here?");
                    return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
                }

                if (TryTestToken(TokenKlass::LParen) ||
                    (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LParen, 1))) {
                    // function decl
                    return ParseFunctionDecl(beginTokIndex, typeResult.Get());
                }
                else if (TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier)) {
                    // type/variable decl
                    return ParseTypeOrVariableDecl(beginTokIndex, typeResult.Get());
                }
                else {
                    // unknown decl
                    ReportError("unknown decl");
                    return {false, CreateAstNode<AstEmptyDecl>(beginTokIndex)};
                }
            }
        }

        auto ParseDeclNoRecovery() -> ParseResult<AstDecl*>
        {
            auto declResult = ParseDeclaration();
            if (!declResult.Success()) {
                RecoverFromBadDecl();
            }

            return {true, declResult.Get()};
        }

        // EXPECT: 'ID'
        // PARSE: declarator_list
        //      - declarator_list := declarator [',' declarator]...
        //      - declarator := 'ID' [array_spec] ['=' initializer]
        //      - initializer := ?
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseVariableDeclarators() -> ParseResult<std::vector<VariableDeclarator>>
        {
            TRACE_PARSER();

            bool recoveryMode = false;
            std::vector<VariableDeclarator> result;
            while (!Eof()) {
                auto declTok = ParseDeclId();

                AstArraySpec* arraySpec = nullptr;
                if (TryTestToken(TokenKlass::LBracket)) {
                    auto arraySpecResult = ParseArraySpec();
                    if (!arraySpecResult.Success()) {
                        recoveryMode = true;
                    }

                    arraySpec = arraySpecResult.Get();
                }

                // FIXME: parse init
                AstExpr* init = nullptr;
                if (TryConsumeToken(TokenKlass::Assign)) {
                    auto initResult = ParseExpr();
                    if (!initResult.Success()) {
                        recoveryMode = true;
                    }

                    init = initResult.Get();
                }

                result.push_back(VariableDeclarator{.declTok = declTok.Get(), .arraySize = arraySpec, .init = init});
                if (recoveryMode || !TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }

            return {!recoveryMode, std::move(result)};
        }

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
        auto ParseFunctionParamList() -> ParseResult<std::vector<AstParamDecl*>>
        {
            TRACE_PARSER();

            ConsumeTokenAssert(TokenKlass::LParen);

            // empty parameter list
            if (TryConsumeToken(TokenKlass::RParen)) {
                return {{}};
            }

            // TODO: needed? spec doesn't include this grammar.
            if (TryTestToken(TokenKlass::K_void) && TryTestToken(TokenKlass::RParen, 1)) {
                ConsumeToken();
                ConsumeToken();
                return {{}};
            }

            // parse parameters
            std::vector<AstParamDecl*> result;
            while (PeekToken().klass != TokenKlass::Eof) {
                auto beginTokIndex = GetTokenIndex();

                auto type = ParseQualType();
                if (!type.Success()) {
                    break;
                }

                auto id = ParseDeclId();
                if (id.Get().klass == TokenKlass::Error) {
                    break;
                }

                result.push_back(CreateAstNode<AstParamDecl>(beginTokIndex, type.Move(), id.Move()));

                if (!TryConsumeToken(TokenKlass::Comma)) {
                    break;
                }
            }

            bool closed = TryConsumeToken(TokenKlass::RParen);
            if (!closed) {
                // FIXME: end local paren?
                RecoverFromError(RecoveryMode::Paren);
                closed = TryConsumeToken(TokenKlass::RParen);
            }

            return {closed, std::move(result)};
        }

        // EXPECT: 'ID' or '('
        //
        // PARSE: func_decl
        //      - func_decl := 'ID' func_param_list ';'
        //      - func_decl := 'ID' func_param_list compound_stmt
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> ParseResult<AstDecl*>
        {
            TRACE_PARSER();

            // Parse function name
            auto declTok = ParseDeclId().Get();

            // Parse function parameter list
            auto paramsResult = ParseFunctionParamList();

            if (TryTestToken(TokenKlass::LBrace)) {
                GLSLD_ASSERT(paramsResult.Success());

                auto bodyResult = ParseCompoundStmt();
                return {bodyResult.Success(), CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok,
                                                                             paramsResult.Move(), bodyResult.Get())};
            }

            // Parse trailing ';'
            // NOTE we will keep the ';' for recovery if parsing failed
            if (paramsResult.Success() && !TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expect ';' or function body");
            }

            return {paramsResult.Success(),
                    CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok, paramsResult.Move())};
        }

        // EXPECT: ';' or 'ID'
        //
        // PARSE: type_or_variable_decl
        //      - type_or_variable_decl := ';'
        //      - type_or_variable_decl := declarator [',' declarator]... ';'
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseTypeOrVariableDecl(size_t beginTokIndex, AstQualType* variableType) -> ParseResult<AstDecl*>
        {
            TRACE_PARSER();

            GLSLD_ASSERT(TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier));

            // Parse type decl
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType);
            }

            // Parse variable decl
            auto declaratorsResult = ParseVariableDeclarators();

            // Parse trailing ';'
            // NOTE we will keep the ';' for recovery if parsing failed
            if (declaratorsResult.Success()) {
                ParsePermissiveSemicolon();
            }

            return {declaratorsResult.Success(),
                    CreateAstNode<AstVariableDecl>(beginTokIndex, variableType, declaratorsResult.Move())};
        }

        // EXPECT: 'ID' or '{'
        // RECOVERY: ^'EOF' or ^';'
        auto ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> ParseResult<AstDecl*>
        {
            TRACE_PARSER();

            GLSLD_NO_IMPL();
        }

        // EXPECT: 'K_precision'
        // PARSE: precision_decl
        //      - precision_decl := ???
        //
        // RECOVERY: ^'EOF' or ^';'
        auto ParsePrecisionDecl() -> ParseResult<AstDecl*>
        {
            TRACE_PARSER();

            GLSLD_NO_IMPL();
        }

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

        auto ParseStmtNoRecovery() -> ParseResult<AstStmt*>
        {
            auto stmtResult = ParseStmt();
            if (!stmtResult.Success()) {
                RecoverFromBadStmt();
            }

            return {true, stmtResult.Get()};
        }

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
        //      - jump_stmt := 'K_break' ';'
        //      - jump_stmt := 'K_continue' ';'
        //      - jump_stmt := 'K_discard' ';'
        //      - jump_stmt := 'K_return' [expr] ';'
        //
        // ACCEPT: ??? (';' could be inferred?)
        //
        // RECOVERY: ^'EOF' or ^';' or ^'}'
        auto ParseJumpStmt() -> ParseResult<AstStmt*>;

        auto ParseExprStmt() -> ParseResult<AstStmt*>;

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

            while (PeekToken().klass != TokenKlass::Eof) {
                switch (PeekToken().klass) {
                case TokenKlass::RParen:
                    if (mode == RecoveryMode::Paren && parenDepth == initParenDepth) {
                        return;
                    }
                    break;
                case TokenKlass::RBracket:
                    if (mode == RecoveryMode::Bracket && bracketDepth == initBracketDepth) {
                        return;
                    }
                    break;
                case TokenKlass::RBrace:
                    if ((mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket || mode == RecoveryMode::Brace ||
                         mode == RecoveryMode::Semi) &&
                        braceDepth == initBraceDepth) {
                        return;
                    }
                    break;
                case TokenKlass::Semicolon:
                    if ((mode == RecoveryMode::Paren || mode == RecoveryMode::Bracket || mode == RecoveryMode::Semi) &&
                        braceDepth == initBraceDepth) {
                        return;
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
            requires std::is_base_of_v<SyntaxNode, T>
        auto CreateRangedAstNode(size_t beginTokIndex, size_t endTokIndex, Args&&... args) -> T*
        {
            auto result = new T(std::forward<Args>(args)...);
            result->UpdateRange(lexer->GetSyntaxRange(beginTokIndex, endTokIndex));
            return result;
        }

        template <typename T, typename... Args>
            requires std::is_base_of_v<SyntaxNode, T>
        auto CreateAstNode(size_t beginTokIndex, Args&&... args) -> T*
        {
            auto result = new T(std::forward<Args>(args)...);
            result->UpdateRange(lexer->GetSyntaxRange(beginTokIndex, GetTokenIndex()));
            return result;
        }

    private:
        size_t parenDepth      = 0;
        size_t bracketDepth    = 0;
        size_t braceDepth      = 0;
        size_t scopeBraceDepth = 0;

        LexContext* lexer      = nullptr;
        size_t currentTokIndex = 0;
        SyntaxToken currentTok = {};

        ParsedAst ast;

        DiagnosticContext* diagCtx;
    };

} // namespace glsld