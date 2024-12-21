#pragma once
#include "Basic/Common.h"
#include "Language/Semantic.h"
#include "Language/Typing.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace glsld
{
    // Represents a compile-time constant primitive. This can be either:
    // - error
    // - scalar
    // - column vector
    // - matrix
    class ConstValue
    {
    private:
        // Underlying scalar type. Real type is `ScalarKind`.
        int16_t scalarType;
        // Flat array size. For scalar, this is 1. Also, we use array size 0 to denote error type.
        int16_t arraySize;
        // Row dimension. For non-matrix, this is 1. For error, this is 0.
        int16_t rowSize;
        // Column dimension. For scalar, this is 1. For error, this is 0.
        int16_t colSize;

        // We do small buffer optimization here. If the constant value doesn't fit the buffer, we use heap memory.
        union
        {
            std::byte* bufferPtr;
            alignas(8) std::byte localBuffer[24];
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

        // We disable copy ctor to avoid accidental heap allocation.
        ConstValue(const ConstValue& other)            = delete;
        ConstValue& operator=(const ConstValue& other) = delete;

        ConstValue(ConstValue&& other) noexcept
            : scalarType(other.scalarType), arraySize(other.arraySize), rowSize(other.rowSize), colSize(other.colSize)
        {
            std::ranges::copy(other.localBuffer, localBuffer);
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
            std::ranges::copy(other.localBuffer, localBuffer);
            other.InitializeAsError();
            return *this;
        }
        auto operator==(const ConstValue& other) const noexcept -> bool
        {
            if (scalarType != other.scalarType || arraySize != other.arraySize || rowSize != other.rowSize ||
                colSize != other.colSize) {
                return false;
            }

            return std::ranges::equal(GetBufferAsBlob(), other.GetBufferAsBlob());
        }

        template <typename T>
        static auto CreateScalar(const T& value) -> ConstValue
        {
            ConstValue result;
            result.InitializeAs<T>(1, 1)[0] = value;
            return result;
        }

        template <typename T>
        static auto CreateVector(std::initializer_list<T> values) -> ConstValue
        {
            if (values.size() == 0) {
                return ConstValue();
            }

            ConstValue result;
            auto elements = result.InitializeAs<T>(1, values.size());
            std::ranges::copy(values, elements.begin());
            return result;
        }

        auto GetBufferAsBlob() const -> ArrayView<std::byte>
        {
            return ArrayView<std::byte>(GetBufferPtr(), GetBufferSize());
        }

        template <typename T>
        auto GetBufferAs() const -> ArrayView<T>
        {
            GLSLD_ASSERT(GetScalarKindFromCppType<T>() == GetScalarKind());
            return ArrayView<T>(reinterpret_cast<const T*>(GetBufferPtr()), arraySize);
        }
        auto GetBoolValue() const -> bool
        {
            GLSLD_ASSERT(IsScalarBool());
            return GetBufferAs<bool>()[0];
        }
        auto GetInt32Value() const -> int32_t
        {
            GLSLD_ASSERT(IsScalarInt32());
            return GetBufferAs<int32_t>()[0];
        }
        auto GetUInt32Value() const -> uint32_t
        {
            GLSLD_ASSERT(IsScalarUInt32());
            return GetBufferAs<uint32_t>()[0];
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

        auto GetGlslType() const noexcept -> std::optional<GlslBuiltinType>;

        auto GetScalarKind() const noexcept -> ScalarKind
        {
            return static_cast<ScalarKind>(scalarType);
        }
        auto GetRowSize() const noexcept -> int
        {
            return rowSize;
        }
        auto GetColumnSize() const noexcept -> int
        {
            return colSize;
        }
        auto GetArraySize() const noexcept -> int
        {
            return arraySize;
        }

        auto IsScalarBool() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Bool;
        }
        auto IsScalarInt32() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Int;
        }
        auto IsScalarUInt32() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Uint;
        }
        auto IsScalarFloat() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Float;
        }
        auto IsScalarDouble() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Double;
        }
        auto IsScalarInt8() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Int8;
        }
        auto IsScalarUInt8() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Uint8;
        }
        auto IsScalarInt16() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Int16;
        }
        auto IsScalarUInt16() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Uint16;
        }
        auto IsScalarInt64() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Int64;
        }
        auto IsScalarUInt64() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Uint64;
        }
        auto IsScalarFloat16() const noexcept -> bool
        {
            return IsScalar() && GetScalarKind() == ScalarKind::Float16;
        }

        auto GetScalarSize() const noexcept -> int;

        auto ToString() const -> std::string;
        auto Clone() const -> ConstValue;

        auto GetElement(int index) const -> ConstValue;
        auto GetSwizzle(SwizzleDesc swizzle) const -> ConstValue;

        auto ElemwiseNegate() const -> ConstValue;
        auto ElemwiseBitNot() const -> ConstValue;
        auto ElemwiseLogicalNot() const -> ConstValue;
        auto Length() const -> ConstValue;

        auto ElemwisePlus(const ConstValue& other) const -> ConstValue;
        auto ElemwiseMinus(const ConstValue& other) const -> ConstValue;
        auto ElemwiseMul(const ConstValue& other) const -> ConstValue;
        auto ElemwiseDiv(const ConstValue& other) const -> ConstValue;
        auto ElemwiseMod(const ConstValue& other) const -> ConstValue;
        auto ElemwiseBitAnd(const ConstValue& other) const -> ConstValue;
        auto ElemwiseBitOr(const ConstValue& other) const -> ConstValue;
        auto ElemwiseBitXor(const ConstValue& other) const -> ConstValue;
        auto ElemwiseLogicalAnd(const ConstValue& other) const -> ConstValue;
        auto ElemwiseLogicalOr(const ConstValue& other) const -> ConstValue;
        auto ElemwiseLogicalXor(const ConstValue& other) const -> ConstValue;
        auto ElemwiseShiftLeft(const ConstValue& other) const -> ConstValue;
        auto ElemwiseShiftRight(const ConstValue& other) const -> ConstValue;

        auto ElemwiseEquals(const ConstValue& other) const -> ConstValue;
        auto ElemwiseNotEquals(const ConstValue& other) const -> ConstValue;
        auto ElemwiseLessThan(const ConstValue& other) const -> ConstValue;
        auto ElemwiseLessThanEq(const ConstValue& other) const -> ConstValue;
        auto ElemwiseGreaterThan(const ConstValue& other) const -> ConstValue;
        auto ElemwiseGreaterThanEq(const ConstValue& other) const -> ConstValue;

        auto ElemwiseRadians() const -> ConstValue;
        auto ElemwiseDegrees() const -> ConstValue;
        auto ElemwiseSin() const -> ConstValue;
        auto ElemwiseCos() const -> ConstValue;
        auto ElemwiseAsin() const -> ConstValue;
        auto ElemwiseAcos() const -> ConstValue;
        auto ElemwisePow(const ConstValue& other) const -> ConstValue;
        auto ElemwiseExp() const -> ConstValue;
        auto ElemwiseLog() const -> ConstValue;
        auto ElemwiseExp2() const -> ConstValue;
        auto ElemwiseLog2() const -> ConstValue;
        auto ElemwiseSqrt() const -> ConstValue;
        auto ElemwiseInverseSqrt() const -> ConstValue;
        auto ElemwiseAbs() const -> ConstValue;
        auto ElemwiseSign() const -> ConstValue;
        auto ElemwiseFloor() const -> ConstValue;
        auto ElemwiseTrunc() const -> ConstValue;
        auto ElemwiseRound() const -> ConstValue;
        auto ElemwiseCeil() const -> ConstValue;
        // auto ElemwiseMod(const ConstValue& other) const -> ConstValue;
        auto ElemwiseMin(const ConstValue& other) const -> ConstValue;
        auto ElemwiseMax(const ConstValue& other) const -> ConstValue;
        auto ElemwiseClamp(const ConstValue& min, const ConstValue& max) const -> ConstValue;

    private:
        template <typename T>
        static constexpr auto GetScalarKindFromCppType() -> ScalarKind
        {
            if constexpr (std::is_same_v<T, bool>) {
                return ScalarKind::Bool;
            }
            else if constexpr (std::is_same_v<T, int32_t>) {
                return ScalarKind::Int;
            }
            else if constexpr (std::is_same_v<T, uint32_t>) {
                return ScalarKind::Uint;
            }
            else if constexpr (std::is_same_v<T, float>) {
                return ScalarKind::Float;
            }
            else if constexpr (std::is_same_v<T, double>) {
                return ScalarKind::Double;
            }
            else if constexpr (std::is_same_v<T, int8_t>) {
                return ScalarKind::Int8;
            }
            else if constexpr (std::is_same_v<T, uint8_t>) {
                return ScalarKind::Uint8;
            }
            else if constexpr (std::is_same_v<T, int16_t>) {
                return ScalarKind::Int16;
            }
            else if constexpr (std::is_same_v<T, uint16_t>) {
                return ScalarKind::Uint16;
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                return ScalarKind::Int64;
            }
            else if constexpr (std::is_same_v<T, uint64_t>) {
                return ScalarKind::Uint64;
            }
            else {
                static_assert(AlwaysFalse<T>);
            }
        }

        template <typename T, typename F>
        auto ApplyElemwiseUnaryOpUnsafe(F f) const -> ConstValue
        {
            ConstValue result;

            auto srcBuffer = GetBufferAs<T>();
            auto dstBuffer = result.InitializeAs<T>(rowSize, colSize);

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
            auto dstBuffer  = result.InitializeAs<T>(rowSize, colSize);

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
            auto dstBuffer  = result.InitializeAs<bool>(rowSize, colSize);

            for (int i = 0; i < arraySize; ++i) {
                dstBuffer[i] = f(srcBuffer1[i], srcBuffer2[i]);
            }
            return result;
        }

        template <typename F>
        auto ApplyElemwiseUnaryOp(F f) const -> ConstValue
        {
#define SWITCH_CASE(CPPTYPE)                                                                                           \
    if constexpr (requires(CPPTYPE x) { f(x); }) {                                                                     \
        return ApplyElemwiseUnaryOpUnsafe<CPPTYPE>(f);                                                                 \
    }                                                                                                                  \
    else {                                                                                                             \
        return ConstValue();                                                                                           \
    }
            switch (static_cast<ScalarKind>(scalarType)) {
            case ScalarKind::Bool:
                SWITCH_CASE(bool)
            case ScalarKind::Int:
                SWITCH_CASE(int32_t)
            case ScalarKind::Uint:
                SWITCH_CASE(uint32_t)
            case ScalarKind::Float:
                SWITCH_CASE(float)
            case ScalarKind::Double:
                SWITCH_CASE(double)
            case ScalarKind::Int8:
                SWITCH_CASE(int8_t)
            case ScalarKind::Int16:
                SWITCH_CASE(int16_t)
            case ScalarKind::Int64:
                SWITCH_CASE(int64_t)
            case ScalarKind::Uint8:
                SWITCH_CASE(int8_t)
            case ScalarKind::Uint16:
                SWITCH_CASE(uint16_t)
            case ScalarKind::Uint64:
                SWITCH_CASE(uint64_t)
            case ScalarKind::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
#undef SWITCH_CASE
        }

        template <typename F>
        auto ApplyElemwiseBinaryOp(const ConstValue& other, F f) const -> ConstValue
        {
#define SWITCH_CASE(CPPTYPE)                                                                                           \
    if constexpr (requires(CPPTYPE x) { f(x, x); }) {                                                                  \
        return ApplyElemwiseBinaryOpUnsafe<CPPTYPE>(other, f);                                                         \
    }                                                                                                                  \
    else {                                                                                                             \
        return ConstValue();                                                                                           \
    }
            switch (static_cast<ScalarKind>(scalarType)) {
            case ScalarKind::Bool:
                SWITCH_CASE(bool)
            case ScalarKind::Int:
                SWITCH_CASE(int32_t)
            case ScalarKind::Uint:
                SWITCH_CASE(uint32_t)
            case ScalarKind::Float:
                SWITCH_CASE(float)
            case ScalarKind::Double:
                SWITCH_CASE(double)
            case ScalarKind::Int8:
                SWITCH_CASE(int8_t)
            case ScalarKind::Int16:
                SWITCH_CASE(int16_t)
            case ScalarKind::Int64:
                SWITCH_CASE(int64_t)
            case ScalarKind::Uint8:
                SWITCH_CASE(int8_t)
            case ScalarKind::Uint16:
                SWITCH_CASE(uint16_t)
            case ScalarKind::Uint64:
                SWITCH_CASE(uint64_t)
            case ScalarKind::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
#undef SWITCH_CASE
        }

        template <typename F>
        auto ApplyElemwiseComparisonOp(const ConstValue& other, F f) const -> ConstValue
        {
#define SWITCH_CASE(CPPTYPE)                                                                                           \
    if constexpr (requires(CPPTYPE x) { f(x, x); }) {                                                                  \
        return ApplyElemwiseComparisonOpUnsafe<CPPTYPE>(other, f);                                                     \
    }                                                                                                                  \
    else {                                                                                                             \
        return ConstValue();                                                                                           \
    }
            switch (static_cast<ScalarKind>(scalarType)) {
            case ScalarKind::Bool:
                SWITCH_CASE(bool)
            case ScalarKind::Int:
                SWITCH_CASE(int32_t)
            case ScalarKind::Uint:
                SWITCH_CASE(uint32_t)
            case ScalarKind::Float:
                SWITCH_CASE(float)
            case ScalarKind::Double:
                SWITCH_CASE(double)
            case ScalarKind::Int8:
                SWITCH_CASE(int8_t)
            case ScalarKind::Int16:
                SWITCH_CASE(int16_t)
            case ScalarKind::Int64:
                SWITCH_CASE(int64_t)
            case ScalarKind::Uint8:
                SWITCH_CASE(int8_t)
            case ScalarKind::Uint16:
                SWITCH_CASE(uint16_t)
            case ScalarKind::Uint64:
                SWITCH_CASE(uint64_t)
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

        auto GetBufferPtr() const noexcept -> const std::byte*
        {
            if (UseHeapBuffer()) {
                return bufferPtr;
            }
            else {
                return &localBuffer[0];
            }
        }

        auto UseHeapBuffer() const noexcept -> bool
        {
            return GetBufferSize() > sizeof(localBuffer);
        }

        // Note this doesn't release the memory of existing heap pointer
        auto InitializeAsError() -> void
        {
            scalarType = 0;
            arraySize  = 0;
            rowSize    = 0;
            colSize    = 0;
            std::ranges::fill(localBuffer, std::byte(0));
        }

        // Note this doesn't release the memory of existing heap pointer
        auto InitializeAsBlob(ScalarKind scalarType, int16_t rowSize, int16_t colSize) -> ArraySpan<std::byte>;

        // Note this doesn't release the memory of existing heap pointer
        template <typename T>
        auto InitializeAs(int16_t rowSize, int16_t colSize) -> ArraySpan<T>
        {
            auto blob = InitializeAsBlob(GetScalarKindFromCppType<T>(), rowSize, colSize);
            return ArraySpan<T>(reinterpret_cast<T*>(blob.data()), arraySize);
        }
    };

    auto ParseNumberLiteral(StringView literalText) -> ConstValue;
} // namespace glsld