#include "ParseContext.h"

namespace glsld
{
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

    // Entry function to parse any expression AST.
    // This function always return a valid AstExpr*. In presence of a parsing error, an AstErrorExpr* will be used to
    // replace the expression failed to be constructed.
    auto ParseContext::ParseExpr() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();
        auto lhs           = ParseAssignmentExpr();

        while (TryConsumeToken(TokenKlass::Comma)) {
            auto rhs = ParseAssignmentExpr();

            lhs = CreateRangedAstNode<AstBinaryExpr>(beginTokIndex, ExprOp::Comma, lhs, rhs);
        }

        return lhs;
    }

    auto ParseContext::ParseAssignmentExpr() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();
        auto lhs           = ParseUnaryExpr();

        auto opDesc = GetAssignmentOpDesc(PeekToken().klass);
        if (opDesc) {
            ConsumeToken();
            auto rhs = ParseAssignmentExpr();
            return CreateRangedAstNode<AstBinaryExpr>(beginTokIndex, *opDesc, lhs, rhs);
        }
        else {
            return ParseBinaryOrConditionalExpr(beginTokIndex, lhs);
        }
    }

    auto ParseContext::ParseBinaryOrConditionalExpr(size_t beginTokIndex, AstExpr* firstTerm) -> AstExpr*
    {
        auto predicateOrExpr = ParseBinaryExpr(beginTokIndex, firstTerm, 0);

        if (!TryConsumeToken(TokenKlass::Question)) {
            return predicateOrExpr;
        }

        auto positveExpr = ParseExpr();
        if (!TryConsumeToken(TokenKlass::Colon)) {
            ReportError("expecting ':'");
        }

        // Even if ':' is missing, we'll continue parsing the nenative part
        auto negativeExpr = ParseAssignmentExpr();

        return CreateRangedAstNode<AstSelectExpr>(beginTokIndex, predicateOrExpr, positveExpr, negativeExpr);
    }

    auto ParseContext::ParseBinaryExpr(size_t beginTokIndex, AstExpr* firstTerm, int minPrecedence) -> AstExpr*
    {
        auto lhs = firstTerm;
        while (true) {
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

            lhs = CreateRangedAstNode<AstBinaryExpr>(beginTokIndex, opDesc->op, lhs, rhs);

            // Since all binary operators are left-associative, the `beginTokIndex` should stay the same
        }

        return lhs;
    }

    auto ParseContext::ParseUnaryExpr() -> AstExpr*
    {
        auto opDesc = GetPrefixUnaryOpDesc(PeekToken().klass);
        if (opDesc) {
            auto beginTokIndex = GetTokenIndex();
            ConsumeToken();
            // TODO: avoid recursion
            auto childExpr = ParseUnaryExpr();
            return CreateRangedAstNode<AstUnaryExpr>(beginTokIndex, *opDesc, childExpr);
        }
        else {
            return ParsePostfixExpr();
        }
    }

    auto ParseContext::ParsePostfixExpr() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();
        auto result        = ParsePrimaryExpr();

        // FIXME: implement this
        bool error = false;
        while (!error) {
            bool parsedPostfix = false;
            switch (PeekToken().klass) {
            case TokenKlass::LParen:
                // function call
                break;
            case TokenKlass::LBracket:
                // indexing access
                break;
            case TokenKlass::Dot:
                // name access
                ConsumeToken();
                if (TryTestToken(TokenKlass::Identifier)) {
                    // FIXME: handle .length()
                    auto accessName = PeekToken().text;
                    ConsumeToken();
                    result = CreateRangedAstNode<AstVarAccessExpr>(beginTokIndex, result, accessName);
                }
                else {
                    error = true;
                }
                break;
            case TokenKlass::Increment:
                // postfix inc
                ConsumeToken();
                result = CreateRangedAstNode<AstUnaryExpr>(beginTokIndex, ExprOp::PostfixInc, result);
                break;
            case TokenKlass::Decrement:
                // postfix dnc
                ConsumeToken();
                result = CreateRangedAstNode<AstUnaryExpr>(beginTokIndex, ExprOp::PostfixDec, result);
                break;
            default:
                break;
            }

            if (!parsedPostfix) {
                break;
            }
        }

        if (error) {
            // FIXME: could be GlobalSemi, since this could be a part of initializer.
            RecoverFromError(RecoveryMode::LocalSemi);
        }
        return result;
    }

    auto ParseContext::ParsePrimaryExpr() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();

        auto tokText = PeekToken().text;
        switch (PeekToken().klass) {
        case TokenKlass::Identifier:
            // variable name
            ConsumeToken();
            return CreateRangedAstNode<AstVarAccessExpr>(beginTokIndex, tokText);
        case TokenKlass::IntegerConstant:
        case TokenKlass::FloatConstant:
            // integer/float constant
            ConsumeToken();
            return CreateRangedAstNode<AstConstantExpr>(beginTokIndex, tokText);
        case TokenKlass::K_true:
            // bool constant (true)
            ConsumeToken();
            return CreateRangedAstNode<AstConstantExpr>(beginTokIndex, tokText);
        case TokenKlass::K_false:
            // bool constant (false)
            ConsumeToken();
            return CreateRangedAstNode<AstConstantExpr>(beginTokIndex, tokText);
        case TokenKlass::LParen:
            // expr in wrapped parens
            return ParseParenWrappedExpr();
        default:
            // error
            return CreateRangedAstNode2<AstErrorExpr>(beginTokIndex, beginTokIndex);
        }
    }

    auto ParseContext::ParseParenWrappedExpr() -> AstExpr*
    {
        ConsumeTokenAssert(TokenKlass::LParen);

        auto result = ParseExpr();

        // consume ")"
        if (!TryConsumeToken(TokenKlass::RParen)) {
            ReportError("expect ')'");
            RecoverFromError(RecoveryMode::Paren);
        }

        return result;
    };

#pragma endregion

#pragma region Parsing Stmt

    auto ParseContext::ParseStmt() -> AstExpr*
    {
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
        case TokenKlass::Semicolon:
            // expression stmt (empty)
            ConsumeToken();
            // TODO: We are using AstExprStmt for now. Should have a dedicated type?
            return CreateRangedAstNode<AstExprStmt>(beginTokIndex, nullptr);
        case TokenKlass::K_break:
        case TokenKlass::K_continue:
        case TokenKlass::K_discard:
        case TokenKlass::K_return:
            // jump stmt
            return ParseJumpStmt();
        default:
            // expression/declaration stmt
            return ParseDeclOrExprStmt();
        }
    }

    auto ParseContext::ParseCompoundStmt() -> AstCompoundStmt*
    {
        auto beginTokIndex = GetTokenIndex();

        // consume "{"
        ConsumeTokenAssert(TokenKlass::LBrace);

        std::vector<AstExpr*> children;

        // Handle empty compound statement
        if (TryConsumeToken(TokenKlass::RBrace)) {
            return CreateRangedAstNode<AstCompoundStmt>(beginTokIndex, std::move(children));
        }

        while (!Eof()) {
            auto stmt = ParseStmt();
            GLSLD_ASSERT(stmt != nullptr);

            children.push_back(stmt);

            if (TryTestToken(TokenKlass::RBrace)) {
                break;
            }
        }

        if (Eof()) {
            ReportError("unexpected EOF");
        }
        else {
            ConsumeTokenAssert(TokenKlass::RBrace);
        }

        return CreateRangedAstNode<AstCompoundStmt>(beginTokIndex, std::move(children));
    }

    auto ParseContext::ParseSelectionStmt() -> AstIfStmt*
    {
        auto beginTokIndex = GetTokenIndex();

        // consume "if"
        ConsumeTokenAssert(TokenKlass::K_if);

        auto predicateExpr = ParseParenWrappedExpr();
        // TODO: error handling
        auto positiveStmt = ParseStmt();
        // TODO: error handling

        if (PeekToken().klass != TokenKlass::K_else) {
            return CreateRangedAstNode<AstIfStmt>(beginTokIndex, predicateExpr, positiveStmt);
        }

        ConsumeTokenAssert(TokenKlass::K_else);

        // TODO: check with if?
        auto negativeStmt = ParseStmt();
        // TODO: error handling
        return CreateRangedAstNode<AstIfStmt>(beginTokIndex, predicateExpr, positiveStmt, negativeStmt);
    }

    auto ParseContext::ParseForStmt() -> AstForStmt*
    {
        GLSLD_NO_IMPL();
    }

    auto ParseContext::ParseWhileStmt() -> AstWhileStmt*
    {
        auto beginTokIndex = GetTokenIndex();

        ConsumeTokenAssert(TokenKlass::K_while);

        auto predicateExpr = ParseParenWrappedExpr();
        // TODO: error handling
        auto bodyStmt = ParseStmt();
        // TODO: error handling

        return CreateRangedAstNode<AstWhileStmt>(beginTokIndex, predicateExpr, bodyStmt);
    }

    auto ParseContext::ParseSwitchStmt() -> AstSwitchStmt*
    {
        ConsumeTokenAssert(TokenKlass::K_switch);

        GLSLD_NO_IMPL();
    }

    auto ParseContext::ParseJumpStmt() -> AstExpr*
    {
        auto beginTokIndex = GetTokenIndex();

        switch (PeekToken().klass) {
        case TokenKlass::K_break:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateRangedAstNode<AstJumpStmt>(beginTokIndex, JumpType::Break);
        case TokenKlass::K_continue:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateRangedAstNode<AstJumpStmt>(beginTokIndex, JumpType::Continue);
        case TokenKlass::K_discard:
            ConsumeToken();
            ParsePermissiveSemicolon();
            return CreateRangedAstNode<AstJumpStmt>(beginTokIndex, JumpType::Discard);
        case TokenKlass::K_return:
            ConsumeToken();
            if (TryConsumeToken(TokenKlass::Semicolon)) {
                return CreateRangedAstNode<AstReturnStmt>(beginTokIndex);
            }
            else {
                auto returnValue = ParseExpr();

                ParsePermissiveSemicolon();
                return CreateRangedAstNode<AstReturnStmt>(beginTokIndex, returnValue);
            }
        default:
            GLSLD_UNREACHABLE();
        }
    }

    auto ParseContext::ParseDeclOrExprStmt() -> AstStmt*
    {
        auto beginTokIndex = GetTokenIndex();

        // FIXME: how to handle decl?
        auto expr = ParseExpr();

        ParsePermissiveSemicolon();
        return CreateRangedAstNode<AstExprStmt>(beginTokIndex, expr);
    }

#pragma endregion
} // namespace glsld