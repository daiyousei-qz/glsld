#include "Language/ConstValue.h"

#include <concepts>
#include <functional>

namespace glsld
{
    auto EvalUnaryConstExpr(UnaryOp op, const ConstValue& operand) -> ConstValue
    {
        if (operand.IsError()) {
            return ConstValue{};
        }

        switch (op) {
        case UnaryOp::Identity:
            return operand.Clone();
        case UnaryOp::Negate:
            return operand.ElemwiseNegate();
        case UnaryOp::BitwiseNot:
            return operand.ElemwiseBitNot();
        case UnaryOp::LogicalNot:
            return operand.ElemwiseLogicalNot();
        case UnaryOp::PrefixInc:
        case UnaryOp::PrefixDec:
        case UnaryOp::PostfixInc:
        case UnaryOp::PostfixDec:
            // Fast path. Returns error since constant expression cannot have side effect.
            return ConstValue{};
        case UnaryOp::Length:
            return operand.Length();
        }

        GLSLD_UNREACHABLE();
    }
    auto EvalBinaryConstExpr(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue
    {
        if (lhs.IsError() || rhs.IsError()) {
            return ConstValue{};
        }

        switch (op) {
        case BinaryOp::Plus:
            return lhs.ElemwisePlus(rhs);
        case BinaryOp::Minus:
            return lhs.ElemwiseMinus(rhs);
        case BinaryOp::Mul:
            return lhs.ElemwiseMul(rhs);
        case BinaryOp::Div:
            return lhs.ElemwiseDiv(rhs);
        case BinaryOp::Modulo:
            return lhs.ElemwiseMod(rhs);
        case BinaryOp::Equal:
            return lhs.ElemwiseEquals(rhs);
        case BinaryOp::NotEqual:
            return lhs.ElemwiseNotEquals(rhs);
        case BinaryOp::Less:
            return lhs.ElemwiseLessThan(rhs);
        case BinaryOp::LessEq:
            return lhs.ElemwiseLessThanEq(rhs);
        case BinaryOp::Greater:
            return lhs.ElemwiseGreaterThan(rhs);
        case BinaryOp::GreaterEq:
            return lhs.ElemwiseGreaterThanEq(rhs);
        case BinaryOp::BitwiseAnd:
            return lhs.ElemwiseBitAnd(rhs);
        case BinaryOp::BitwiseOr:
            return lhs.ElemwiseBitOr(rhs);
        case BinaryOp::BitwiseXor:
            return lhs.ElemwiseBitXor(rhs);
        case BinaryOp::LogicalAnd:
            return lhs.ElemwiseLogicalAnd(rhs);
        case BinaryOp::LogicalOr:
            return lhs.ElemwiseLogicalOr(rhs);
        case BinaryOp::LogicalXor:
            return lhs.ElemwiseLogicalXor(rhs);
        case BinaryOp::ShiftLeft:
            return lhs.ElemwiseShiftLeft(rhs);
        case BinaryOp::ShiftRight:
            return lhs.ElemwiseShiftRight(rhs);
        case BinaryOp::Comma:
        case BinaryOp::Assign:
        case BinaryOp::MulAssign:
        case BinaryOp::DivAssign:
        case BinaryOp::ModAssign:
        case BinaryOp::AddAssign:
        case BinaryOp::SubAssign:
        case BinaryOp::LShiftAssign:
        case BinaryOp::RShiftAssign:
        case BinaryOp::AndAssign:
        case BinaryOp::XorAssign:
        case BinaryOp::OrAssign:
            // Fast path. These ops cannot produce a constant expression.
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }
    auto EvalSelectConstExpr(const ConstValue& predicate, const ConstValue& ifBranchVal,
                             const ConstValue& elseBranchVal) -> ConstValue
    {
        // Predicate must be bool
        if (!predicate.IsScalarBool()) {
            return ConstValue{};
        }

        // FIXME: type check if branch and else branch
        if (predicate.GetBoolValue()) {
            return ifBranchVal.Clone();
        }
        else {
            return elseBranchVal.Clone();
        }
    }
} // namespace glsld