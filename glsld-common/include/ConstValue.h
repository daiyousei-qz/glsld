#pragma once
#include "Semantic.h"
#include "Typing.h"
#include "VecMat.h"

#include <array>
#include <cstdint>
#include <variant>
#include <type_traits>

namespace glsld
{
    class ConstValue
    {
    public:
        ConstValue() : scalarType(0), arraySize(0), rowSize(0), colSize(0), localBuffer()
        {
        }
        ~ConstValue()
        {
            if (UseHeapBuffer()) {
                GLSLD_ASSERT(bufferPtr != nullptr);
                free(bufferPtr);
                bufferPtr = nullptr;
            }
        }

        ConstValue(const ConstValue& other)            = delete;
        ConstValue& operator=(const ConstValue& other) = delete;

        ConstValue(ConstValue&& other) noexcept
            : scalarType(other.scalarType), arraySize(other.arraySize), rowSize(other.rowSize), colSize(other.colSize),
              localBuffer()
        {
            bufferPtr        = other.bufferPtr;
            other.scalarType = 0;
            other.arraySize  = 0;
            other.rowSize    = 0;
            other.colSize    = 0;
            other.bufferPtr  = nullptr;
        }
        ConstValue& operator=(ConstValue&& other) noexcept
        {
            if (UseHeapBuffer()) {
                GLSLD_ASSERT(bufferPtr != nullptr);
                free(bufferPtr);
            }

            scalarType       = other.scalarType;
            arraySize        = other.arraySize;
            rowSize          = other.rowSize;
            colSize          = other.colSize;
            bufferPtr        = other.bufferPtr;
            other.scalarType = 0;
            other.arraySize  = 0;
            other.rowSize    = 0;
            other.colSize    = 0;
            other.bufferPtr  = nullptr;
            return *this;
        }

        template <typename T>
        static auto FromValue(const T& value) -> ConstValue
        {
            ConstValue result;
            if constexpr (std::is_same_v<T, bool>) {
                result.InitializeAs<bool>(ScalarType::Bool, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int32_t>) {
                result.InitializeAs<int32_t>(ScalarType::Int, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint32_t>) {
                result.InitializeAs<uint32_t>(ScalarType::Uint, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, float>) {
                result.InitializeAs<float>(ScalarType::Float, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, double>) {
                result.InitializeAs<double>(ScalarType::Double, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int8_t>) {
                result.InitializeAs<int8_t>(ScalarType::Int8, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint8_t>) {
                result.InitializeAs<uint8_t>(ScalarType::Uint8, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int16_t>) {
                result.InitializeAs<int16_t>(ScalarType::Int16, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint16_t>) {
                result.InitializeAs<uint16_t>(ScalarType::Uint16, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                result.InitializeAs<int64_t>(ScalarType::Int64, 1, 1, 1)[0] = value;
            }
            else if constexpr (std::is_same_v<T, uint64_t>) {
                result.InitializeAs<uint64_t>(ScalarType::Uint64, 1, 1, 1)[0] = value;
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

        auto IsVectorOrMatrix() const noexcept -> bool
        {
            return arraySize > 1;
        }

        auto GetScalarType() const noexcept -> ScalarType
        {
            return static_cast<ScalarType>(scalarType);
        }
        auto IsScalarBool() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarType::Bool;
        }
        auto IsScalarInt32() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarType::Int;
        }
        auto IsScalarUInt32() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarType::Uint;
        }
        auto IsScalarFloat() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarType::Float;
        }
        auto IsScalarDouble() const noexcept -> bool
        {
            return IsScalar() && GetScalarType() == ScalarType::Double;
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
            switch (static_cast<ScalarType>(scalarType)) {
            case ScalarType::Bool:
            case ScalarType::Int8:
            case ScalarType::Uint8:
                return 1;
            case ScalarType::Int16:
            case ScalarType::Uint16:
            case ScalarType::Float16:
                return 2;
            case ScalarType::Int:
            case ScalarType::Uint:
            case ScalarType::Float:
                return 4;
            case ScalarType::Double:
            case ScalarType::Int64:
            case ScalarType::Uint64:
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
            case ScalarType::Bool:
                return GetBoolValue() ? "true" : "false";
            case ScalarType::Int:
                return std::to_string(GetInt32Value());
            case ScalarType::Uint:
                return std::to_string(GetUInt32Value());
            case ScalarType::Float:
                return std::to_string(GetFloatValue());
            case ScalarType::Double:
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
                result.bufferPtr = malloc(arraySize * GetScalarSize());
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
            if (GetScalarType() != ScalarType::Bool) {
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
            if (GetScalarType() == ScalarType::Bool) {
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
            if (GetScalarType() != ScalarType::Bool) {
                return ApplyElemwiseBinaryOp(other, std::plus<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseMinus(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarType::Bool) {
                return ApplyElemwiseBinaryOp(other, std::minus<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseMul(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarType::Bool) {
                return ApplyElemwiseBinaryOp(other, std::multiplies<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseDiv(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarType::Bool) {
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
            if (GetScalarType() == ScalarType::Bool) {
                return ApplyElemwiseBinaryOp(other, std::logical_and<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLogicalOr(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() == ScalarType::Bool) {
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

            if (GetScalarType() == ScalarType::Bool) {
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
            if (GetScalarType() != ScalarType::Bool) {
                return ApplyElemwiseComparisonOp(other, std::less<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseLessThanEq(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarType::Bool) {
                return ApplyElemwiseComparisonOp(other, std::less_equal<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseGreaterThan(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarType::Bool) {
                return ApplyElemwiseComparisonOp(other, std::greater<>{});
            }
            else {
                return ConstValue();
            }
        }
        auto ElemwiseGreaterThanEq(const ConstValue& other) const -> ConstValue
        {
            if (GetScalarType() != ScalarType::Bool) {
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
            auto dstBuffer  = result.InitializeAs<bool>(ScalarType::Bool, arraySize, rowSize, colSize);

            for (int i = 0; i < arraySize; ++i) {
                dstBuffer[i] = f(srcBuffer1[i], srcBuffer2[i]);
            }
            return result;
        }

        template <typename F>
        auto ApplyElemwiseUnaryOp(F f) const -> ConstValue
        {
            switch (static_cast<ScalarType>(scalarType)) {
#if GLSLD_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4804)
#endif
            case ScalarType::Bool:
                if constexpr (requires(bool x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<bool>(f);
                }
                else {
                    return ConstValue();
                }
#if GLSLD_COMPILER_MSVC
#pragma warning(pop)
#endif
            case ScalarType::Int:
                if constexpr (requires(int32_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int32_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint:
                if constexpr (requires(uint32_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint32_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Float:
                if constexpr (requires(float x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<float>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Double:
                if constexpr (requires(double x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<double>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int8:
                if constexpr (requires(int8_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int8_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int16:
                if constexpr (requires(int16_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int16_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int64:
                if constexpr (requires(int64_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<int64_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint8:
                if constexpr (requires(uint8_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint8_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint16:
                if constexpr (requires(uint16_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint16_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint64:
                if constexpr (requires(uint64_t x) { f(x); }) {
                    return ApplyElemwiseUnaryOpUnsafe<uint64_t>(f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
        }

        template <typename F>
        auto ApplyElemwiseBinaryOp(const ConstValue& other, F f) const -> ConstValue
        {
            switch (static_cast<ScalarType>(scalarType)) {
#if GLSLD_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4804)
#endif
            case ScalarType::Bool:
                if constexpr (requires(bool x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<bool>(other, f);
                }
                else {
                    return ConstValue();
                }
#if GLSLD_COMPILER_MSVC
#pragma warning(pop)
#endif
            case ScalarType::Int:
                if constexpr (requires(int32_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint:
                if constexpr (requires(uint32_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Float:
                if constexpr (requires(float x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<float>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Double:
                if constexpr (requires(double x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<double>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int8:
                if constexpr (requires(int8_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int16:
                if constexpr (requires(int16_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int64:
                if constexpr (requires(int64_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<int64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint8:
                if constexpr (requires(uint8_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint16:
                if constexpr (requires(uint16_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint64:
                if constexpr (requires(uint64_t x) { f(x, x); }) {
                    return ApplyElemwiseBinaryOpUnsafe<uint64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Float16:
                GLSLD_NO_IMPL();
            default:
                return ConstValue();
            }
        }

        template <typename F>
        auto ApplyElemwiseComparisonOp(const ConstValue& other, F f) const -> ConstValue
        {
            switch (static_cast<ScalarType>(scalarType)) {
            case ScalarType::Bool:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<bool>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<bool>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int32_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint32_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint32_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Float:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<float>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<float>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Double:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<double>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<double>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int8:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int8_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int16:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int16_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Int64:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<int64_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<int64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint8:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint8_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint8_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint16:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint16_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint16_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Uint64:
                if constexpr (requires { ApplyElemwiseComparisonOpUnsafe<uint64_t>(other, f); }) {
                    return ApplyElemwiseComparisonOpUnsafe<uint64_t>(other, f);
                }
                else {
                    return ConstValue();
                }
            case ScalarType::Float16:
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

        template <typename T>
        auto InitializeAs(ScalarType scalarType, int16_t arraySize, int16_t rowSize, int16_t colSize) -> ArraySpan<T>
        {
            GLSLD_ASSERT(IsError() && arraySize > 0 && arraySize == rowSize * colSize);

            this->scalarType = static_cast<int16_t>(scalarType);
            this->arraySize  = arraySize;
            this->rowSize    = rowSize;
            this->colSize    = colSize;

            GLSLD_ASSERT(GetScalarSize() == sizeof(T));
            void* buffer = nullptr;
            if (UseHeapBuffer()) {
                bufferPtr = std::malloc(GetBufferSize());
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

        int16_t scalarType;
        int16_t arraySize;
        int16_t rowSize;
        int16_t colSize;

        union
        {
            void* bufferPtr;
            alignas(8) uint8_t localBuffer[8];
        };
    };

    auto EvalUnaryConstExpr(UnaryOp op, const ConstValue& operand) -> ConstValue;
    auto EvalBinaryConstExpr(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue;
    auto EvalSelectConstExpr(const ConstValue& predicate, const ConstValue& ifBranchVal,
                             const ConstValue& elseBranchVal) -> ConstValue;
} // namespace glsld