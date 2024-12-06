#include "Language/ConstValue.h"

#include <charconv>
#include <concepts>
#include <functional>

#if GLSLD_COMPILER_MSVC
// Deliberately allow things like -1u as GLSL spec allows it
#pragma warning(disable : 4146)
#endif

namespace glsld
{
    auto ConstValue::GetGlslType() const noexcept -> std::optional<GlslBuiltinType>
    {
        if (IsScalar()) {
            switch (static_cast<ScalarKind>(scalarType)) {
            case ScalarKind::Bool:
                return GlslBuiltinType::Ty_bool;
            case ScalarKind::Int:
                return GlslBuiltinType::Ty_int;
            case ScalarKind::Uint:
                return GlslBuiltinType::Ty_uint;
            case ScalarKind::Float:
                return GlslBuiltinType::Ty_float;
            case ScalarKind::Double:
                return GlslBuiltinType::Ty_double;
            default:
                // FIXME: non-standard types?
                break;
            }
        }
        else if (IsVector()) {
            // FIXME: should we use row vector or column vector?
            switch (static_cast<ScalarKind>(scalarType)) {
            case ScalarKind::Bool:
                switch (colSize) {
                case 2:
                    return GlslBuiltinType::Ty_bvec2;
                case 3:
                    return GlslBuiltinType::Ty_bvec3;
                case 4:
                    return GlslBuiltinType::Ty_bvec4;
                default:
                    break;
                }
                break;
            case ScalarKind::Int:
                switch (colSize) {
                case 2:
                    return GlslBuiltinType::Ty_ivec2;
                case 3:
                    return GlslBuiltinType::Ty_ivec3;
                case 4:
                    return GlslBuiltinType::Ty_ivec4;
                default:
                    break;
                }
                break;
            case ScalarKind::Uint:
                switch (colSize) {
                case 2:
                    return GlslBuiltinType::Ty_uvec2;
                case 3:
                    return GlslBuiltinType::Ty_uvec3;
                case 4:
                    return GlslBuiltinType::Ty_uvec4;
                default:
                    break;
                }
                break;
            case ScalarKind::Float:
                switch (colSize) {
                case 2:
                    return GlslBuiltinType::Ty_vec2;
                case 3:
                    return GlslBuiltinType::Ty_vec3;
                case 4:
                    return GlslBuiltinType::Ty_vec4;
                default:
                    break;
                }
                break;
            case ScalarKind::Double:
                switch (colSize) {
                case 2:
                    return GlslBuiltinType::Ty_dvec2;
                case 3:
                    return GlslBuiltinType::Ty_dvec3;
                case 4:
                    return GlslBuiltinType::Ty_dvec4;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
        else if (IsMatrix()) {
            // FIXME: implement this
        }

        return std::nullopt;
    };

    template <typename F, typename... Ts>
    struct ExcludingTypes
    {
        // Exclude bool type to avoid compiler warning
        template <typename T>
        auto operator()(T value) const -> T
            requires(!(std::is_same_v<T, Ts> || ...) && requires(F f) { f(value); })
        {
            return F{}(value);
        }

        template <typename T>
        auto operator()(T lhs, T rhs) const -> T
            requires(!(std::is_same_v<T, Ts> || ...) && requires(F f) { f(lhs, rhs); })
        {
            return F{}(lhs, rhs);
        }
    };

    template <typename F>
    using ExcludingBool = ExcludingTypes<F, bool>;

    template <typename F>
    using ExcludingBoolFloat = ExcludingTypes<F, bool, float, double>;

    auto ConstValue::ElemwiseNegate() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(ExcludingBool<std::negate<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseUnaryOp(ExcludingTypes<std::negate<>, bool>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseBitNot() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(ExcludingBoolFloat<std::bit_not<>>{});
        // if (IsIntegralScalarType(GetScalarType())) {
        //     return ApplyElemwiseUnaryOp(ExcludingTypes<std::bit_not<>, bool, float, double>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseLogicalNot() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(std::logical_not<bool>{});
        // if (GetScalarType() == ScalarKind::Bool) {
        //     return ApplyElemwiseUnaryOp(std::logical_not<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::Length() const -> ConstValue
    {
        if (colSize > 0) {
            // FIXME: which type should it be?
            return ConstValue::FromValue(static_cast<int32_t>(colSize));
        }
        else {
            return ConstValue{};
        }
    }

    auto ConstValue::ElemwisePlus(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::plus<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, std::plus<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseMinus(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::minus<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, std::minus<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseMul(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::multiplies<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, std::multiplies<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseDiv(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::divides<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, ExcludingBool<std::divides<>>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseMod(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::modulus<>>{});
        // if (IsIntegralScalarType(GetScalarType())) {
        //     return ApplyElemwiseBinaryOp(other, ExcludingBool<std::modulus<>>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseBitAnd(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::bit_and<>>{});
        // if (IsIntegralScalarType(GetScalarType())) {
        //     return ApplyElemwiseBinaryOp(other, std::bit_and<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseBitOr(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::bit_or<>>{});
        // if (IsIntegralScalarType(GetScalarType())) {
        //     return ApplyElemwiseBinaryOp(other, std::bit_or<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseBitXor(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::bit_xor<>>{});
        // if (IsIntegralScalarType(GetScalarType())) {
        //     return ApplyElemwiseBinaryOp(other, std::bit_xor<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseLogicalAnd(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, std::logical_and<bool>{});
        // if (GetScalarType() == ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, std::logical_and<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseLogicalOr(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, std::logical_or<bool>{});
        // if (GetScalarType() == ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, std::logical_or<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseLogicalXor(const ConstValue& other) const -> ConstValue
    {
        struct LogicalXor
        {
            auto operator()(bool lhs, bool rhs) -> bool
            {
                return lhs != rhs;
            }
        };

        return ApplyElemwiseBinaryOp(other, LogicalXor{});
        // if (GetScalarType() == ScalarKind::Bool) {
        //     return ApplyElemwiseBinaryOp(other, LogicalXor{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseShiftLeft(const ConstValue& other) const -> ConstValue
    {
        // FIXME: implement this
        return ConstValue();
    }
    auto ConstValue::ElemwiseShiftRight(const ConstValue& other) const -> ConstValue
    {
        // FIXME: implement this
        return ConstValue();
    }

    auto ConstValue::ElemwiseEquals(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, std::equal_to<>{});
    }
    auto ConstValue::ElemwiseNotEquals(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, std::not_equal_to<>{});
    }
    auto ConstValue::ElemwiseLessThan(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::less<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseComparisonOp(other, std::less<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseLessThanEq(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::less_equal<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseComparisonOp(other, std::less_equal<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseGreaterThan(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::greater<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseComparisonOp(other, std::greater<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }
    auto ConstValue::ElemwiseGreaterThanEq(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::greater_equal<>>{});
        // if (GetScalarType() != ScalarKind::Bool) {
        //     return ApplyElemwiseComparisonOp(other, std::greater_equal<>{});
        // }
        // else {
        //     return ConstValue();
        // }
    }

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

    auto ParseNumberLiteral(StringView literalText) -> ConstValue
    {
        // FIXME: make sure we don't allow overflow
        if (literalText.EndWith("u") || literalText.EndWith("U")) {
            auto literalTextNoSuffix = literalText.DropBack(1);

            uint32_t value;
            auto parseResult = std::from_chars(literalTextNoSuffix.data(),
                                               literalTextNoSuffix.data() + literalTextNoSuffix.Size(), value);
            if (parseResult.ptr == literalTextNoSuffix.data() + literalTextNoSuffix.Size()) {
                return ConstValue::FromValue<uint32_t>(value);
            }
        }
        else if (literalText.EndWith("lf") || literalText.EndWith("LF")) {
            auto literalTextNoSuffix = literalText.DropBack(2);

            double value;
            auto parseResult = std::from_chars(literalTextNoSuffix.data(),
                                               literalTextNoSuffix.data() + literalTextNoSuffix.Size(), value);
            if (parseResult.ptr == literalTextNoSuffix.data() + literalTextNoSuffix.Size()) {
                return ConstValue::FromValue<double>(value);
            }
        }
        else if (literalText.EndWith("f") || literalText.EndWith("F")) {
            auto literalTextNoSuffix = literalText.DropBack(1);

            float value;
            auto parseResult = std::from_chars(literalTextNoSuffix.data(),
                                               literalTextNoSuffix.data() + literalTextNoSuffix.Size(), value);
            if (parseResult.ptr == literalTextNoSuffix.data() + literalTextNoSuffix.Size()) {
                return ConstValue::FromValue<float>(value);
            }
        }
        else {
            if (literalText.Contains('.') || literalText.Contains('e')) {
                float value;
                auto parseResult = std::from_chars(literalText.data(), literalText.data() + literalText.Size(), value);
                if (parseResult.ptr == literalText.data() + literalText.Size()) {
                    return ConstValue::FromValue<float>(value);
                }
            }
            else {
                int32_t value;
                auto parseResult = std::from_chars(literalText.data(), literalText.data() + literalText.Size(), value);
                if (parseResult.ptr == literalText.data() + literalText.Size()) {
                    return ConstValue::FromValue<int32_t>(value);
                }
            }
        }

        return ConstValue{};
    }
} // namespace glsld