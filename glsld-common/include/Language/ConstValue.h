#pragma once
#include "Basic/Common.h"
#include "Language/Semantic.h"
#include "Language/Typing.h"

#include <cstdint>
#include <optional>
#include <type_traits>

namespace glsld
{
    // Represents a compile-time constant. This can be either:
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

        // We disable copy ctor to avoid accidental heap allocation.
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
        auto operator==(const ConstValue& other) const noexcept -> bool
        {
            if (scalarType != other.scalarType || arraySize != other.arraySize || rowSize != other.rowSize ||
                colSize != other.colSize) {
                return false;
            }

            return std::ranges::equal(GetBufferAsBlob(), other.GetBufferAsBlob());
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

        auto GetGlslType() const noexcept -> std::optional<GlslBuiltinType>;

        auto GetScalarKind() const noexcept -> ScalarKind
        {
            return static_cast<ScalarKind>(scalarType);
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

        auto GetScalarSize() const noexcept -> int;

        auto ToString() const -> std::string;
        auto Clone() const -> ConstValue;

        auto GetElement(int index) -> ConstValue;
        auto GetSwizzle(SwizzleDesc swizzle) -> ConstValue;

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

    private:
        template <typename T, typename F>
        auto ApplyElemwiseUnaryOpUnsafe(F f) const -> ConstValue
        {
            ConstValue result;

            auto srcBuffer = GetBufferAs<T>();
            auto dstBuffer = result.InitializeAs<T>(GetScalarKind(), arraySize, rowSize, colSize);

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
            auto dstBuffer  = result.InitializeAs<T>(GetScalarKind(), arraySize, rowSize, colSize);

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
        auto InitializeAsBlob(ScalarKind scalarType, int16_t arraySize, int16_t rowSize, int16_t colSize)
            -> ArraySpan<std::byte>
        {
            GLSLD_ASSERT(IsError() && arraySize > 0 && arraySize == rowSize * colSize);

            this->scalarType = static_cast<int16_t>(scalarType);
            this->arraySize  = arraySize;
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

            return ArraySpan<std::byte>(buffer, arraySize);
        }

        // Note this doesn't release the memory of existing heap pointer
        template <typename T>
        auto InitializeAs(ScalarKind scalarType, int16_t arraySize, int16_t rowSize, int16_t colSize) -> ArraySpan<T>
        {
            auto blob = InitializeAsBlob(scalarType, arraySize, rowSize, colSize);
            GLSLD_ASSERT(GetScalarSize() == sizeof(T));
            return ArraySpan<T>(reinterpret_cast<T*>(blob.data()), arraySize);
        }

        auto GetBufferAsBlob() const -> ArrayView<std::byte>
        {
            return ArrayView<std::byte>(GetBufferPtr(), GetBufferSize());
        }

        template <typename T>
        auto GetBufferAs() const -> ArrayView<T>
        {
            GLSLD_ASSERT(GetScalarSize() == sizeof(T));
            return ArrayView<T>(reinterpret_cast<const T*>(GetBufferPtr()), arraySize);
        }
    };

    auto EvalUnaryConstExpr(UnaryOp op, const ConstValue& operand) -> ConstValue;
    auto EvalBinaryConstExpr(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue;
    auto EvalSelectConstExpr(const ConstValue& predicate, const ConstValue& ifBranchVal,
                             const ConstValue& elseBranchVal) -> ConstValue;

    auto ParseNumberLiteral(StringView literalText) -> ConstValue;
} // namespace glsld