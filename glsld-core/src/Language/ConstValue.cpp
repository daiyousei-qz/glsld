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
    // Assuming F is applicable to the operand, this function applies the operator in an element-wise manner and returns
    // the result.
    template <typename SrcScalarType, typename DstScalarType, typename F>
    static auto ApplyElemwiseUnaryOpUnsafe(const ConstValue& operand, F f) -> ConstValue
    {
        GLSLD_ASSERT(operand.GetScalarKind() == GetScalarKindFromCppType<SrcScalarType>());

        auto srcBuffer = operand.GetBufferAs<SrcScalarType>();

        ConstValue result{GetScalarKindFromCppType<DstScalarType>(), static_cast<int16_t>(operand.GetRowSize()),
                          static_cast<int16_t>(operand.GetColumnSize())};
        auto dstBuffer = result.GetMutableBufferAs<DstScalarType>();

        for (int i = 0; i < dstBuffer.size(); ++i) {
            dstBuffer[i] = f(srcBuffer[i]);
        }

        return result;
    }

    // Assuming F is applicable to the operands, this function applies the operator in an element-wise manner and
    // returns the result.
    template <typename SrcScalarType, typename DstScalarType, typename F>
    static auto ApplyElemwiseBinaryOpUnsafe(const ConstValue& lhs, const ConstValue& rhs, F f) -> ConstValue
    {
        GLSLD_ASSERT(lhs.GetScalarKind() == GetScalarKindFromCppType<SrcScalarType>());
        GLSLD_ASSERT(rhs.GetScalarKind() == GetScalarKindFromCppType<SrcScalarType>());

        auto lhsBuffer = lhs.GetBufferAs<SrcScalarType>();
        auto rhsBuffer = rhs.GetBufferAs<SrcScalarType>();

        // Case 1: both operands have the same shape, we apply the operator element-wise.
        if (lhs.GetColumnSize() == rhs.GetColumnSize() && lhs.GetRowSize() == rhs.GetRowSize()) {
            ConstValue result{GetScalarKindFromCppType<DstScalarType>(), static_cast<int16_t>(lhs.GetRowSize()),
                              static_cast<int16_t>(lhs.GetColumnSize())};
            ArraySpan<DstScalarType> dstBuffer = result.GetMutableBufferAs<DstScalarType>();

            for (int i = 0; i < dstBuffer.size(); ++i) {
                dstBuffer[i] = f(lhsBuffer[i], rhsBuffer[i]);
            }

            return result;
        }
        // Case 2: `<scalar> <op> <vector/matrix>`, we broadcast the scalar to all elements and apply the operator.
        else if (lhs.IsScalar()) {
            GLSLD_ASSERT(!rhs.IsScalar());

            ConstValue result{GetScalarKindFromCppType<DstScalarType>(), static_cast<int16_t>(rhs.GetRowSize()),
                              static_cast<int16_t>(rhs.GetColumnSize())};
            ArraySpan<DstScalarType> dstBuffer = result.GetMutableBufferAs<DstScalarType>();

            for (int i = 0; i < dstBuffer.size(); ++i) {
                dstBuffer[i] = f(lhsBuffer[0], rhsBuffer[i]);
            }

            return result;
        }
        // Case 3: `<vector/matrix> <op> <scalar>`, we broadcast the scalar to all elements and apply the operator.
        else if (rhs.IsScalar()) {
            GLSLD_ASSERT(!lhs.IsScalar());

            ConstValue result{GetScalarKindFromCppType<DstScalarType>(), static_cast<int16_t>(lhs.GetRowSize()),
                              static_cast<int16_t>(lhs.GetColumnSize())};
            ArraySpan<DstScalarType> dstBuffer = result.GetMutableBufferAs<DstScalarType>();

            for (int i = 0; i < dstBuffer.size(); ++i) {
                dstBuffer[i] = f(lhsBuffer[i], rhsBuffer[0]);
            }

            return result;
        }
        // Case 4: Incompatible shapes
        else {
            return ConstValue{};
        }
    }

    template <typename DstType>
    auto ApplyElemwiseCast(const ConstValue& operand) -> ConstValue
    {
        switch (operand.GetScalarKind()) {
        case ScalarKind::Bool:
            return ApplyElemwiseUnaryOpUnsafe<bool, DstType>(operand, [](bool x) { return static_cast<DstType>(x); });
        case ScalarKind::Int:
            return ApplyElemwiseUnaryOpUnsafe<int32_t, DstType>(operand,
                                                                [](int32_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Uint:
            return ApplyElemwiseUnaryOpUnsafe<uint32_t, DstType>(operand,
                                                                 [](uint32_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Float:
            return ApplyElemwiseUnaryOpUnsafe<float, DstType>(operand, [](float x) { return static_cast<DstType>(x); });
        case ScalarKind::Double:
            return ApplyElemwiseUnaryOpUnsafe<double, DstType>(operand,
                                                               [](double x) { return static_cast<DstType>(x); });
        case ScalarKind::Int8:
            return ApplyElemwiseUnaryOpUnsafe<int8_t, DstType>(operand,
                                                               [](int8_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Int16:
            return ApplyElemwiseUnaryOpUnsafe<int16_t, DstType>(operand,
                                                                [](int16_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Int64:
            return ApplyElemwiseUnaryOpUnsafe<int64_t, DstType>(operand,
                                                                [](int64_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Uint8:
            return ApplyElemwiseUnaryOpUnsafe<uint8_t, DstType>(operand,
                                                                [](uint8_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Uint16:
            return ApplyElemwiseUnaryOpUnsafe<uint16_t, DstType>(operand,
                                                                 [](uint16_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Uint64:
            return ApplyElemwiseUnaryOpUnsafe<uint64_t, DstType>(operand,
                                                                 [](uint64_t x) { return static_cast<DstType>(x); });
        case ScalarKind::Float16:
            // FIXME: implement this
            return ConstValue{};
        default:
            return ConstValue{};
        }
    }

    template <typename F>
    static auto ApplyElemwiseUnaryOp(const ConstValue& operand, F f) -> ConstValue
    {
#define SWITCH_CASE(CPPTYPE)                                                                                           \
    if constexpr (requires(CPPTYPE x) { f(x); }) {                                                                     \
        return ApplyElemwiseUnaryOpUnsafe<CPPTYPE, CPPTYPE>(operand, f);                                               \
    }                                                                                                                  \
    else {                                                                                                             \
        return ConstValue();                                                                                           \
    }
        switch (static_cast<ScalarKind>(operand.GetScalarKind())) {
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
            SWITCH_CASE(uint8_t)
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
    static auto ApplyElemwiseBinaryOp(const ConstValue& lhs, const ConstValue& rhs, F f) -> ConstValue
    {
        if (lhs.GetScalarKind() != rhs.GetScalarKind()) {
            // Incompatible scalar kinds
            return ConstValue{};
        }

#define SWITCH_CASE(CPPTYPE)                                                                                           \
    if constexpr (requires(CPPTYPE x) { f(x, x); }) {                                                                  \
        return ApplyElemwiseBinaryOpUnsafe<CPPTYPE, CPPTYPE>(lhs, rhs, f);                                             \
    }                                                                                                                  \
    else {                                                                                                             \
        break;                                                                                                         \
    }
        switch (lhs.GetScalarKind()) {
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
            SWITCH_CASE(uint8_t)
        case ScalarKind::Uint16:
            SWITCH_CASE(uint16_t)
        case ScalarKind::Uint64:
            SWITCH_CASE(uint64_t)
        case ScalarKind::Float16:
            // FIXME: implement this
            break;
        }
#undef SWITCH_CASE

        return ConstValue{};
    }

    template <typename F>
    static auto ApplyElemwiseComparisonOp(const ConstValue& lhs, const ConstValue& rhs, F f) -> ConstValue
    {
        if (lhs.GetScalarKind() != rhs.GetScalarKind()) {
            // Incompatible scalar kinds
            return ConstValue{};
        }
        if (lhs.GetRowSize() != rhs.GetRowSize() || lhs.GetColumnSize() != rhs.GetColumnSize()) {
            // Incompatible shapes
            return ConstValue{};
        }

#define SWITCH_CASE(CPPTYPE)                                                                                           \
    if constexpr (requires(CPPTYPE x) { f(x, x); }) {                                                                  \
        return ApplyElemwiseBinaryOpUnsafe<CPPTYPE, bool>(lhs, rhs, f);                                                \
    }                                                                                                                  \
    else {                                                                                                             \
        break;                                                                                                         \
    }
        switch (lhs.GetScalarKind()) {
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
            SWITCH_CASE(uint8_t)
        case ScalarKind::Uint16:
            SWITCH_CASE(uint16_t)
        case ScalarKind::Uint64:
            SWITCH_CASE(uint64_t)
        case ScalarKind::Float16:
            // FIXME: implement this
            break;
        }
#undef SWITCH_CASE

        return ConstValue{};
    }

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

    auto ConstValue::ComposeVector(ArrayView<ConstValue> values, ScalarKind kind, int dimSize) -> ConstValue
    {
        // We don't distinguish between vector and matrix here. So we just treat it as a matrix.
        return ComposeMatrix(values, kind, 1, dimSize);
    }
    auto ConstValue::ComposeMatrix(ArrayView<ConstValue> values, ScalarKind kind, int rowSize, int colSize)
        -> ConstValue
    {
        int totalScalarCount = 0;
        for (const auto& value : values) {
            totalScalarCount += value.GetArraySize();
        }

        if (totalScalarCount != rowSize * colSize) {
            // Wrong number of scalars
            return ConstValue();
        }

        ConstValue result;
        auto blob = result.InitializeAsBlob(kind, rowSize, colSize);

        int offset = 0;
        for (const auto& value : values) {
            // FIXME: optimize this to avoid so many temporary objects
            std::ranges::copy(value.CastScalar(kind).GetBufferAsBlob(), blob.begin() + offset);
            offset += value.GetBufferSize();
        }

        return result;
    }
    auto ConstValue::ConstructScalar(const ConstValue& value, ScalarKind kind) -> ConstValue
    {
        if (value.IsScalar()) {
            return value.CastScalar(kind);
        }
        else if (value.IsVector()) {
            return value.GetElement(0).CastScalar(kind);
        }
        else if (value.IsMatrix()) {
            return value.GetElement(0).GetElement(0).CastScalar(kind);
        }
        else {
            return ConstValue();
        }
    }
    auto ConstValue::ConstructVector(const ConstValue& value, ScalarKind kind, int dimSize) -> ConstValue
    {
        if (value.IsScalar()) {
            ConstValue result;
            auto blob = result.InitializeAsBlob(value.GetScalarKind(), 1, dimSize);
            for (int i = 0; i < dimSize; ++i) {
                std::ranges::copy(value.GetBufferAsBlob(), blob.begin() + i * value.GetBufferSize());
            }

            if (value.GetScalarKind() != kind) {
                return result.CastScalar(kind);
            }
            else {
                return result;
            }
        }
        else if (value.IsVector() && value.GetColumnSize() == dimSize) {
            return value.CastScalar(kind);
        }
        else if (value.IsMatrix() && value.GetArraySize() == dimSize) {
            return ComposeVector({&value, 1}, kind, dimSize);
        }

        return ConstValue();
    }
    auto ConstValue::ConstructMatrix(const ConstValue& value, ScalarKind kind, int rolSize, int colSize) -> ConstValue
    {
        if (value.IsScalar()) {
            ConstValue result;
            auto blob = result.InitializeAsBlob(value.GetScalarKind(), rolSize, colSize);
            for (int i = 0; i < rolSize; ++i) {
                for (int j = 0; j < colSize; ++j) {
                    if (i == j) {
                        std::ranges::copy(value.GetBufferAsBlob(),
                                          blob.begin() + (i * colSize + j) * value.GetBufferSize());
                    }
                    else {
                        std::fill(blob.begin() + (i * colSize + j) * value.GetBufferSize(),
                                  blob.begin() + (i * colSize + j + 1) * value.GetBufferSize(), std::byte{0});
                    }
                }
            }

            if (value.GetScalarKind() != kind) {
                return result.CastScalar(kind);
            }
            else {
                return result;
            }
        }
        else if (value.IsVector() && value.GetArraySize() == rolSize * colSize) {
            return ComposeMatrix({&value, 1}, kind, rolSize, colSize);
        }
        else if (value.IsMatrix()) {
            ConstValue one = ConstValue::CreateScalar(1).CastScalar(value.GetScalarKind());
            ConstValue result;
            auto blob = result.InitializeAsBlob(value.GetScalarKind(), rolSize, colSize);
            for (int i = 0; i < rolSize; ++i) {
                for (int j = 0; j < colSize; ++j) {
                    if (i < value.GetRowSize() && j < value.GetColumnSize()) {
                        std::ranges::copy(value.GetElementAsBlob(i, j),
                                          blob.begin() + (i * colSize + j) * one.GetBufferSize());
                    }
                    else if (i == j) {
                        // Fill diagonal with ones
                        std::ranges::copy(one.GetBufferAsBlob(),
                                          blob.begin() + (i * colSize + j) * one.GetBufferSize());
                    }
                    else {
                        // Fill other elements with zeros
                        std::fill(blob.begin() + (i * colSize + j) * one.GetBufferSize(),
                                  blob.begin() + (i * colSize + j + 1) * one.GetBufferSize(), std::byte{0});
                    }
                }
            }

            if (value.GetScalarKind() != kind) {
                return result.CastScalar(kind);
            }
            else {
                return result;
            }
        }

        return ConstValue();
    }

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

        std::string result;
        if (IsVector()) {
            switch (GetScalarKind()) {
            case ScalarKind::Bool:
                result += "bvec";
                break;
            case ScalarKind::Int:
                result += "ivec";
                break;
            case ScalarKind::Uint:
                result += "uvec";
                break;
            case ScalarKind::Float:
                result += "vec";
                break;
            case ScalarKind::Double:
                result += "dvec";
                break;
            case ScalarKind::Int8:
                result += "i8vec";
                break;
            case ScalarKind::Uint8:
                result += "u8vec";
                break;
            case ScalarKind::Int16:
                result += "i16vec";
                break;
            case ScalarKind::Uint16:
                result += "u16vec";
                break;
            case ScalarKind::Int64:
                result += "i64vec";
                break;
            case ScalarKind::Uint64:
                result += "u64vec";
                break;
            case ScalarKind::Float16:
                result += "f16vec";
                break;
            default:
                return "<vector>";
            }
            result += fmt::to_string(GetArraySize());
            result += "(";
        }
        else if (IsMatrix()) {
            switch (GetScalarKind()) {
            case ScalarKind::Bool:
                result += "bmat";
                break;
            case ScalarKind::Int:
                result += "imat";
                break;
            case ScalarKind::Uint:
                result += "umat";
                break;
            case ScalarKind::Float:
                result += "mat";
                break;
            case ScalarKind::Double:
                result += "dmat";
                break;
            case ScalarKind::Int8:
                result += "i8mat";
                break;
            case ScalarKind::Uint8:
                result += "u8mat";
                break;
            case ScalarKind::Int16:
                result += "i16mat";
                break;
            case ScalarKind::Uint16:
                result += "u16mat";
                break;
            case ScalarKind::Int64:
                result += "i64mat";
                break;
            case ScalarKind::Uint64:
                result += "u64mat";
                break;
            case ScalarKind::Float16:
                result += "f16mat";
                break;
            default:
                return "<matrix>";
            }

            result += fmt::to_string(GetRowSize());
            result += "x";
            result += fmt::to_string(GetColumnSize());
            result += "(";
        }

        for (int index = 0; index < GetArraySize(); ++index) {
            if (index > 0) {
                result += ", ";
            }

            switch (GetScalarKind()) {
            case ScalarKind::Bool:
                result += GetBufferAs<bool>()[0] ? "true" : "false";
                break;
            case ScalarKind::Int:
                result += fmt::to_string(GetBufferAs<int32_t>()[index]);
                break;
            case ScalarKind::Uint:
                result += fmt::to_string(GetBufferAs<uint32_t>()[index]);
                break;
            case ScalarKind::Float:
                result += fmt::to_string(GetBufferAs<float>()[index]);
                break;
            case ScalarKind::Double:
                result += fmt::to_string(GetBufferAs<double>()[index]);
                break;
            case ScalarKind::Int8:
                result += fmt::to_string(GetBufferAs<int8_t>()[index]);
                break;
            case ScalarKind::Int16:
                result += fmt::to_string(GetBufferAs<int16_t>()[index]);
                break;
            case ScalarKind::Int64:
                result += fmt::to_string(GetBufferAs<int64_t>()[index]);
                break;
            case ScalarKind::Uint8:
                result += fmt::to_string(GetBufferAs<uint8_t>()[index]);
                break;
            case ScalarKind::Uint16:
                result += fmt::to_string(GetBufferAs<uint16_t>()[index]);
                break;
            case ScalarKind::Uint64:
                result += fmt::to_string(GetBufferAs<uint64_t>()[index]);
                break;
            case ScalarKind::Float16:
                result += "<float16>";
            }
        }

        if (IsVectorOrMatrix()) {
            result += ")";
        }

        return result;
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
            return ApplyElemwiseCast<bool>(*this);
        case ScalarKind::Int:
            return ApplyElemwiseCast<int32_t>(*this);
        case ScalarKind::Uint:
            return ApplyElemwiseCast<uint32_t>(*this);
        case ScalarKind::Float:
            return ApplyElemwiseCast<float>(*this);
        case ScalarKind::Double:
            return ApplyElemwiseCast<double>(*this);
        case ScalarKind::Int8:
            return ApplyElemwiseCast<int8_t>(*this);
        case ScalarKind::Uint8:
            return ApplyElemwiseCast<uint8_t>(*this);
        case ScalarKind::Int16:
            return ApplyElemwiseCast<int16_t>(*this);
        case ScalarKind::Uint16:
            return ApplyElemwiseCast<uint16_t>(*this);
        case ScalarKind::Int64:
            return ApplyElemwiseCast<int64_t>(*this);
        case ScalarKind::Uint64:
            return ApplyElemwiseCast<uint64_t>(*this);
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
        if (IsError() || index < 0) {
            return ConstValue{};
        }

        ConstValue result;
        ArraySpan<std::byte> blob;
        if (IsScalar() || IsVector()) {
            blob = result.InitializeAsBlob(GetScalarKind(), 1, 1);
        }
        else if (IsMatrix()) {
            blob = result.InitializeAsBlob(GetScalarKind(), 1, colSize);
        }

        auto elementOffset = index * blob.size();
        if (elementOffset + blob.size() > GetBufferSize()) {
            return ConstValue{};
        }

        std::ranges::copy_n(GetBufferAsBlob().begin() + elementOffset, blob.size(), blob.begin());
        return result;
    }

    auto ConstValue::GetSwizzle(SwizzleDesc swizzle) const -> ConstValue
    {
        // FIXME: verify this is correct
        if (IsError() || !swizzle.IsValid()) {
            return ConstValue{};
        }

        ConstValue result;
        auto blob = result.InitializeAsBlob(GetScalarKind(), rowSize, swizzle.GetDimension());
        for (auto it = blob.begin(); auto index : swizzle.GetIndices()) {
            auto elem = GetElement(index);
            if (elem.IsError()) {
                return ConstValue{};
            }

            it = std::ranges::copy(elem.GetBufferAsBlob(), it).out;
        }

        return result;
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
        return ApplyElemwiseUnaryOp(*this, ExcludingBool<std::negate<>>{});
    }
    auto ConstValue::ElemwiseBitNot() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, ExcludingBoolFloat<std::bit_not<>>{});
    }
    auto ConstValue::ElemwiseLogicalNot() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, std::logical_not<bool>{});
    }

#pragma region Binary
    auto ConstValue::ElemwisePlus(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBool<std::plus<>>{});
    }
    auto ConstValue::ElemwiseMinus(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBool<std::minus<>>{});
    }
    auto ConstValue::ElemwiseMul(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBool<std::multiplies<>>{});
    }
    auto ConstValue::ElemwiseDiv(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBool<std::divides<>>{});
    }
    auto ConstValue::ElemwiseMod(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBoolFloat<std::modulus<>>{});
    }
    auto ConstValue::ElemwiseBitAnd(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBoolFloat<std::bit_and<>>{});
    }
    auto ConstValue::ElemwiseBitOr(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBoolFloat<std::bit_or<>>{});
    }
    auto ConstValue::ElemwiseBitXor(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, ExcludingBoolFloat<std::bit_xor<>>{});
    }
    auto ConstValue::ElemwiseLogicalAnd(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, std::logical_and<bool>{});
    }
    auto ConstValue::ElemwiseLogicalOr(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, std::logical_or<bool>{});
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

        return ApplyElemwiseBinaryOp(*this, other, LogicalXor{});
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
        return ApplyElemwiseComparisonOp(*this, other, std::equal_to<>{});
    }
    auto ConstValue::ElemwiseNotEquals(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(*this, other, std::not_equal_to<>{});
    }
    auto ConstValue::ElemwiseLessThan(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(*this, other, ExcludingBool<std::less<>>{});
    }
    auto ConstValue::ElemwiseLessThanEq(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(*this, other, ExcludingBool<std::less_equal<>>{});
    }
    auto ConstValue::ElemwiseGreaterThan(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(*this, other, ExcludingBool<std::greater<>>{});
    }
    auto ConstValue::ElemwiseGreaterThanEq(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseComparisonOp(*this, other, ExcludingBool<std::greater_equal<>>{});
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
        return ApplyElemwiseUnaryOp(*this, Radians{});
    }
    auto ConstValue::ElemwiseDegrees() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Degrees{});
    }
    auto ConstValue::ElemwiseSin() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Sin{});
    }
    auto ConstValue::ElemwiseCos() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Cos{});
    }
    auto ConstValue::ElemwiseAsin() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Asin{});
    }
    auto ConstValue::ElemwiseAcos() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Acos{});
    }

    auto ConstValue::ElemwisePow(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, Pow{});
    }
    auto ConstValue::ElemwiseExp() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Exp{});
    }
    auto ConstValue::ElemwiseLog() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Log{});
    }
    auto ConstValue::ElemwiseExp2() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Exp2{});
    }
    auto ConstValue::ElemwiseLog2() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Log2{});
    }
    auto ConstValue::ElemwiseSqrt() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Sqrt{});
    }
    auto ConstValue::ElemwiseInverseSqrt() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, InverseSqrt{});
    }
    auto ConstValue::ElemwiseAbs() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Abs{});
    }
    auto ConstValue::ElemwiseSign() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Sign{});
    }
    auto ConstValue::ElemwiseFloor() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Floor{});
    }
    auto ConstValue::ElemwiseTrunc() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Trunc{});
    }
    auto ConstValue::ElemwiseRound() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Round{});
    }
    auto ConstValue::ElemwiseCeil() const -> ConstValue
    {
        return ApplyElemwiseUnaryOp(*this, Ceil{});
    }
    // auto ConstValue::ElemwiseMod(const ConstValue& other) const -> ConstValue
    // {
    //     return ApplyElemwiseBinaryOp(other, Mod{});
    // }
    auto ConstValue::ElemwiseMin(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, Min{});
    }
    auto ConstValue::ElemwiseMax(const ConstValue& other) const -> ConstValue
    {
        return ApplyElemwiseBinaryOp(*this, other, Max{});
    }
    auto ConstValue::ElemwiseClamp(const ConstValue& min, const ConstValue& max) const -> ConstValue
    {
        return ElemwiseMax(min).ElemwiseMin(max);
    }

#pragma endregion

    auto ConstValue::InitializeAsError() -> void
    {
        scalarType = 0;
        arraySize  = 0;
        rowSize    = 0;
        colSize    = 0;
        std::ranges::fill(localBuffer, std::byte(0));
    }

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

        auto result = ArraySpan<std::byte>(buffer, GetBufferSize());
        std::ranges::fill(result, std::byte(0));
        return result;
    }

    template <std::integral T>
    static auto ParseIntegralLiteral(StringView literalText) -> ConstValue
    {
        T value           = 0;
        const char* start = literalText.data();
        const char* end   = literalText.data() + literalText.size();

        // Determine base
        int base = 10;
        if (literalText.StartWith("0x") || literalText.StartWith("0X")) {
            base = 16;
            start += 2;
        }
        else if (literalText.StartWith("0") && literalText.size() > 1) {
            base = 8;
            start += 1;
        }

        auto parseResult = std::from_chars(start, end, value, base);
        if (parseResult.ptr == end && parseResult.ec == std::errc{}) {
            return ConstValue::CreateScalar<T>(value);
        }

        return ConstValue{};
    }

    template <std::floating_point T>
    static auto ParseFloatLiteral(StringView literalText) -> ConstValue
    {
        T value;
        auto parseResult = std::from_chars(literalText.data(), literalText.data() + literalText.size(), value);
        if (parseResult.ptr == literalText.data() + literalText.size() && parseResult.ec == std::errc{}) {
            return ConstValue::CreateScalar<T>(value);
        }

        return ConstValue{};
    }

    auto ParseNumberLiteral(StringView literalText) -> ConstValue
    {
        // FIXME: Our lexer may be far permissive than what std::from_chars can handle. We should report error for these
        //        cases instead of just returning an error ConstValue.
        bool hasHexPrefix    = literalText.StartWith("0x") || literalText.StartWith("0X");
        bool hasDecimalPoint = literalText.Contains('.');
        bool hasExponent     = !hasHexPrefix && (literalText.Contains('e') || literalText.Contains('E'));

        // FIXME: make sure we don't allow overflow
        if (literalText.EndWith("u") || literalText.EndWith("U")) {
            return ParseIntegralLiteral<uint32_t>(literalText.DropBack(1));
        }
        else if ((hasDecimalPoint || hasExponent) && (literalText.EndWith("lf") || literalText.EndWith("LF"))) {
            return ParseFloatLiteral<double>(literalText.DropBack(2));
        }
        else if ((hasDecimalPoint || hasExponent) && (literalText.EndWith("f") || literalText.EndWith("F"))) {
            return ParseFloatLiteral<float>(literalText.DropBack(1));
        }
        else {
            if (hasDecimalPoint || hasExponent) {
                return ParseFloatLiteral<float>(literalText);
            }
            else {
                return ParseIntegralLiteral<int32_t>(literalText);
            }
        }

        return ConstValue{};
    }
} // namespace glsld