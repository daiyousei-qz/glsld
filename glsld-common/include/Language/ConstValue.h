#pragma once
#include "Language/Semantic.h"
#include "Language/Typing.h"

#include <cstdint>
#include <optional>
#include <functional>
#include <type_traits>

namespace glsld
{
    class ConstValue
    {
    private:
        // Underlying scalar type. Real type is `ScalarType`.
        int16_t scalarType;
        // Flat array size. For scalar, this is 1.
        int16_t arraySize;
        // Row size of matrix. For non-matrix, this is 1.
        int16_t rowSize;
        // Column size of matrix. For non-matrix, this is 1.
        int16_t colSize;

        // We do small buffer optimization here. If the constant value doesn't fit the buffer, we use heap memory.
        // Note this object is not copyable to avoid frequent heap allocation.
        union
        {
            std::byte* bufferPtr;
            alignas(8) std::byte localBuffer[8];
        };

    public:
        ConstValue() : scalarType(0), arraySize(0), rowSize(0), colSize(0), localBuffer()
        {
        }
        ~ConstValue()
        {
            if (UseHeapBuffer()) {
                GLSLD_ASSERT(bufferPtr != nullptr);
                delete[] bufferPtr;
                bufferPtr = nullptr;
            }
        }

        ConstValue(const ConstValue& other)            = delete;
        ConstValue& operator=(const ConstValue& other) = delete;

        ConstValue(ConstValue&& other) noexcept
            : scalarType(other.scalarType), arraySize(other.arraySize), rowSize(other.rowSize), colSize(other.colSize),
              localBuffer()
        {
            bufferPtr = other.bufferPtr;
            other.InitializeAsError();
        }
        ConstValue& operator=(ConstValue&& other) noexcept
        {
            if (UseHeapBuffer()) {
                GLSLD_ASSERT(bufferPtr != nullptr);
                delete[] bufferPtr;
            }

            scalarType = other.scalarType;
            arraySize  = other.arraySize;
            rowSize    = other.rowSize;
            colSize    = other.colSize;
            bufferPtr  = other.bufferPtr;
            other.InitializeAsError();
            return *this;
        }

        template <typename T>
        static auto FromValue(const T& value) -> ConstValue
        {
            ConstValue result;
            if constexpr (std::is_same_v<T, bool>) {
                result.InitializeAs<bool>(ScalarKind::Bool, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int32_t>) {
                result.InitializeAs<int32_t>(ScalarKind::Int, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint32_t>) {
                result.InitializeAs<uint32_t>(ScalarKind::Uint, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, float>) {
                result.InitializeAs<float>(ScalarKind::Float, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, double>) {
                result.InitializeAs<double>(ScalarKind::Double, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int8_t>) {
                result.InitializeAs<int8_t>(ScalarKind::Int8, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint8_t>) {
                result.InitializeAs<uint8_t>(ScalarKind::Uint8, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int16_t>) {
                result.InitializeAs<int16_t>(ScalarKind::Int16, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint16_t>) {
                result.InitializeAs<uint16_t>(ScalarKind::Uint16, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                result.InitializeAs<int64_t>(ScalarKind::Int64, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint64_t>) {
                result.InitializeAs<uint64_t>(ScalarKind::Uint64, 1, 1, 1)[0] = value;
            }
            else {
                static_assert(AlwaysFalse<T>);
            }

            return result;
        }

        auto IsError() const noexcept -> bool
        {
            return arraySize == 0;
        }

        auto IsScalar() const noexcept -> bool
        {
            return arraySize == 1;
        }

        auto IsVector() const noexcept -> bool
        {
            // FIXME: should we use row vector or column vector?
            return rowSize == 1 && colSize > 1;
        }

        auto IsMatrix() const noexcept -> bool
        {
            return rowSize > 1 && colSize > 1;
        }

        auto IsVectorOrMatrix() const noexcept -> bool
        {
            return arraySize > 1;
        }

        auto GetGlslType() const noexcept -> std::optional<GlslBuiltinType>
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

        auto GetScalarType() const noexcept -> ScalarKind
        {
            return static_cast<ScalarKind>(scalarType);
        }
        auto IsScalarBool() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarKind::Bool;
        }
        auto IsScalarInt32() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarKind::Int;
        }
        auto IsScalarUInt32() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarKind::Uint;
        }
        auto IsScalarFloat() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarKind::Float;
        }
        auto IsScalarDouble() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarKind::Double;
        }

        auto GetBoolValue() const noexcept -> bool
        {
            GLSLD_ASSERT(IsScalarBool() && !UseHeapBuffer());
            return *reinterpret_cast<const bool*>(localBuffer);
        }
        auto GetInt32Value() const noexcept -> int32_t
        {
            GLSLD_ASSERT(IsScalarInt32() && !UseHeapBuffer());
            return *reinterpret_cast<const int32_t*>(localBuffer);
        }
        auto GetUInt32Value() const noexcept -> uint32_t
        {
            GLSLD_ASSERT(IsScalarUInt32() && !UseHeapBuffer());
            return *reinterpret_cast<const uint32_t*>(localBuffer);
        }
        auto GetFloatValue() const noexcept -> float
        {
            GLSLD_ASSERT(IsScalarFloat() && !UseHeapBuffer());
            return *reinterpret_cast<const float*>(localBuffer);
        }
        auto GetDoubleValue() const noexcept -> double
        {
            GLSLD_ASSERT(IsScalarDouble() && !UseHeapBuffer());
            return *reinterpret_cast<const double*>(localBuffer);
        }

        auto GetScalarSize() const noexcept -> int
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

        auto ToString() const -> std::string
        {
            if (IsError()) {
                return "<error>";
            }

            if (!IsScalar()) {
                return "<non-scalar>";
            }

            switch (GetScalarType()) {
            case ScalarKind::Bool:
                return GetBoolValue() ? "true" : "false";
            case ScalarKind::Int:
                return std::to_string(GetInt32Value());
            case ScalarKind::Uint:
                return std::to_string(GetUInt32Value());
            case ScalarKind::Float:
                return std::to_string(GetFloatValue());
            case ScalarKind::Double:
                return std::to_string(GetDoubleValue());
            default:
                return "<other>";
            }
        }

        auto Clone() const -> ConstValue
        {
            ConstValue result;
            result.scalarType = scalarType;
            result.arraySize  = arraySize;
            result.rowSize    = rowSize;
            result.colSize    = colSize;
            if (UseHeapBuffer()) {
                result.bufferPtr = new std::byte[arraySize * GetScalarSize()];
                std::memcpy(result.bufferPtr, bufferPtr, arraySize * GetScalarSize());
            }
            else {
                std::memcpy(result.localBuffer, localBuffer, sizeof(localBuffer));
            }

            return result;
        }

        template <typename F>
        struct ExcludingBool
        {
            // Exclude bool type to avoid compiler warning
            template <typename T>
            auto operator()(T value) const -> T
                requires(!std::is_same_v<T, bool> && requires(F f) { f(value); })
            {
                return F{}(value);
            }

            template <typename T>
            auto operator()(T lhs, T rhs) const -> T
                requires(!std::is_same_v<T, bool> && requires(F f) { f(lhs, rhs); })
            {
                return F{}(lhs, rhs);
            }
        };

        template <typename F>
        struct ExcludingUnsigned
        {
            // Exclude unsigned type to avoid compiler warning
            template <typename T>
            auto operator()(T value) const -> T
                requires(!std::is_unsigned_v<T> && requires(F f) { f(value); })
            {
                return F{}(value);
            }

            template <typename T>
            auto operator()(T lhs, T rhs) const -> T
                requires(!std::is_unsigned_v<T> && requires(F f) { f(lhs, rhs); })
            {
                return F{}(lhs, rhs);
            }
        };

        auto ElemwiseNegate() const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseUnaryOp(ExcludingUnsigned<ExcludingBool<std::negate<>>>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseBitNot() const -> ConstValue
        {
            if (IsIntegralScalarType(GetScalarType())) {
                return ApplyElemwiseUnaryOp(ExcludingBool<std::bit_not<>>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLogicalNot() const -> ConstValue
        {
            if (GetScalarType() == ScalarKind::Bool) {
                return ApplyElemwiseUnaryOp(std::logical_not<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto Length() const -> ConstValue
        {
            if (arraySize > 0) {
                // FIXME: implement this
                return ConstValue();
            }
            else {
                return ConstValue();
            }
        }

        auto ElemwisePlus(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, std::plus<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseMinus(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, std::minus<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseMul(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, std::multiplies<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseDiv(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, ExcludingBool<std::divides<>>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseMod(const ConstValue& other) const -> ConstValue
        {
            if (IsIntegralScalarType(GetScalarType())) {
                return ApplyElemwiseBinaryOp(other, ExcludingBool<std::modulus<>>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseBitAnd(const ConstValue& other) const -> ConstValue
        {
            if (IsIntegralScalarType(GetScalarType())) {
                return ApplyElemwiseBinaryOp(other, std::bit_and<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseBitOr(const ConstValue& other) const -> ConstValue
        {
            if (IsIntegralScalarType(GetScalarType())) {
                return ApplyElemwiseBinaryOp(other, std::bit_or<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseBitXor(const ConstValue& other) const -> ConstValue
        {
            if (IsIntegralScalarType(GetScalarType())) {
                return ApplyElemwiseBinaryOp(other, std::bit_xor<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLogicalAnd(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() == ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, std::logical_and<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLogicalOr(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() == ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, std::logical_or<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLogicalXor(const ConstValue& other) const -> ConstValue
        {
            struct LogicalXor
            {
                auto operator()(bool lhs, bool rhs) -> bool
                {
                    return lhs != rhs;
                }
            };

            if (GetScalarType() == ScalarKind::Bool) {
                return ApplyElemwiseBinaryOp(other, LogicalXor{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseShiftLeft(const ConstValue& other) const -> ConstValue
        {
            // FIXME: implement this
            return ConstValue();
        }
        auto ElemwiseShiftRight(const ConstValue& other) const -> ConstValue
        {
            // FIXME: implement this
            return ConstValue();
        }

        auto ElemwiseEquals(const ConstValue& other) const -> ConstValue
        {
            return ApplyElemwiseComparisonOp(other, std::equal_to<>{});
        }
        auto ElemwiseNotEquals(const ConstValue& other) const -> ConstValue
        {
            return ApplyElemwiseComparisonOp(other, std::not_equal_to<>{});
        }
        auto ElemwiseLessThan(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseComparisonOp(other, std::less<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLessThanEq(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseComparisonOp(other, std::less_equal<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseGreaterThan(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseComparisonOp(other, std::greater<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseGreaterThanEq(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarKind::Bool) {
                return ApplyElemwiseComparisonOp(other, std::greater_equal<>{});
            }
            else {
                return ConstValue();
            }
        }

    private:
        template <typename T, typename F>
        auto ApplyElemwiseUnaryOpUnsafe(F f) const -> ConstValue
        {
            ConstValue result;

            auto srcBuffer = GetBufferAs<T>();
            auto dstBuffer = result.InitializeAs<T>(GetScalarType(), arraySize, rowSize, colSize);

            for (int i = 0; i < arraySize; ++i) {
                dstBuffer[i] = f(srcBuffer[i]);
            }
            return result;
        }

        template <typename T, typename F>
        auto ApplyElemwiseBinaryOpUnsafe(const ConstValue& other, F f) const -> ConstValue
        {
            ConstValue result;

            auto srcBuffer1 = GetBufferAs<T>();
            auto srcBuffer2 = other.GetBufferAs<T>();
            auto dstBuffer  = result.InitializeAs<T>(GetScalarType(), arraySize, rowSize, colSize);

            for (int i = 0; i < arraySize; ++i) {
                dstBuffer[i] = f(srcBuffer1[i], srcBuffer2[i]);
            }
            return result;
        }

        template <typename T, typename F>
        auto ApplyElemwiseComparisonOpUnsafe(const ConstValue& other, F f) const -> ConstValue
        {
            ConstValue result;

            auto srcBuffer1 = GetBufferAs<T>();
            auto srcBuffer2 = other.GetBufferAs<T>();
            auto dstBuffer  = result.InitializeAs<bool>(ScalarKind::Bool, arraySize, rowSize, colSize);

            for (int i = 0; i < arraySize; ++i) {
                dstBuffer[i] = f(srcBuffer1[i], srcBuffer2[i]);
            }
            return result;
        }

        template <typename F>
        auto ApplyElemwiseUnaryOp(F f) const -> ConstValue
        {
            switch (static_cast<ScalarKind>(scalarType)) {
#if GLSLD_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4804)
#endif
            case ScalarKind::Bool:
                if constexpr (requires(bool x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<bool>(f);
                }
                else {
                    return ConstValue();
                }
#if GLSLD_COMPILER_MSVC
#pragma warning(pop)
#endif
            case ScalarKind::Int:
                if constexpr (requires(int32_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int32_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint:
                if constexpr (requires(uint32_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint32_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Float:
                if constexpr (requires(float x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<float>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Double:
                if constexpr (requires(double x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<double>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int8:
                if constexpr (requires(int8_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int8_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int16:
                if constexpr (requires(int16_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int16_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int64:
                if constexpr (requires(int64_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int64_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint8:
                if constexpr (requires(uint8_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint8_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint16:
                if constexpr (requires(uint16_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint16_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint64:
                if constexpr (requires(uint64_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint64_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
        }

        template <typename F>
        auto ApplyElemwiseBinaryOp(const ConstValue& other, F f) const -> ConstValue
        {
            switch (static_cast<ScalarKind>(scalarType)) {
#if GLSLD_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4804)
#endif
            case ScalarKind::Bool:
                if constexpr (requires(bool x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<bool>(other, f);
                }
                else {
                    return ConstValue();
                }
#if GLSLD_COMPILER_MSVC
#pragma warning(pop)
#endif
            case ScalarKind::Int:
                if constexpr (requires(int32_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint:
                if constexpr (requires(uint32_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Float:
                if constexpr (requires(float x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<float>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Double:
                if constexpr (requires(double x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<double>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int8:
                if constexpr (requires(int8_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int16:
                if constexpr (requires(int16_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int64:
                if constexpr (requires(int64_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint8:
                if constexpr (requires(uint8_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint16:
                if constexpr (requires(uint16_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint64:
                if constexpr (requires(uint64_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
        }

        template <typename F>
        auto ApplyElemwiseComparisonOp(const ConstValue& other, F f) const -> ConstValue
        {
            switch (static_cast<ScalarKind>(scalarType)) {
            case ScalarKind::Bool:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<bool>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<bool>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int32_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint32_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Float:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<float>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<float>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Double:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<double>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<double>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int8:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int8_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int16:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int16_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Int64:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int64_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint8:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint8_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint16:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint16_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Uint64:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint64_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarKind::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
        }

        auto GetBufferSize() const noexcept -> size_t
        {
            return GetScalarSize() * arraySize;
        }

        auto UseHeapBuffer() const noexcept -> bool
        {
            return GetBufferSize() > sizeof(localBuffer);
        }

        auto InitializeAsError() -> void
        {
            scalarType = 0;
            arraySize  = 0;
            rowSize    = 0;
            colSize    = 0;
            std::ranges::fill(localBuffer, std::byte(0));
        }

        template <typename T>
        auto InitializeAs(ScalarKind scalarType, int16_t arraySize, int16_t rowSize, int16_t colSize) -> ArraySpan<T>
        {
            GLSLD_ASSERT(IsError() && arraySize > 0 && arraySize == rowSize * colSize);

            this->scalarType = static_cast<int16_t>(scalarType);
            this->arraySize  = arraySize;
            this->rowSize    = rowSize;
            this->colSize    = colSize;

            GLSLD_ASSERT(GetScalarSize() == sizeof(T));
            void* buffer = nullptr;
            if (UseHeapBuffer()) {
                bufferPtr = new std::byte[GetBufferSize()];
                buffer    = bufferPtr;
            }
            else {
                buffer = &localBuffer;
            }

            return ArraySpan<T>(static_cast<T*>(buffer), arraySize);
        }

        template <typename T>
        auto GetBufferAs() const -> ArrayView<T>
        {
            GLSLD_ASSERT(GetScalarSize() == sizeof(T));
            const void* buffer = nullptr;
            if (UseHeapBuffer()) {
                buffer = bufferPtr;
            }
            else {
                buffer = &localBuffer;
            }

            return ArrayView<T>(static_cast<const T*>(buffer), arraySize);
        }
    };

    auto EvalUnaryConstExpr(UnaryOp op, const ConstValue& operand) -> ConstValue;
    auto EvalBinaryConstExpr(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue;
    auto EvalSelectConstExpr(const ConstValue& predicate, const ConstValue& ifBranchVal,
                             const ConstValue& elseBranchVal) -> ConstValue;

    auto ParseNumberLiteral(StringView literalText) -> ConstValue;
} // namespace glsld