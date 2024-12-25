#include "Language/ConstValue.h"

#include <charconv>
#include <concepts>
#include <functional>
#include <numbers>

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

    auto ConstValue::GetScalarSize() const noexcept -> int
    {
        switch (static_cast<ScalarKind>(scalarType)) {
        case ScalarKind::Bool:
        case ScalarKind::Int8:
        case ScalarKind::Uint8:
            return 1;
        case ScalarKind::Int16:
        case ScalarKind::Uint16:
        case ScalarKind::Float16:
            return 2;
        case ScalarKind::Int:
        case ScalarKind::Uint:
        case ScalarKind::Float:
            return 4;
        case ScalarKind::Double:
        case ScalarKind::Int64:
        case ScalarKind::Uint64:
            return 8;
        default:
            return 0;
        }
    }

    auto ConstValue::ToString() const -> std::string
    {
        if (IsError()) {
            return "<error>";
        }

        if (!IsScalar()) {
            // FIXME: handle non-scalar types
            return "<non-scalar>";
        }

        switch (GetScalarKind()) {
        case ScalarKind::Bool:
            return GetBufferAs<bool>()[0] ? "true" : "false";
        case ScalarKind::Int:
            return std::to_string(GetBufferAs<int32_t>()[0]);
        case ScalarKind::Uint:
            return std::to_string(GetBufferAs<uint32_t>()[0]);
        case ScalarKind::Float:
            return std::to_string(GetBufferAs<float>()[0]);
        case ScalarKind::Double:
            return std::to_string(GetBufferAs<double>()[0]);
        case ScalarKind::Int8:
            return std::to_string(GetBufferAs<int8_t>()[0]);
        case ScalarKind::Int16:
            return std::to_string(GetBufferAs<int16_t>()[0]);
        case ScalarKind::Int64:
            return std::to_string(GetBufferAs<int64_t>()[0]);
        case ScalarKind::Uint8:
            return std::to_string(GetBufferAs<uint8_t>()[0]);
        case ScalarKind::Uint16:
            return std::to_string(GetBufferAs<uint16_t>()[0]);
        case ScalarKind::Uint64:
            return std::to_string(GetBufferAs<uint64_t>()[0]);
        case ScalarKind::Float16:
            GLSLD_NO_IMPL();
            return "<float16>";
        default:
            return "<other>";
        }
    }

    auto ConstValue::Clone() const -> ConstValue
    {
        ConstValue result;

        auto blob = result.InitializeAsBlob(static_cast<ScalarKind>(scalarType), rowSize, colSize);
        std::ranges::copy(GetBufferAsBlob(), blob.begin());

        return result;
    }

    auto ConstValue::CastScalar(ScalarKind kind) const -> ConstValue
    {
        if (IsError()) {
            return ConstValue{};
        }
        if (GetScalarKind() == kind) {
            return Clone();
        }

        switch (kind) {
        case ScalarKind::Bool:
            return ApplyElemwiseCast<bool>();
        case ScalarKind::Int:
            return ApplyElemwiseCast<int32_t>();
        case ScalarKind::Uint:
            return ApplyElemwiseCast<uint32_t>();
        case ScalarKind::Float:
            return ApplyElemwiseCast<float>();
        case ScalarKind::Double:
            return ApplyElemwiseCast<double>();
        case ScalarKind::Int8:
            return ApplyElemwiseCast<int8_t>();
        case ScalarKind::Uint8:
            return ApplyElemwiseCast<uint8_t>();
        case ScalarKind::Int16:
            return ApplyElemwiseCast<int16_t>();
        case ScalarKind::Uint16:
            return ApplyElemwiseCast<uint16_t>();
        case ScalarKind::Int64:
            return ApplyElemwiseCast<int64_t>();
        case ScalarKind::Uint64:
            return ApplyElemwiseCast<uint64_t>();
        case ScalarKind::Float16:
            GLSLD_NO_IMPL();
            return ConstValue{};
        default:
            return ConstValue{};
        }
    }

    auto ConstValue::GetElement(int index) const -> ConstValue
    {
        // FIXME: verify this is correct
        if (colSize <= 1 || index < 0 || index >= colSize) {
            return ConstValue{};
        }

        ConstValue result;
        auto blob          = result.InitializeAsBlob(GetScalarKind(), 1, rowSize);
        auto elementSize   = GetBufferSize() / colSize;
        auto elementOffset = index * elementSize;
        std::ranges::copy_n(GetBufferAsBlob().begin() + elementOffset, blob.size(), blob.begin());
        return result;
    }

    auto ConstValue::GetSwizzle(SwizzleDesc swizzle) const -> ConstValue
    {
        // FIXME: verify this is correct
        if (IsError() || !swizzle.IsValid()) {
            return ConstValue{};
        }

        std::vector<std::byte> buffer;
        for (auto index : swizzle.GetIndices()) {
            auto elem = GetElement(index);
            if (elem.IsError()) {
                return ConstValue{};
            }

            std::ranges::copy(elem.GetBufferAsBlob(), std::back_inserter(buffer));
        }

        ConstValue result;
        auto blob = result.InitializeAsBlob(GetScalarKind(), rowSize, swizzle.GetDimension());
        std::ranges::copy(buffer, blob.data());
        return result;
    }

    namespace
    {
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
    } // namespace

    auto ConstValue::ElemwiseNegate() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(ExcludingBool<std::negate<>>{});
    }
    auto ConstValue::ElemwiseBitNot() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(ExcludingBoolFloat<std::bit_not<>>{});
    }
    auto ConstValue::ElemwiseLogicalNot() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(std::logical_not<bool>{});
    }
    auto ConstValue::Length() const -> ConstValue
    {
        if (colSize > 0) {
            // FIXME: which type should it be?
            return ConstValue::CreateScalar(static_cast<int32_t>(colSize));
        }
        else {
            return ConstValue{};
        }
    }

#pragma region Binary
    auto ConstValue::ElemwisePlus(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::plus<>>{});
    }
    auto ConstValue::ElemwiseMinus(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::minus<>>{});
    }
    auto ConstValue::ElemwiseMul(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::multiplies<>>{});
    }
    auto ConstValue::ElemwiseDiv(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBool<std::divides<>>{});
    }
    auto ConstValue::ElemwiseMod(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::modulus<>>{});
    }
    auto ConstValue::ElemwiseBitAnd(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::bit_and<>>{});
    }
    auto ConstValue::ElemwiseBitOr(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::bit_or<>>{});
    }
    auto ConstValue::ElemwiseBitXor(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, ExcludingBoolFloat<std::bit_xor<>>{});
    }
    auto ConstValue::ElemwiseLogicalAnd(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, std::logical_and<bool>{});
    }
    auto ConstValue::ElemwiseLogicalOr(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, std::logical_or<bool>{});
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
    }
    auto ConstValue::ElemwiseLessThanEq(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::less_equal<>>{});
    }
    auto ConstValue::ElemwiseGreaterThan(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::greater<>>{});
    }
    auto ConstValue::ElemwiseGreaterThanEq(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(other, ExcludingBool<std::greater_equal<>>{});
    }
#pragma endregion

#pragma region Builtin
    namespace
    {
        // FIXME: check std rounding
        struct Radians
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return (std::numbers::pi_v<FloatType> / static_cast<FloatType>(180)) * value;
            }
        };

        struct Degrees
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return (static_cast<FloatType>(180) / std::numbers::pi_v<FloatType>)*value;
            }
        };

        struct Sin
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::sin(value);
            }
        };

        struct Cos
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::cos(value);
            }
        };

        struct Asin
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::asin(value);
            }
        };

        struct Acos
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::acos(value);
            }
        };

        struct Pow
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType lhs, FloatType rhs) -> FloatType
            {
                return std::pow(lhs, rhs);
            }
        };

        struct Exp
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::exp(value);
            }
        };

        struct Log
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::log(value);
            }
        };

        struct Exp2
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::exp2(value);
            }
        };

        struct Log2
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::log2(value);
            }
        };

        struct Sqrt
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::sqrt(value);
            }
        };

        struct InverseSqrt
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return static_cast<FloatType>(1) / std::sqrt(value);
            }
        };

        struct Abs
        {
            template <typename ArithmeticType>
                requires std::signed_integral<ArithmeticType> || std::floating_point<ArithmeticType>
            auto operator()(ArithmeticType value) -> ArithmeticType
            {
                return value > 0 ? value : -value;
            }
        };

        struct Sign
        {
            template <typename ArithmeticType>
                requires std::signed_integral<ArithmeticType> || std::floating_point<ArithmeticType>
            auto operator()(ArithmeticType value) -> ArithmeticType
            {
                return value > 0 ? 1 : (value < 0 ? -1 : 0);
            }
        };

        struct Floor
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::floor(value);
            }
        };

        struct Trunc
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::trunc(value);
            }
        };

        struct Round
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::round(value);
            }
        };

        struct Ceil
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType value) -> FloatType
            {
                return std::ceil(value);
            }
        };

        struct Mod
        {
            template <std::floating_point FloatType>
            auto operator()(FloatType lhs, FloatType rhs) -> FloatType
            {
                return std::fmod(lhs, rhs);
            }
        };

        struct Min
        {
            template <typename ArithmeticType>
                requires std::integral<ArithmeticType> || std::floating_point<ArithmeticType>
            auto operator()(ArithmeticType lhs, ArithmeticType rhs) -> ArithmeticType
            {
                return std::min<ArithmeticType>(lhs, rhs);
            }
        };

        struct Max
        {
            template <typename ArithmeticType>
                requires std::integral<ArithmeticType> || std::floating_point<ArithmeticType>
            auto operator()(ArithmeticType lhs, ArithmeticType rhs) -> ArithmeticType
            {
                return std::max<ArithmeticType>(lhs, rhs);
            }
        };
    } // namespace

    auto ConstValue::ElemwiseRadians() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Radians{});
    }
    auto ConstValue::ElemwiseDegrees() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Degrees{});
    }
    auto ConstValue::ElemwiseSin() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Sin{});
    }
    auto ConstValue::ElemwiseCos() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Cos{});
    }
    auto ConstValue::ElemwiseAsin() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Asin{});
    }
    auto ConstValue::ElemwiseAcos() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Acos{});
    }

    auto ConstValue::ElemwisePow(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, Pow{});
    }
    auto ConstValue::ElemwiseExp() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Exp{});
    }
    auto ConstValue::ElemwiseLog() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Log{});
    }
    auto ConstValue::ElemwiseExp2() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Exp2{});
    }
    auto ConstValue::ElemwiseLog2() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Log2{});
    }
    auto ConstValue::ElemwiseSqrt() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Sqrt{});
    }
    auto ConstValue::ElemwiseInverseSqrt() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(InverseSqrt{});
    }
    auto ConstValue::ElemwiseAbs() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Abs{});
    }
    auto ConstValue::ElemwiseSign() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Sign{});
    }
    auto ConstValue::ElemwiseFloor() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Floor{});
    }
    auto ConstValue::ElemwiseTrunc() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Trunc{});
    }
    auto ConstValue::ElemwiseRound() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Round{});
    }
    auto ConstValue::ElemwiseCeil() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(Ceil{});
    }
    // auto ConstValue::ElemwiseMod(const ConstValue& other) const -> ConstValue
    // {
    //     return ApplyElemwiseBinaryOp(other, Mod{});
    // }
    auto ConstValue::ElemwiseMin(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, Min{});
    }
    auto ConstValue::ElemwiseMax(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(other, Max{});
    }
    auto ConstValue::ElemwiseClamp(const ConstValue& min, const ConstValue& max) const -> ConstValue
    {
        return ElemwiseMax(min).ElemwiseMin(max);
    }
#pragma endregion

    auto ConstValue::InitializeAsBlob(ScalarKind scalarType, int16_t rowSize, int16_t colSize) -> ArraySpan<std::byte>
    {
        GLSLD_ASSERT(IsError() && rowSize >= 0 && colSize >= 0);

        this->scalarType = static_cast<int16_t>(scalarType);
        this->arraySize  = rowSize * colSize;
        this->rowSize    = rowSize;
        this->colSize    = colSize;

        std::byte* buffer = nullptr;
        if (UseHeapBuffer()) {
            bufferPtr = new std::byte[GetBufferSize()];
            buffer    = bufferPtr;
        }
        else {
            buffer = reinterpret_cast<std::byte*>(&localBuffer);
        }

        return ArraySpan<std::byte>(buffer, GetBufferSize());
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
                return ConstValue::CreateScalar<uint32_t>(value);
            }
        }
        else if (literalText.EndWith("lf") || literalText.EndWith("LF")) {
            auto literalTextNoSuffix = literalText.DropBack(2);

            double value;
            auto parseResult = std::from_chars(literalTextNoSuffix.data(),
                                               literalTextNoSuffix.data() + literalTextNoSuffix.Size(), value);
            if (parseResult.ptr == literalTextNoSuffix.data() + literalTextNoSuffix.Size()) {
                return ConstValue::CreateScalar<double>(value);
            }
        }
        else if (literalText.EndWith("f") || literalText.EndWith("F")) {
            auto literalTextNoSuffix = literalText.DropBack(1);

            float value;
            auto parseResult = std::from_chars(literalTextNoSuffix.data(),
                                               literalTextNoSuffix.data() + literalTextNoSuffix.Size(), value);
            if (parseResult.ptr == literalTextNoSuffix.data() + literalTextNoSuffix.Size()) {
                return ConstValue::CreateScalar<float>(value);
            }
        }
        else {
            if (literalText.Contains('.') || literalText.Contains('e')) {
                float value;
                auto parseResult = std::from_chars(literalText.data(), literalText.data() + literalText.Size(), value);
                if (parseResult.ptr == literalText.data() + literalText.Size()) {
                    return ConstValue::CreateScalar<float>(value);
                }
            }
            else {
                int32_t value;
                auto parseResult = std::from_chars(literalText.data(), literalText.data() + literalText.Size(), value);
                if (parseResult.ptr == literalText.data() + literalText.Size()) {
                    return ConstValue::CreateScalar<int32_t>(value);
                }
            }
        }

        return ConstValue{};
    }
} // namespace glsld