#include "ConstValue.h"

#include <concepts>
#include <functional>

namespace glsld
{
    template <GlslBuiltinType... DispatchingTypes>
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

    template <GlslBuiltinType FirstType, GlslBuiltinType... RestTypes>
    struct ConstOpDispatcher<FirstType, RestTypes...>
    {
        template <typename F>
        auto DispatchUnaryOp(const ConstValue& operand) -> ConstValue
        {
            if (operand.GetValueType() == FirstType) {
                return ConstValue::FromValue<FirstType>(F{}(operand.GetCppValue<FirstType>()));
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
                return ConstValue::FromValue<FirstType>(
                    F{}(lhs.GetCppValue<FirstType>(), rhs.GetCppValue<FirstType>()));
            }
            else {
                return ConstOpDispatcher<RestTypes...>{}.template DispatchBinaryOp<F>(lhs, rhs);
            }
        }
    };

    using ArithmeticScalarDispatcher = ConstOpDispatcher<GlslBuiltinType::Ty_int, GlslBuiltinType::Ty_uint,
                                                         GlslBuiltinType::Ty_float, GlslBuiltinType::Ty_double>;
    using NegatableScalarDispatcher =
        ConstOpDispatcher<GlslBuiltinType::Ty_int, GlslBuiltinType::Ty_float, GlslBuiltinType::Ty_double>;
    using IntegralScalarDispatcher = ConstOpDispatcher<GlslBuiltinType::Ty_int, GlslBuiltinType::Ty_uint>;
    using FloatScalarDispatcher    = ConstOpDispatcher<GlslBuiltinType::Ty_float, GlslBuiltinType::Ty_double>;
    using BoolScalarDispatcher     = ConstOpDispatcher<GlslBuiltinType::Ty_bool>;

    using UniversalDispatcher  = ArithmeticScalarDispatcher;
    using ArithmeticDispatcher = ArithmeticScalarDispatcher;
    using IntegralDispatcher   = IntegralScalarDispatcher;
    using BoolDispatcher       = BoolScalarDispatcher;

    auto EvaluateUnaryOp(UnaryOp op, const ConstValue& operand) -> ConstValue
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
        case UnaryOp::Length:
            // FIXME: implement this
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }
    auto EvaluateBinaryOp(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue
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
    auto EvaluateSelectOp(const ConstValue& predicate, const ConstValue& ifBranchVal, const ConstValue& elseBranchVal)
        -> ConstValue
    {
        // Predicate must be bool
        if (predicate.GetValueType() != GlslBuiltinType::Ty_bool) {
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