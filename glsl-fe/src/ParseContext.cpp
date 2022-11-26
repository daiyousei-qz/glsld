#include "ParseContext.h"

namespace glsld
{
#pragma region Parsing Decl

#pragma endregion

#pragma region Parsing Expr

    static auto GetPrefixUnaryOpDesc(TokenKlass klass) -> std::optional<ExprOp>
    {
        switch (klass) {
        case TokenKlass::Plus:
            return ExprOp::Plus;
        case TokenKlass::Dash:
            return ExprOp::Minus;
        case TokenKlass::Tilde:
            return ExprOp::BitwiseNot;
        case TokenKlass::Bang:
            return ExprOp::LogicalNot;
        case TokenKlass::Increment:
            return ExprOp::PrefixInc;
        case TokenKlass::Decrement:
            return ExprOp::PrefixDec;
        default:
            return std::nullopt;
        }
    }

    // These are all left-associative operators
    struct BinaryOpDesc
    {
        ExprOp op;
        int precedence;
    };
    static auto GetBinaryOpDesc(TokenKlass tok) -> std::optional<BinaryOpDesc>
    {
        switch (tok) {
        case TokenKlass::Or:
            return BinaryOpDesc{ExprOp::LogicalOr, 0};
        case TokenKlass::Xor:
            return BinaryOpDesc{ExprOp::LogicalXor, 1};
        case TokenKlass::And:
            return BinaryOpDesc{ExprOp::LogicalAnd, 2};
        case TokenKlass::VerticalBar:
            return BinaryOpDesc{ExprOp::BitwiseOr, 3};
        case TokenKlass::Caret:
            return BinaryOpDesc{ExprOp::BitwiseXor, 4};
        case TokenKlass::Ampersand:
            return BinaryOpDesc{ExprOp::BitwiseAnd, 5};
        case TokenKlass::Equal:
            return BinaryOpDesc{ExprOp::Equal, 6};
        case TokenKlass::NotEqual:
            return BinaryOpDesc{ExprOp::NotEqual, 6};
        case TokenKlass::LAngle:
            return BinaryOpDesc{ExprOp::Less, 7};
        case TokenKlass::RAngle:
            return BinaryOpDesc{ExprOp::Greater, 7};
        case TokenKlass::LessEq:
            return BinaryOpDesc{ExprOp::LessEq, 7};
        case TokenKlass::GreaterEq:
            return BinaryOpDesc{ExprOp::GreaterEq, 7};
        case TokenKlass::LShift:
            return BinaryOpDesc{ExprOp::ShiftLeft, 8};
        case TokenKlass::RShift:
            return BinaryOpDesc{ExprOp::ShiftRight, 8};
        case TokenKlass::Plus:
            return BinaryOpDesc{ExprOp::Plus, 9};
        case TokenKlass::Dash:
            return BinaryOpDesc{ExprOp::Minus, 9};
        case TokenKlass::Star:
            return BinaryOpDesc{ExprOp::Mul, 10};
        case TokenKlass::Slash:
            return BinaryOpDesc{ExprOp::Div, 10};
        case TokenKlass::Percent:
            return BinaryOpDesc{ExprOp::Modulo, 10};
        default:
            return std::nullopt;
        }
    };

    static auto GetAssignmentOpDesc(TokenKlass klass) -> std::optional<ExprOp>
    {
        switch (klass) {
        case TokenKlass::Assign:
            return ExprOp::Assign;
        case TokenKlass::MulAssign:
            return ExprOp::MulAssign;
        case TokenKlass::DivAssign:
            return ExprOp::DivAssign;
        case TokenKlass::ModAssign:
            return ExprOp::ModAssign;
        case TokenKlass::AddAssign:
            return ExprOp::AddAssign;
        case TokenKlass::SubAssign:
            return ExprOp::SubAssign;
        case TokenKlass::LShiftAssign:
            return ExprOp::LShiftAssign;
        case TokenKlass::RShiftAssign:
            return ExprOp::RShiftAssign;
        case TokenKlass::AndAssign:
            return ExprOp::AndAssign;
        case TokenKlass::XorAssign:
            return ExprOp::XorAssign;
        case TokenKlass::OrAssign:
            return ExprOp::OrAssign;
        default:
            return std::nullopt;
        }
    }

    auto ParseContext::ParseExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseAssignmentExpr();

        while (TryConsumeToken(TokenKlass::Comma)) {
            GLSLD_ASSERT(lhsResult.Success());
            auto rhsResult = ParseAssignmentExpr();

            lhsResult = {rhsResult.Success(),
                         CreateAstNode<AstBinaryExpr>(beginTokIndex, ExprOp::Comma, lhsResult.Get(), rhsResult.Get())};
        }

        // NOTE an expression parser could consume zero token and return an error expr.
        // If so, we explicit put the parser into recovery mode.
        if (GetTokenIndex() == beginTokIndex) {
            return {false, lhsResult.Get()};
        }

        return lhsResult;
    }

    auto ParseContext::ParseAssignmentExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto lhsResult     = ParseUnaryExpr();

        auto opDesc = GetAssignmentOpDesc(PeekToken().klass);
        if (opDesc) {
            ConsumeToken();
            GLSLD_ASSERT(lhsResult.Success());
            auto rhsResult = ParseAssignmentExpr();
            return {rhsResult.Success(),
                    CreateAstNode<AstBinaryExpr>(beginTokIndex, *opDesc, lhsResult.Move(), rhsResult.Move())};
        }
        else {
            return ParseBinaryOrConditionalExpr(beginTokIndex, lhsResult);
        }
    }

    auto ParseContext::ParseBinaryOrConditionalExpr(size_t beginTokIndex, ParseResult<AstExpr*> firstTerm)
        -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto predicateOrExprResult = ParseBinaryExpr(beginTokIndex, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExprResult;
        }

        auto positveExprResult = ParseExpr();

        if (!positveExprResult.Success()) {
            auto tokIndex     = GetTokenIndex();
            auto negativeExpr = CreateRangedAstNode<AstErrorExpr>(tokIndex, tokIndex);
            return {false, CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult.Move(),
                                                        positveExprResult.Move(), negativeExpr)};
        }

        if (!TryConsumeToken(TokenKlass::Colon)) {
            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto negativeExprResult = ParseAssignmentExpr();

        return {negativeExprResult.Success(),
                CreateAstNode<AstSelectExpr>(beginTokIndex, predicateOrExprResult.Move(), positveExprResult.Move(),
                                             negativeExprResult.Move())};
    }

    auto ParseContext::ParseBinaryExpr(size_t beginTokIndex, ParseResult<AstExpr*> firstTerm, int minPrecedence)
        -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto lhs = firstTerm;
        while (true) {
            if (!lhs.Success()) {
                // NOTE lhs is either unary_expr or binary expr, meaning we have incoming 'EOF' or ';' or '}'.
                // Obviously, we cannot parse anything so just quit early
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

            lhs = {rhs.Success(), CreateAstNode<AstBinaryExpr>(beginTokIndex, opDesc->op, lhs.Get(), rhs.Get())};
            // Since all binary operators are left-associative, the `beginTokIndex` should stay the same
        }

        return lhs;
    }

    auto ParseContext::ParseUnaryExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto opDesc = GetPrefixUnaryOpDesc(PeekToken().klass);
        if (opDesc) {
            auto beginTokIndex = GetTokenIndex();
            ConsumeToken();
            // TODO: avoid recursion
            auto childExpr = ParseUnaryExpr();
            return ParseResult<AstExpr*>(childExpr.Success(),
                                         CreateAstNode<AstUnaryExpr>(beginTokIndex, *opDesc, childExpr.Move()));
        }
        else {
            return ParsePostfixExpr();
        }
    }

    auto ParseContext::ParsePostfixExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();

        // Parse primary_expr or constructor call
        ParseResult<AstExpr*> result{false, nullptr};
        if (GetBuiltinType(PeekToken())) {
            result = ParseConstructorCall();
        }
        else {
            // FIXME: construct like 'S[2](1, 2)' where S[2] is a type could fall into this case.
            //        handle correctly!
            result = ParsePrimaryExpr();
        }

        // FIXME: implement this
        bool parsedPostfix = true;
        while (!result.Success() && parsedPostfix) {
            parsedPostfix = false;
            switch (PeekToken().klass) {
            case TokenKlass::LParen:
            {
                // function call
                // FIXME: constructor call is "type_spec '(' ??? ')'"
                auto args = ParseFunctionArgumentList();
                result    = ParseResult<AstExpr*>(args.Success(),
                                               CreateAstNode<AstInvokeExpr>(beginTokIndex, InvocationType::FunctionCall,
                                                                            result.Move(), args.Move()));
                break;
            }
            case TokenKlass::LBracket:
            {
                // indexing access
                // FIXME: impl this
                GLSLD_NO_IMPL();
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
                result          = CreateAstNode<AstNameAccessExpr>(beginTokIndex, result.Move(), accessName.Move());
            }

            break;
            case TokenKlass::Increment:
            {
                // postfix inc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, ExprOp::PostfixInc, result.Move());
                break;
            }
            case TokenKlass::Decrement:
            {
                // postfix dnc
                ConsumeToken();
                result = CreateAstNode<AstUnaryExpr>(beginTokIndex, ExprOp::PostfixDec, result.Move());
                break;
            }
            default:
                break;
            }
        }

        return result;
    }

    auto ParseContext::ParsePrimaryExpr() -> ParseResult<AstExpr*>
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
            return CreateAstNode<AstConstantExpr>(beginTokIndex, tok.text);
        case TokenKlass::LParen:
            // expr in wrapped parens
            return ParseParenWrappedExpr();
        default:
            // error, but we see this as a successful parse
            ReportError("expect identifier or constant");
            return CreateRangedAstNode<AstErrorExpr>(beginTokIndex, beginTokIndex);
        }
    }

    auto ParseContext::ParseParenWrappedExpr() -> ParseResult<AstExpr*>
    {
        TRACE_PARSER();

        ConsumeTokenAssert(TokenKlass::LParen);

        if (TryConsumeToken(TokenKlass::RParen)) {
            return CreateErrorExpr();
        }

        auto result = ParseExpr();

        // consume ")"
        bool closed = TryConsumeToken(TokenKlass::RParen);
        if (!closed) {
            ReportError("expect ')'");

            RecoverFromError(RecoveryMode::Paren);
            closed = TryConsumeToken(TokenKlass::RParen);
        }

        return {closed, result.Move()};
    };

#pragma endregion

#pragma region Parsing Stmt

    auto ParseContext::ParseStmt() -> ParseResult<AstStmt*>
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

    auto ParseContext::ParseCompoundStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::LBrace);

        auto beginTokIndex = GetTokenIndex();

        // consume "{"
        ConsumeTokenAssert(TokenKlass::LBrace);

        std::vector<AstExpr*> children;
        while (!Eof() && !TryTestToken(TokenKlass::RBrace)) {
            children.push_back(ParseStmtNoRecovery().Get());
        }

        if (Eof()) {
            ReportError("unexpected EOF");
        }
        else {
            ConsumeTokenAssert(TokenKlass::RBrace);
        }

        return {!Eof(), CreateAstNode<AstCompoundStmt>(beginTokIndex, std::move(children))};
    }

    auto ParseContext::ParseSelectionStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_if);

        auto beginTokIndex = GetTokenIndex();

        // parse "K_if"
        ConsumeTokenAssert(TokenKlass::K_if);

        // parse predicate condition
        auto predicateExprResult = ParseParenWrappedExpr();
        if (!predicateExprResult.Success()) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult.Get(), CreateErrorStmt());
        }

        // parse positive branch
        auto positiveStmtResult = ParseStmt();
        // TODO: error handling

        if (!TryConsumeToken(TokenKlass::K_else)) {
            return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult.Get(), positiveStmtResult.Get());
        }

        // TODO: check with if?
        auto negativeStmtResult = ParseStmt();
        // TODO: error handling
        return CreateAstNode<AstIfStmt>(beginTokIndex, predicateExprResult.Get(), positiveStmtResult.Get(),
                                        negativeStmtResult.Get());
    }

    auto ParseContext::ParseForStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_for);

        GLSLD_NO_IMPL();
    }

    auto ParseContext::ParseWhileStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_while);

        auto beginTokIndex = GetTokenIndex();

        // parse 'K_while'
        ConsumeTokenAssert(TokenKlass::K_while);

        // parse predicate condition
        auto predicateExpr = ParseParenWrappedExpr();
        if (!predicateExpr.Success()) {
            return {false, CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr.Get(), CreateErrorStmt())};
        }

        // parse loop body
        auto bodyStmt = ParseStmt();
        return {bodyStmt.Success(), CreateAstNode<AstWhileStmt>(beginTokIndex, predicateExpr.Get(), bodyStmt.Get())};
    }

    auto ParseContext::ParseSwitchStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER(TokenKlass::K_switch);
        GLSLD_NO_IMPL();
    }

    auto ParseContext::ParseJumpStmt() -> ParseResult<AstStmt*>
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

                if (returnExprResult.Success()) {
                    ParsePermissiveSemicolon();
                }
                return {returnExprResult.Success(),
                        CreateAstNode<AstReturnStmt>(beginTokIndex, returnExprResult.Get())};
            }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto ParseContext::ParseExprStmt() -> ParseResult<AstStmt*>
    {
        TRACE_PARSER();

        auto beginTokIndex = GetTokenIndex();
        auto exprResult    = ParseExpr();

        // if failed, we keep the ';' for error recovery
        if (exprResult.Success()) {
            ParsePermissiveSemicolon();
        }

        return {exprResult.Success(), CreateAstNode<AstExprStmt>(beginTokIndex, exprResult.Get())};
    }

    auto ParseContext::ParseDeclOrExprStmt() -> ParseResult<AstStmt*>
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
            return CreateAstNode<AstDeclStmt>(beginTokIndex, ParseDeclNoRecovery().Get());
        }
    }

#pragma endregion
} // namespace glsld