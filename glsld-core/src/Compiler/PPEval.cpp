#include "Compiler/PPEval.h"

#include "Compiler/PPTokenScanner.h"
#include "Language/ConstValue.h"

namespace glsld
{
    auto PPExprEvaluator::Evaluate(ArrayView<PPToken> tokens) -> std::optional<int64_t>
    {
        bool parsedLhs = false;

        PPTokenScanner scanner{tokens};
        while (!scanner.CursorAtEnd()) {
            auto token = scanner.ConsumeToken();

            if (parsedLhs) {
                if (token.klass == TokenKlass::RParen) {
                    while (!operatorStack.empty() && operatorStack.back().op != PPOperator::LParen) {
                        if (!EvalOperatorOnTop()) {
                            // FIXME: report error, failed to evaluate the expression somehow
                            return std::nullopt;
                        }
                    }

                    if (!operatorStack.empty()) {
                        GLSLD_ASSERT(operatorStack.back().op == PPOperator::LParen);
                        operatorStack.pop_back();
                    }
                    else {
                        // FIXME: report error, mismatched parenthesis
                        return std::nullopt;
                    }
                    parsedLhs = true;
                }
                else if (auto ppOpInfo = ParseBinaryPPOperator(token)) {
                    if (!PushOperator(*ppOpInfo)) {
                        // FIXME: report error, failed to evaluate the expression somehow
                        return std::nullopt;
                    }
                    parsedLhs = false;
                }
                else {
                    // FIXME: report error, expected a binary operator
                    return std::nullopt;
                }
            }
            else {
                if (token.klass == TokenKlass::IntegerConstant) {
                    auto value = ParseNumberLiteral(token.text.StrView());
                    if (value.IsScalarInt32()) {
                        PushOperand(value.GetInt32Value());
                    }
                    else if (value.IsScalarUInt32()) {
                        PushOperand(value.GetUInt32Value());
                    }
                    else {
                        // FIXME: report error, bad integer literal
                        return std::nullopt;
                    }
                    parsedLhs = true;
                }
                else if (token.klass == TokenKlass::DefinedYes) {
                    PushOperand(1);
                    parsedLhs = true;
                }
                else if (token.klass == TokenKlass::DefinedNo || token.klass == TokenKlass::Identifier) {
                    // NOTE macros are already expanded at this point. Unknown identifier is treated as 0.
                    PushOperand(0);
                    parsedLhs = true;
                }
                else if (token.klass == TokenKlass::LParen) {
                    operatorStack.push_back(GetParenPPOperator());
                    parsedLhs = false;
                }
                else if (auto ppOpInfo = ParseUnaryPPOperator(token)) {
                    if (!PushOperator(*ppOpInfo)) {
                        // FIXME: report error, failed to evaluate the expression somehow
                        return std::nullopt;
                    }
                    parsedLhs = false;
                }
                else {
                    // FIXME: report error, expected a unary operator or literal
                    return std::nullopt;
                }
            }
        }

        while (!operatorStack.empty()) {
            if (!EvalOperatorOnTop()) {
                // FIXME: report error, failed to evaluate the expression somehow
                return std::nullopt;
            }
        }

        if (valueStack.size() == 1) {
            return valueStack.back();
        }
        else {
            return std::nullopt;
        }
    }

    auto PPExprEvaluator::GetParenPPOperator() -> PPOperatorInfo
    {
        // Notably, we set precedence of parentheses to a very high value to ensure that it always stays on stack until
        // we find the matching right parenthesis.
        return PPOperatorInfo{.op = PPOperator::LParen, .precedence = 999, .isUnary = false};
    }

    auto PPExprEvaluator::ParseUnaryPPOperator(const PPToken& token) -> std::optional<PPOperatorInfo>
    {
        switch (token.klass) {
        case TokenKlass::Plus:
            return PPOperatorInfo{.op = PPOperator::Identity, .precedence = 2, .isUnary = true};
        case TokenKlass::Dash:
            return PPOperatorInfo{.op = PPOperator::Negate, .precedence = 2, .isUnary = true};
        case TokenKlass::Tilde:
            return PPOperatorInfo{.op = PPOperator::BitNot, .precedence = 2, .isUnary = true};
        case TokenKlass::Bang:
            return PPOperatorInfo{.op = PPOperator::LogicalNot, .precedence = 2, .isUnary = true};
        default:
            return std::nullopt;
        }
    }

    auto PPExprEvaluator::ParseBinaryPPOperator(const PPToken& token) -> std::optional<PPOperatorInfo>
    {
        switch (token.klass) {
        case TokenKlass::Star:
            return PPOperatorInfo{.op = PPOperator::Multiply, .precedence = 3, .isUnary = false};
        case TokenKlass::Slash:
            return PPOperatorInfo{.op = PPOperator::Divide, .precedence = 3, .isUnary = false};
        case TokenKlass::Percent:
            return PPOperatorInfo{.op = PPOperator::Modulo, .precedence = 3, .isUnary = false};
        case TokenKlass::Plus:
            return PPOperatorInfo{.op = PPOperator::Add, .precedence = 4, .isUnary = false};
        case TokenKlass::Dash:
            return PPOperatorInfo{.op = PPOperator::Subtract, .precedence = 4, .isUnary = false};
        case TokenKlass::LShift:
            return PPOperatorInfo{.op = PPOperator::LShift, .precedence = 5, .isUnary = false};
        case TokenKlass::RShift:
            return PPOperatorInfo{.op = PPOperator::RShift, .precedence = 5, .isUnary = false};
        case TokenKlass::LAngle:
            return PPOperatorInfo{.op = PPOperator::Less, .precedence = 6, .isUnary = false};
        case TokenKlass::LessEq:
            return PPOperatorInfo{.op = PPOperator::LessEq, .precedence = 6, .isUnary = false};
        case TokenKlass::RAngle:
            return PPOperatorInfo{.op = PPOperator::Greater, .precedence = 6, .isUnary = false};
        case TokenKlass::GreaterEq:
            return PPOperatorInfo{.op = PPOperator::GreaterEq, .precedence = 6, .isUnary = false};
        case TokenKlass::Equal:
            return PPOperatorInfo{.op = PPOperator::Equal, .precedence = 7, .isUnary = false};
        case TokenKlass::NotEqual:
            return PPOperatorInfo{.op = PPOperator::NotEqual, .precedence = 7, .isUnary = false};
        case TokenKlass::Ampersand:
            return PPOperatorInfo{.op = PPOperator::BitAnd, .precedence = 8, .isUnary = false};
        case TokenKlass::Caret:
            return PPOperatorInfo{.op = PPOperator::BitXor, .precedence = 9, .isUnary = false};
        case TokenKlass::VerticalBar:
            return PPOperatorInfo{.op = PPOperator::BitOr, .precedence = 10, .isUnary = false};
        case TokenKlass::And:
            return PPOperatorInfo{.op = PPOperator::LogicalAnd, .precedence = 11, .isUnary = false};
        case TokenKlass::Or:
            return PPOperatorInfo{.op = PPOperator::LogicalOr, .precedence = 12, .isUnary = false};
        default:
            return std::nullopt;
        }
    }

    auto PPExprEvaluator::EvaluatePPOperator(PPOperator op, int64_t lhs, int64_t rhs) -> int64_t
    {
        switch (op) {
        case PPOperator::LParen:
            return 0;
        case PPOperator::Identity:
            return lhs;
        case PPOperator::Negate:
            return -lhs;
        case PPOperator::BitNot:
            return ~lhs;
        case PPOperator::LogicalNot:
            return !lhs;
        case PPOperator::Multiply:
            return lhs * rhs;
        case PPOperator::Divide:
            return rhs != 0 ? lhs / rhs : 0;
        case PPOperator::Modulo:
            return rhs != 0 ? lhs % rhs : 0;
        case PPOperator::Add:
            return lhs + rhs;
        case PPOperator::Subtract:
            return lhs - rhs;
        case PPOperator::LShift:
            return lhs << rhs;
        case PPOperator::RShift:
            return lhs >> rhs;
        case PPOperator::Less:
            return lhs < rhs;
        case PPOperator::LessEq:
            return lhs <= rhs;
        case PPOperator::Greater:
            return lhs > rhs;
        case PPOperator::GreaterEq:
            return lhs >= rhs;
        case PPOperator::Equal:
            return lhs == rhs;
        case PPOperator::NotEqual:
            return lhs != rhs;
        case PPOperator::BitAnd:
            return lhs & rhs;
        case PPOperator::BitXor:
            return lhs ^ rhs;
        case PPOperator::BitOr:
            return lhs | rhs;
        case PPOperator::LogicalAnd:
            return lhs && rhs;
        case PPOperator::LogicalOr:
            return lhs || rhs;
        }

        return 0;
    }

    auto PPExprEvaluator::EvalOperatorOnTop() -> bool
    {
        if (operatorStack.empty()) {
            return false;
        }

        const auto& op = operatorStack.back();
        if (op.op == PPOperator::LParen) {
            return false;
        }

        uint64_t lhs = 0;
        uint64_t rhs = 0;
        if (op.isUnary) {
            if (valueStack.empty()) {
                return false;
            }
            lhs = valueStack.back();
            valueStack.pop_back();
        }
        else {
            if (valueStack.size() < 2) {
                return false;
            }
            rhs = valueStack.back();
            valueStack.pop_back();
            lhs = valueStack.back();
            valueStack.pop_back();
        }

        valueStack.push_back(EvaluatePPOperator(op.op, lhs, rhs));
        operatorStack.pop_back();
        return true;
    }

    auto PPExprEvaluator::PushOperator(PPOperatorInfo op) -> bool
    {
        GLSLD_ASSERT(op.op != PPOperator::LParen);

        // All unary operators are right-associative and binary operators are left-associative for now.
        while (!operatorStack.empty()) {
            if (operatorStack.back().HasHigherPrecedenceThan(op) ||
                (operatorStack.back().precedence == op.precedence && op.IsLeftAssociative())) {
                if (!EvalOperatorOnTop()) {
                    return false;
                }
            }
            else {
                break;
            }
        }

        operatorStack.push_back(op);
        return true;
    }

    auto PPExprEvaluator::PushOperand(int64_t value) -> bool
    {
        valueStack.push_back(value);
        return true;
    }
} // namespace glsld