#include "Parser.h"

namespace glsld
{
#pragma region Parsing QualType
    auto Parser::ParseLayoutQualifier() -> void
    {
        TRACE_PARSER();
    }

    auto Parser::ParseTypeQualifiers() -> AstTypeQualifierSeq*
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

    auto Parser::ParseStructBody() -> std::vector<AstStructMemberDecl*>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LBrace);

        if (TryTestToken(TokenKlass::RBrace)) {
            // empty struct body
            return {};
        }

        std::vector<AstStructMemberDecl*> result;
        while (!Eof()) {
            auto typeResult  = ParseQualType();
            auto declarators = ParseVariableDeclarators();

            if (!TryTestToken(TokenKlass::Semicolon, TokenKlass::RBrace)) {
                // FIXME: error recovery
            }
            if (TryTestToken(TokenKlass::Semicolon)) {
                ConsumeToken();
            }
            if (TryTestToken(TokenKlass::RBrace)) {
                // FIXME: error recovery
                ConsumeToken();
                break;
            }
        }
        return result;
    }

    auto Parser::ParseStructDefinition() -> AstStructDecl*
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
            return CreateAstNode<AstStructDecl>(beginTokIndex, declTok, structBodyResult);
        }

        // FIXME: could we do better on recovery?
        EnterRecoveryMode();
        return nullptr;
    }

    auto Parser::ParseArraySpec() -> AstArraySpec*
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
            sizes.push_back(ParseExpr());

            ParseClosingBracket();
            if (InRecoveryMode()) {
                break;
            }
        }

        return CreateAstNode<AstArraySpec>(beginTokIndex, std::move(sizes));
    }

    auto Parser::ParseType(AstTypeQualifierSeq* quals) -> AstQualType*
    {
        TRACE_PARSER();

        // FIXME: shouldn't this be the beginning of qualifiers?
        auto beginTokIndex = GetTokenIndex();

        // FIXME: parse array spec
        if (TryTestToken(TokenKlass::K_struct)) {
            auto structDefinitionResult = ParseStructDefinition();
            return CreateAstNode<AstQualType>(beginTokIndex, quals, structDefinitionResult);
        }

        if (TryTestToken(TokenKlass::Identifier) || GetBuiltinType(PeekToken()).has_value()) {
            auto tok = PeekToken();
            ConsumeToken();
            return CreateAstNode<AstQualType>(beginTokIndex, quals, tok);
        }

        // TODO: handle error
        return nullptr;
    }

    auto Parser::ParseQualType() -> AstQualType*
    {
        TRACE_PARSER();

        auto qualifiers = ParseTypeQualifiers();
        return ParseType(qualifiers);
    }

#pragma endregion

#pragma region Parsing Decl

    auto Parser::ParseDeclaration() -> AstDecl*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        if (TryConsumeToken(TokenKlass::Semicolon)) {
            // empty decl
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }

        if (TryTestToken(TokenKlass::K_precision)) {
            // precision decl
            // FIXME: implement this
            ReportError("precision decl not supported yet");
            EnterRecoveryMode();
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }

        auto quals = ParseTypeQualifiers();
        if (TryTestToken(TokenKlass::Semicolon)) {
            // default qualifier decl
            // FIXME: implement this
            ReportError("default qualifier decl not supported yet");
            EnterRecoveryMode();
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }
        else if (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::Semicolon, TokenKlass::Comma, 1)) {
            // qualifier overwrite decl
            // FIXME: implement this
            ReportError("qualifier overwrite decl not supported yet");
            EnterRecoveryMode();
            return CreateAstNode<AstEmptyDecl>(beginTokIndex);
        }
        if (TryTestToken(TokenKlass::LBrace) || TryTestToken(TokenKlass::LBrace, 1)) {
            // interface blocks
            // for example, uniform UBO { ... }
            // FIXME: only enter here if qualifier can actually start a interface block
            return ParseInterfaceBlockDecl(beginTokIndex, quals);
        }
        else {
            // function/variable decl
            auto type = ParseType(quals);
            if (InRecoveryMode()) {
                // FIXME: what to do here?
                ReportError("what to do here?");
                return CreateAstNode<AstEmptyDecl>(beginTokIndex);
            }

            if (TryTestToken(TokenKlass::LParen) ||
                (TryTestToken(TokenKlass::Identifier) && TryTestToken(TokenKlass::LParen, 1))) {
                // function decl
                return ParseFunctionDecl(beginTokIndex, type);
            }
            else if (TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier)) {
                // type/variable decl
                return ParseTypeOrVariableDecl(beginTokIndex, type);
            }
            else {
                // unknown decl
                ReportError("unknown decl");
                EnterRecoveryMode();
                return CreateAstNode<AstEmptyDecl>(beginTokIndex);
            }
        }
    }

    auto Parser::ParseDeclAndTryRecover() -> AstDecl*
    {
        auto declResult = ParseDeclaration();
        if (InRecoveryMode()) {
            RecoverFromError(RecoveryMode::Semi);
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                ExitRecoveryMode();
            }
        }

        return declResult;
    }

    auto Parser::ParseVariableDeclarators() -> std::vector<VariableDeclarator>
    {
        TRACE_PARSER();

        std::vector<VariableDeclarator> result;
        while (!Eof()) {
            auto declTok = ParseDeclId();

            AstArraySpec* arraySpec = nullptr;
            if (TryTestToken(TokenKlass::LBracket)) {
                arraySpec = ParseArraySpec();
            }

            // FIXME: parse init
            AstExpr* init = nullptr;
            if (TryConsumeToken(TokenKlass::Assign)) {
                init = ParseExpr();
            }

            result.push_back(VariableDeclarator{.declTok = declTok, .arraySize = arraySpec, .init = init});
            if (InRecoveryMode() || !TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        return std::move(result);
    }

    auto Parser::ParseFunctionParamList() -> std::vector<AstParamDecl*>
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
            if (InRecoveryMode()) {
                break;
            }

            auto id = ParseDeclId();
            if (id.klass == TokenKlass::Error) {
                break;
            }

            result.push_back(CreateAstNode<AstParamDecl>(beginTokIndex, type, id));

            if (!TryConsumeToken(TokenKlass::Comma)) {
                break;
            }
        }

        ParseClosingParen();
        return std::move(result);
    }

    auto Parser::ParseFunctionDecl(size_t beginTokIndex, AstQualType* returnType) -> AstDecl*
    {
        TRACE_PARSER();

        // Parse function name
        auto declTok = ParseDeclId();

        // Parse function parameter list
        auto params = ParseFunctionParamList();

        if (TryTestToken(TokenKlass::LBrace)) {
            // This is a definition
            GLSLD_ASSERT(InParsingMode());

            auto body = ParseCompoundStmt();
            return CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok, std::move(params), body);
        }
        else {
            // This is a declaration

            // Parse trailing ';'
            // NOTE we will keep the ';' for recovery if parsing failed
            if (InParsingMode() && !TryConsumeToken(TokenKlass::Semicolon)) {
                ReportError("expect ';' or function body");
            }

            return CreateAstNode<AstFunctionDecl>(beginTokIndex, returnType, declTok, std::move(params));
        }
    }

    auto Parser::ParseTypeOrVariableDecl(size_t beginTokIndex, AstQualType* variableType) -> AstDecl*
    {
        TRACE_PARSER();

        GLSLD_ASSERT(TryTestToken(TokenKlass::Semicolon, TokenKlass::Identifier));

        // Parse type decl
        if (TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType);
        }

        // Parse variable decl
        auto declarators = ParseVariableDeclarators();

        // Parse trailing ';'
        // NOTE we will keep the ';' for recovery if parsing failed
        if (InParsingMode()) {
            ParsePermissiveSemicolon();
        }

        return CreateAstNode<AstVariableDecl>(beginTokIndex, variableType, declarators);
    }

    auto Parser::ParseInterfaceBlockDecl(size_t beginTokIndex, AstTypeQualifierSeq* quals) -> AstDecl*
    {
        TRACE_PARSER();

        auto declTok = ParseDeclId();

        auto blockBody = ParseStructBody();
        if (InRecoveryMode() || TryConsumeToken(TokenKlass::Semicolon)) {
            return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBody);
        }

        auto declarators = ParseVariableDeclarators();
        if (InParsingMode()) {
            ParsePermissiveSemicolon();
        }

        // FIXME: error on multiple declarators properly
        GLSLD_ASSERT(declarators.size() == 1);
        return CreateAstNode<AstInterfaceBlockDecl>(beginTokIndex, declTok, blockBody, declarators.front());
    }

    auto Parser::ParsePrecisionDecl() -> AstDecl*
    {
        TRACE_PARSER();

        GLSLD_NO_IMPL();
    }

#pragma endregion

#pragma region Parsing Expr

    static auto GetPrefixUnaryOpDesc(TokenKlass klass) -> std::optional<UnaryOp>
    {
        switch (klass) {
        case TokenKlass::Plus:
            return UnaryOp::Identity;
        case TokenKlass::Dash:
            return UnaryOp::Nagate;
        case TokenKlass::Tilde:
            return UnaryOp::BitwiseNot;
        case TokenKlass::Bang:
            return UnaryOp::LogicalNot;
        case TokenKlass::Increment:
            return UnaryOp::PrefixInc;
        case TokenKlass::Decrement:
            return UnaryOp::PrefixDec;
        default:
            return std::nullopt;
        }
    }

    // These are all left-associative operators
    struct BinaryOpDesc
    {
        BinaryOp op;
        int precedence;
    };
    static auto GetBinaryOpDesc(TokenKlass tok) -> std::optional<BinaryOpDesc>
    {
        switch (tok) {
        case TokenKlass::Or:
            return BinaryOpDesc{BinaryOp::LogicalOr, 0};
        case TokenKlass::Xor:
            return BinaryOpDesc{BinaryOp::LogicalXor, 1};
        case TokenKlass::And:
            return BinaryOpDesc{BinaryOp::LogicalAnd, 2};
        case TokenKlass::VerticalBar:
            return BinaryOpDesc{BinaryOp::BitwiseOr, 3};
        case TokenKlass::Caret:
            return BinaryOpDesc{BinaryOp::BitwiseXor, 4};
        case TokenKlass::Ampersand:
            return BinaryOpDesc{BinaryOp::BitwiseAnd, 5};
        case TokenKlass::Equal:
            return BinaryOpDesc{BinaryOp::Equal, 6};
        case TokenKlass::NotEqual:
            return BinaryOpDesc{BinaryOp::NotEqual, 6};
        case TokenKlass::LAngle:
            return BinaryOpDesc{BinaryOp::Less, 7};
        case TokenKlass::RAngle:
            return BinaryOpDesc{BinaryOp::Greater, 7};
        case TokenKlass::LessEq:
            return BinaryOpDesc{BinaryOp::LessEq, 7};
        case TokenKlass::GreaterEq:
            return BinaryOpDesc{BinaryOp::GreaterEq, 7};
        case TokenKlass::LShift:
            return BinaryOpDesc{BinaryOp::ShiftLeft, 8};
        case TokenKlass::RShift:
            return BinaryOpDesc{BinaryOp::ShiftRight, 8};
        case TokenKlass::Plus:
            return BinaryOpDesc{BinaryOp::Plus, 9};
        case TokenKlass::Dash:
            return BinaryOpDesc{BinaryOp::Minus, 9};
        case TokenKlass::Star:
            return BinaryOpDesc{BinaryOp::Mul, 10};
        case TokenKlass::Slash:
            return BinaryOpDesc{BinaryOp::Div, 10};
        case TokenKlass::Percent:
            return BinaryOpDesc{BinaryOp::Modulo, 10};
        default:
            return std::nullopt;
        }
    };

    static auto GetAssignmentOpDesc(TokenKlass klass) -> std::optional<BinaryOp>
    {
        switch (klass) {
        case TokenKlass::Assign:
            return BinaryOp::Assign;
        case TokenKlass::MulAssign:
            return BinaryOp::MulAssign;
        case TokenKlass::DivAssign:
            return BinaryOp::DivAssign;
        case TokenKlass::ModAssign:
            return BinaryOp::ModAssign;
        case TokenKlass::AddAssign:
            return BinaryOp::AddAssign;
        case TokenKlass::SubAssign:
            return BinaryOp::SubAssign;
        case TokenKlass::LShiftAssign:
            return BinaryOp::LShiftAssign;
        case TokenKlass::RShiftAssign:
            return BinaryOp::RShiftAssign;
        case TokenKlass::AndAssign:
            return BinaryOp::AndAssign;
        case TokenKlass::XorAssign:
            return BinaryOp::XorAssign;
        case TokenKlass::OrAssign:
            return BinaryOp::OrAssign;
        default:
            return std::nullopt;
        }
    }

    auto Parser::ParseExpr() -> AstExpr*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseAssignmentExpr();

        while (TryConsumeToken(TokenKlass::Comma)) {
            GLSLD_ASSERT(InParsingMode());
            auto rhsResult = ParseAssignmentExpr();

            lhsResult = CreateAstNode<AstBinaryExpr>(beginTokIndex, BinaryOp::Comma, lhsResult, rhsResult);
        }

        // NOTE an expression parser could consume zero token and return an error expr.
        // If so, we explicit put the parser into recovery mode.
        if (GetTokenIndex() == beginTokIndex) {
            EnterRecoveryMode();
        }

        return lhsResult;
    }

    auto Parser::ParseAssignmentExpr() -> AstExpr*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseUnaryExpr();

        auto opDesc = GetAssignmentOpDesc(PeekToken().klass);
        if (opDesc) {
            ConsumeToken();
            GLSLD_ASSERT(InParsingMode());
            auto rhsResult = ParseAssignmentExpr();
            return CreateAstNode<AstBinaryExpr>(beginTokIndex, *opDesc, lhsResult, rhsResult);
        }
        else {
            return ParseBinaryOrConditionalExpr(beginTokIndex, lhsResult);
        }
    }

    auto Parser::ParseBinaryOrConditionalExpr(size_t beginTokIndex, AstExpr* firstTerm) -> AstExpr*
    {
        TRACE_PARSER();

        auto predicateOrExprResult = ParseBinaryExpr(beginTokIndex, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExprResult;
        }

        auto positveExprResult = ParseExpr();

        if (!TryConsumeToken(TokenKlass::Colon)) {
            if (InRecoveryMode()) {
                // we cannot infer a ':' in recovery mode, so just return early
                auto tokIndex = GetTokenIndex();
                return CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult, positveExprResult,
                                                    CreateRangedAstNode<AstErrorExpr>(tokIndex, tokIndex));
            }

            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto negativeExprResult = ParseAssignmentExpr();

        return CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult, positveExprResult,
                                            negativeExprResult);
    }

    auto Parser::ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*
    {
        TRACE_PARSER();

        auto lhs = firstTerm;
        while (true) {
            if (InRecoveryMode()) {
                // NOTE `lhs` is either unary_expr or binary expr, meaning we have incoming 'EOF' or ';' or '}'.
                // Obviously, we cannot parse anything so just leave early
                break;
            }

            auto opDesc = GetBinaryOpDesc(PeekToken().klass);
            if (!(opDesc.has_value() && opDesc->precedence >= minPrecedence)) {
                break;
            }

            // Consume the operator
            ConsumeToken();

            auto rhsBeginTokIndex = GetTokenIndex();
            auto rhs              = ParseUnaryExpr();

            while (true) {
                auto opDescNext = GetBinaryOpDesc(PeekToken().klass);
                if (!(opDescNext.has_value() && opDescNext->precedence > opDesc->precedence)) {
                    break;
                }

                rhs = ParseBinaryExpr(rhsBeginTokIndex, rhs, opDesc->precedence + 1);
            }

            lhs = CreateAstNode<AstBinaryExpr>(beginTokIndex, opDesc->op, lhs, rhs);
            // Since all binary operators are left-associative, the `beginTokIndex` should stay the same
        }

        return lhs;
    }

    auto Parser::ParseUnaryExpr() -> AstExpr*
    {
        TRACE_PARSER();

        auto opDesc = GetPrefixUnaryOpDesc(PeekToken().klass);
        if (opDesc) {
            auto beginTokIndex = GetTokenIndex();
            ConsumeToken();
            // TODO: avoid recursion
            auto childExpr = ParseUnaryExpr();
            return CreateAstNode<AstUnaryExpr>(beginTokIndex, *opDesc, childExpr);
        }
        else {
            return ParsePostfixExpr();
        }
    }

    auto Parser::ParsePostfixExpr() -> AstExpr*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse primary_expr or constructor call
        AstExpr* result = nullptr;
        if (GetBuiltinType(PeekToken())) {
            auto tok = PeekToken();
            ConsumeToken();
            result = CreateAstNode<AstNameAccessExpr>(beginTokIndex, tok);
        }
        else {
            // FIXME: construct like 'S[2](1, 2)' where S[2] is a type could fall into this case.
            //        handle correctly!
            result = ParsePrimaryExpr();
        }

        // FIXME: implement this
        bool parsedPostfix = true;
        while (InParsingMode() && parsedPostfix) {
            parsedPostfix = false;
            switch (PeekToken().klass) {
            case TokenKlass::LParen:
            {
                // function call
                // FIXME: constructor call is "type_spec '(' ??? ')'"
                auto args = ParseFunctionArgumentList();
                result =
                    CreateAstNode<AstInvokeExpr>(beginTokIndex, InvocationType::FunctionCall, result, std::move(args));
                break;
            }
            case TokenKlass::LBracket:
            {
                // indexing access
                // FIXME: impl this
                EnterRecoveryMode();
                // auto args = ParseArraySpec();
                // result    = ParseResult<AstExpr*>(
                //     args.Success(),
                //     CreateAstNode<AstInvokeExpr>(beginTokIndex, InvocationType::Indexing, result.Move(),
                //     args.Move()));
                break;
            }
            case TokenKlass::Dot:
            {
                // name access
                // FIXME: recovery?
                ConsumeToken();
                auto accessName = ParseDeclId();
                result          = CreateAstNode<AstNameAccessExpr>(beginTokIndex, result, accessName);
                break;
            }
            case TokenKlass::Increment:
            {
                // postfix inc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::PostfixInc, result);
                break;
            }
            case TokenKlass::Decrement:
            {
                // postfix dnc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, UnaryOp::PostfixDec, result);
                break;
            }
            default:
                break;
            }
        }

        return result;
    }

    auto Parser::ParsePrimaryExpr() -> AstExpr*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        auto tok = PeekToken();
        switch (PeekToken().klass) {
        case TokenKlass::Identifier:
            // variable name
            ConsumeToken();
            return CreateAstNode<AstNameAccessExpr>(beginTokIndex, tok);
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
        case TokenKlass::K_true:
        case TokenKlass::K_false:
            // constant
            // TODO: save more info?
            ConsumeToken();
            return CreateAstNode<AstConstantExpr>(beginTokIndex, tok);
        case TokenKlass::LParen:
            // expr in wrapped parens
            return ParseParenWrappedExpr();
        default:
            // error, but we see this as a successful parse
            ReportError("expect identifier or constant");
            return CreateRangedAstNode<AstErrorExpr>(beginTokIndex, beginTokIndex);
        }
    }

    auto Parser::ParseParenWrappedExpr() -> AstExpr*
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LParen);

        if (TryConsumeToken(TokenKlass::RParen)) {
            return CreateErrorExpr();
        }

        auto result = ParseExpr();
        ParseClosingParen();

        return result;
    };

#pragma endregion

#pragma region Parsing Stmt

    auto Parser::ParseStmt() -> AstStmt*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::LBrace:
            // compound stmt
            return ParseCompoundStmt();
        case TokenKlass::K_if:
            // selection stmt
            return ParseSelectionStmt();
        case TokenKlass::K_while:
            // iteration stmt (while)
            return ParseWhileStmt();
        case TokenKlass::K_for:
            // iteration stmt (for)
            return ParseForStmt();
        case TokenKlass::K_switch:
            // switch stmt
            return ParseSwitchStmt();
        case TokenKlass::K_break:
        case TokenKlass::K_continue:
        case TokenKlass::K_discard:
        case TokenKlass::K_return:
            // jump stmt
            return ParseJumpStmt();
        case TokenKlass::K_case:
        case TokenKlass::K_default:
            // labeled stmt
            GLSLD_NO_IMPL();
        default:
            // expression/declaration stmt
            return ParseDeclOrExprStmt();
        }
    }

    auto Parser::ParseStmtAndTryRecover() -> AstStmt*
    {
        auto stmtResult = ParseStmt();
        if (InRecoveryMode()) {
            RecoverFromError(RecoveryMode::Semi);
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                ExitRecoveryMode();
            }
        }

        return stmtResult;
    }

    auto Parser::ParseCompoundStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::LBrace);

        auto beginTokIndex = GetTokenIndex();

        // consume "{"
        ConsumeTokenAssert(TokenKlass::LBrace);

        std::vector<AstStmt*> children;
        while (!Eof() && !TryTestToken(TokenKlass::RBrace)) {
            children.push_back(ParseStmtAndTryRecover());
        }

        if (Eof()) {
            ReportError("unexpected EOF");
        }
        else {
            ConsumeTokenAssert(TokenKlass::RBrace);
        }

        return CreateAstNode<AstCompoundStmt>(beginTokIndex, std::move(children));
    }

    auto Parser::ParseSelectionStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::K_if);

        auto beginTokIndex = GetTokenIndex();

        // parse "K_if"
        ConsumeTokenAssert(TokenKlass::K_if);

        // parse predicate condition
        auto predicateExprResult = ParseParenWrappedExpr();
        if (InRecoveryMode()) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult, CreateErrorStmt());
        }

        // parse positive branch
        auto positiveStmtResult = ParseStmt();
        // TODO: error handling

        if (!TryConsumeToken(TokenKlass::K_else)) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult, positiveStmtResult);
        }

        // TODO: check with if?
        auto negativeStmtResult = ParseStmt();
        // TODO: error handling
        return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult, positiveStmtResult, negativeStmtResult);
    }

    auto Parser::ParseForStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::K_for);

        GLSLD_ASSERT(TryTestToken(TokenKlass::K_for));

        auto beginTokIndex = GetTokenIndex();
        ConsumeToken();

        if (!TryConsumeToken(TokenKlass::LParen)) {
            EnterRecoveryMode();
            return CreateAstNode<AstForStmt>(beginTokIndex, CreateErrorStmt(), CreateErrorStmt(), CreateErrorStmt(),
                                             CreateErrorStmt());
        }

        // FIXME: must be simple stmt (non-compound)
        auto initClause = ParseStmt();
        if (InRecoveryMode()) {
            return CreateAstNode<AstForStmt>(beginTokIndex, initClause, CreateErrorStmt(), CreateErrorStmt(),
                                             CreateErrorStmt());
        }

        // FIXME: must be simple stmt (non-compound)
        auto testClause = ParseStmt();
        if (InRecoveryMode()) {
            return CreateAstNode<AstForStmt>(beginTokIndex, initClause, testClause, CreateErrorStmt(),
                                             CreateErrorStmt());
        }

        // FIXME: must be simple stmt (non-compound)
        auto proceedClause = ParseStmt();
        ParseClosingParen();
        if (InRecoveryMode()) {
            return CreateAstNode<AstForStmt>(beginTokIndex, initClause, testClause, proceedClause, CreateErrorStmt());
        }

        auto loopBody = ParseStmt();
        return CreateAstNode<AstForStmt>(beginTokIndex, initClause, proceedClause, proceedClause, loopBody);
    }

    auto Parser::ParseWhileStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::K_while);

        auto beginTokIndex = GetTokenIndex();

        // parse 'K_while'
        ConsumeTokenAssert(TokenKlass::K_while);

        // parse predicate condition
        auto predicateExpr = ParseParenWrappedExpr();
        if (InRecoveryMode()) {
            return CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, CreateErrorStmt());
        }

        // parse loop body
        auto bodyStmt = ParseStmt();
        return CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, bodyStmt);
    }

    auto Parser::ParseSwitchStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::K_switch);
        GLSLD_NO_IMPL();
    }

    auto Parser::ParseJumpStmt() -> AstStmt*
    {
        TRACE_PARSER(TokenKlass::K_break, TokenKlass::K_continue, TokenKlass::K_discard, TokenKlass::K_return);

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::K_break:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Break);
        case TokenKlass::K_continue:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Continue);
        case TokenKlass::K_discard:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateAstNode<AstJumpStmt>(beginTokIndex, JumpType::Discard);
        case TokenKlass::K_return:
            ConsumeToken();
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                return CreateAstNode<AstReturnStmt>(beginTokIndex);
            }
            else {
                // FIXME: recovery mode?
                auto returnExprResult = ParseExpr();

                if (InParsingMode()) {
                    ParsePermissiveSemicolon();
                }
                return CreateAstNode<AstReturnStmt>(beginTokIndex, returnExprResult);
            }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto Parser::ParseExprStmt() -> AstStmt*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto exprResult    = ParseExpr();

        // if failed, we keep the ';' for error recovery
        if (InParsingMode()) {
            ParsePermissiveSemicolon();
        }

        return CreateAstNode<AstExprStmt>(beginTokIndex, exprResult);
    }

    auto Parser::ParseDeclOrExprStmt() -> AstStmt*
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        switch (PeekToken().klass) {
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
        case TokenKlass::K_true:
        case TokenKlass::K_false:
        case TokenKlass::LParen:
            return ParseExprStmt();
        case TokenKlass::Identifier:
            if (!TryTestToken(TokenKlass::Identifier, 1)) {
                return ParseExprStmt();
            }
            [[fallthrough]];
        default:
            // FIXME: `vec3(0);` is being treated as function declaration but it's a constructor call expr
            return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclAndTryRecover());
        }
    }

#pragma endregion
} // namespace glsld