#include "ConstValue.h"

#include <concepts>
#include <functional>

namespace glsld
{
    template <ConstValueType... DispatchingTypes>
    struct ConstOpDispatcher
    {
        template <typename F>
        auto DispatchUnaryOp(const ConstValue&) -> ConstValue
        {
            return ConstValue{};
        }

        template <typename F>
        auto DispatchBinaryOp(const ConstValue&, const ConstValue&) -> ConstValue
        {
            return ConstValue{};
        }
    };

    template <ConstValueType FirstType, ConstValueType... RestTypes>
    struct ConstOpDispatcher<FirstType, RestTypes...>
    {
        template <typename F>
        auto DispatchUnaryOp(const ConstValue& operand) -> ConstValue
        {
            if (operand.GetValueType() == FirstType) {
                return ConstValue{F{}(operand.GetCValue<FirstType>())};
            }
            else {
                return ConstOpDispatcher<RestTypes...>{}.template DispatchUnaryOp<F>(operand);
            }
        }

        template <typename F>
        auto DispatchBinaryOp(const ConstValue& lhs, const ConstValue& rhs) -> ConstValue
        {
            GLSLD_ASSERT(lhs.GetValueType() == rhs.GetValueType());
            if (lhs.GetValueType() == FirstType) {
                return ConstValue{F{}(lhs.GetCValue<FirstType>(), rhs.GetCValue<FirstType>())};
            }
            else {
                return ConstOpDispatcher<RestTypes...>{}.template DispatchBinaryOp<F>(lhs, rhs);
            }
        }
    };

    using ArithmeticScalarDispatcher =
        ConstOpDispatcher<ConstValueType::Int32, ConstValueType::UInt32, ConstValueType::Float, ConstValueType::Double>;
    using NegatableScalarDispatcher =
        ConstOpDispatcher<ConstValueType::Int32, ConstValueType::Float, ConstValueType::Double>;
    using IntegralScalarDispatcher = ConstOpDispatcher<ConstValueType::Int32, ConstValueType::UInt32>;
    using FloatScalarDispatcher    = ConstOpDispatcher<ConstValueType::Float, ConstValueType::Double>;
    using BoolScalarDispatcher     = ConstOpDispatcher<ConstValueType::Bool>;

    using UniversalDispatcher  = ArithmeticScalarDispatcher;
    using ArithmeticDispatcher = ArithmeticScalarDispatcher;
    using IntegralDispatcher   = IntegralScalarDispatcher;
    using BoolDispatcher       = BoolScalarDispatcher;

    auto EvaluateUnaryOp(UnaryOp op, ConstValue operand) -> ConstValue
    {
        switch (op) {
        case UnaryOp::Identity:
            return operand;
        case UnaryOp::Nagate:
            return NegatableScalarDispatcher{}.DispatchUnaryOp<std::negate<>>(operand);
        case UnaryOp::BitwiseNot:
            return IntegralDispatcher{}.DispatchUnaryOp<std::bit_not<>>(operand);
        case UnaryOp::LogicalNot:
            return BoolDispatcher{}.DispatchUnaryOp<std::logical_not<>>(operand);
        case UnaryOp::PrefixInc:
        case UnaryOp::PrefixDec:
        case UnaryOp::PostfixInc:
        case UnaryOp::PostfixDec:
            // Const value is immutable
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }
    auto EvaluateBinaryOp(BinaryOp op, ConstValue lhs, ConstValue rhs) -> ConstValue
    {
        // FIXME: do we have implicit conversion in constant expression?
        if (lhs.GetValueType() != rhs.GetValueType()) {
            return ConstValue{};
        }

        switch (op) {
        case BinaryOp::Plus:
            return ArithmeticDispatcher{}.DispatchBinaryOp<std::plus<>>(lhs, rhs);
        case BinaryOp::Minus:
            return ArithmeticDispatcher{}.DispatchBinaryOp<std::minus<>>(lhs, rhs);
        case BinaryOp::Mul:
            return ArithmeticDispatcher{}.DispatchBinaryOp<std::multiplies<>>(lhs, rhs);
        case BinaryOp::Div:
            return ArithmeticDispatcher{}.DispatchBinaryOp<std::divides<>>(lhs, rhs);
        case BinaryOp::Modulo:
            return IntegralDispatcher{}.DispatchBinaryOp<std::modulus<>>(lhs, rhs);
        case BinaryOp::Equal:
            return UniversalDispatcher{}.DispatchBinaryOp<std::equal_to<>>(lhs, rhs);
        case BinaryOp::NotEqual:
            return UniversalDispatcher{}.DispatchBinaryOp<std::not_equal_to<>>(lhs, rhs);
        case BinaryOp::Less:
            return UniversalDispatcher{}.DispatchBinaryOp<std::less<>>(lhs, rhs);
        case BinaryOp::LessEq:
            return UniversalDispatcher{}.DispatchBinaryOp<std::less_equal<>>(lhs, rhs);
        case BinaryOp::Greater:
            return UniversalDispatcher{}.DispatchBinaryOp<std::greater<>>(lhs, rhs);
        case BinaryOp::GreaterEq:
            return UniversalDispatcher{}.DispatchBinaryOp<std::greater_equal<>>(lhs, rhs);
        case BinaryOp::BitwiseAnd:
            return IntegralDispatcher{}.DispatchBinaryOp<std::bit_and<>>(lhs, rhs);
        case BinaryOp::BitwiseOr:
            return IntegralDispatcher{}.DispatchBinaryOp<std::bit_or<>>(lhs, rhs);
        case BinaryOp::BitwiseXor:
            return IntegralDispatcher{}.DispatchBinaryOp<std::bit_xor<>>(lhs, rhs);
        case BinaryOp::LogicalAnd:
            return BoolDispatcher{}.DispatchBinaryOp<std::logical_and<>>(lhs, rhs);
        case BinaryOp::LogicalOr:
            return BoolDispatcher{}.DispatchBinaryOp<std::logical_or<>>(lhs, rhs);
        case BinaryOp::LogicalXor:
            using LogicalXor = decltype([](bool lhs, bool rhs) { return (lhs && !rhs) || (rhs || !lhs); });
            return BoolDispatcher{}.DispatchBinaryOp<LogicalXor>(lhs, rhs);
        case BinaryOp::ShiftLeft:
            using ShiftLeft = decltype([](std::integral auto lhs, std::integral auto rhs) { return lhs << rhs; });
            return IntegralDispatcher{}.DispatchBinaryOp<ShiftLeft>(lhs, rhs);
        case BinaryOp::ShiftRight:
            using ShiftRight = decltype([](std::integral auto lhs, std::integral auto rhs) { return lhs >> rhs; });
            return IntegralDispatcher{}.DispatchBinaryOp<ShiftRight>(lhs, rhs);

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
            // These ops cannot operate on a const value
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }
    auto EvaluateSelectOp(ConstValue predicate, ConstValue ifBranchVal, ConstValue elseBranchVal) -> ConstValue
    {
        // Predicate must be bool
        if (predicate.GetValueType() != ConstValueType::Bool) {
            return ConstValue{};
        }

        // FIXME: type check if branch and else branch
        if (predicate.GetBoolValue()) {
            return ifBranchVal;
        }
        else {
            return elseBranchVal;
        }
    }
} // namespace glsld