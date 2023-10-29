#pragma once
#include "Basic/Common.h"

#include <optional>
#include <variant>
#include <vector>

namespace glsld
{
    class Type;

    class AstStructDecl;

    enum class ScalarKind
    {
        // Base language
        Bool,
        Int,
        Uint,
        Float,
        Double,

        // Extension
        Int8,
        Int16,
        Int64,
        Uint8,
        Uint16,
        Uint64,
        Float16,
    };

    inline auto GetLeastPromotedScalarType(ScalarKind lhs, ScalarKind rhs) -> std::optional<ScalarKind>
    {
        if (lhs == rhs) {
            return lhs;
        }

        if (lhs == ScalarKind::Double || rhs == ScalarKind::Double) {
            return ScalarKind::Double;
        }
        else if (lhs == ScalarKind::Float || rhs == ScalarKind::Float) {
            return ScalarKind::Float;
        }
        else if (lhs == ScalarKind::Int64 || rhs == ScalarKind::Int64) {
            return ScalarKind::Int64;
        }
        else if (lhs == ScalarKind::Uint64 || rhs == ScalarKind::Uint64) {
            return ScalarKind::Uint64;
        }
        else if (lhs == ScalarKind::Int || rhs == ScalarKind::Int) {
            return ScalarKind::Int;
        }
        else if (lhs == ScalarKind::Uint || rhs == ScalarKind::Uint) {
            return ScalarKind::Uint;
        }
        else if (lhs == ScalarKind::Int16 || rhs == ScalarKind::Int16) {
            return ScalarKind::Int16;
        }
        else if (lhs == ScalarKind::Uint16 || rhs == ScalarKind::Uint16) {
            return ScalarKind::Uint16;
        }
        else if (lhs == ScalarKind::Int8 || rhs == ScalarKind::Int8) {
            return ScalarKind::Int8;
        }
        else if (lhs == ScalarKind::Uint8 || rhs == ScalarKind::Uint8) {
            return ScalarKind::Uint8;
        }
        else if (lhs == ScalarKind::Float16 || rhs == ScalarKind::Float16) {
            return ScalarKind::Float16;
        }
        else {
            return std::nullopt;
        }
    }

    // Excluding bool type
    inline auto IsIntegralScalarType(ScalarKind type) -> bool
    {
        switch (type) {
        case ScalarKind::Int:
        case ScalarKind::Uint:
        case ScalarKind::Int8:
        case ScalarKind::Int16:
        case ScalarKind::Int64:
        case ScalarKind::Uint8:
        case ScalarKind::Uint16:
        case ScalarKind::Uint64:
            return true;
        default:
            return false;
        }
    }

    inline auto IsFloatPointScalarType(ScalarKind type) -> bool
    {
        switch (type) {
        case ScalarKind::Float:
        case ScalarKind::Double:
        case ScalarKind::Float16:
            return true;
        default:
            return false;
        }
    }

    // An enum of all builtin types in glsl language
    enum class GlslBuiltinType
    {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, ...) Ty_##GLSL_TYPE,
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
    };

    // Bottom type
    struct ErrorTypeDesc
    {
    };

    // Void type
    struct VoidTypeDesc
    {
    };

    struct ScalarTypeDesc
    {
        ScalarKind type;
    };

    // Note this is the column vector
    struct VectorTypeDesc
    {
        ScalarKind scalarType;
        int vectorSize;
    };
    struct MatrixTypeDesc
    {
        // Scalar type of the matrix.
        ScalarKind scalarType;

        // Dimension of the row vector, i.e. the number of columns.
        int dimRow;

        // Dimension of the columns vector, i.e. the number of rows.
        int dimCol;
    };

    enum class SamplingDim
    {
        One,
        Two,
        Three,
        Rect,
        Cube,
        Buffer,
    };
    struct SamplerTypeDesc
    {
        SamplingDim dim;
    };

    struct ArrayTypeDesc
    {
        const Type* elementType;

        // NOTE dim size of 0 means unsized/error-sized dimension
        std::vector<size_t> dimSizes;
    };

    class AstDecl;
    struct StructTypeDesc
    {
        std::string name;
        std::vector<std::pair<std::string, const Type*>> members;
        const AstDecl* decl;
    };

    struct FunctionTypeDesc
    {
    };

    struct ValueDimension
    {
        int dimRow;
        int dimCol;

        constexpr auto operator==(const ValueDimension& rhs) const noexcept -> bool
        {
            return dimRow == rhs.dimRow && dimCol == rhs.dimCol;
        }
    };

    // Type is a wrapper of type descriptor
    // For types except arrays and structs, we have globally unique type instances
    // For array and struct typs, the type instances are managed by AstContext
    class Type
    {
    public:
        // Do we need function type?
        using DescPayloadType = std::variant<ErrorTypeDesc, VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc,
                                             MatrixTypeDesc, SamplerTypeDesc, ArrayTypeDesc, StructTypeDesc>;

    private:
        // FIXME: is this for debug only?
        std::string debugName = "<FIXME>";

        DescPayloadType descPayload;

        bool containsOpaqueType = false;

    public:
        Type(std::string name, DescPayloadType payload) : debugName(std::move(name)), descPayload(std::move(payload))
        {
            if (IsSampler()) {
                containsOpaqueType = true;
            }
            else if (auto desc = GetArrayDesc()) {
                containsOpaqueType = desc->elementType->containsOpaqueType;
            }
            else if (auto desc = GetStructDesc()) {
                for (const auto& [_, type] : desc->members) {
                    if (type->containsOpaqueType) {
                        containsOpaqueType = true;
                        break;
                    }
                }
            }
        }

        // Get a globally unique type instance for error type
        static auto GetErrorType() -> const Type*;

        // Get a globally unique type instance for the particular builtin type
        static auto GetBuiltinType(GlslBuiltinType tag) -> const Type*;

        // Get a globally unique type instance for the particular scalar type, if any
        // NOTE if the scalar type doesn't exist in glsl language, returns nullptr
        static auto GetScalarType(ScalarKind kind) -> const Type*;

        // Get a globally unique type instance for the particular vector type, if any
        // NOTE if the vector type doesn't exist in glsl language, returns nullptr
        static auto GetVectorType(ScalarKind kind, size_t dim) -> const Type*;

        static auto GetMatrixType(ScalarKind kind, size_t dimRow, size_t dimCol) -> const Type*;

        auto IsError() const noexcept -> bool
        {
            return std::holds_alternative<ErrorTypeDesc>(descPayload);
        }
        auto IsVoid() const noexcept -> bool
        {
            return std::holds_alternative<VoidTypeDesc>(descPayload);
        }
        auto IsScalar() const noexcept -> bool
        {
            return std::holds_alternative<ScalarTypeDesc>(descPayload);
        }
        auto IsVector() const noexcept -> bool
        {
            return std::holds_alternative<VectorTypeDesc>(descPayload);
        }
        auto IsMatrix() const noexcept -> bool
        {
            return std::holds_alternative<MatrixTypeDesc>(descPayload);
        }
        auto IsSampler() const noexcept -> bool
        {
            return std::holds_alternative<SamplerTypeDesc>(descPayload);
        }
        auto IsArray() const noexcept -> bool
        {
            return std::holds_alternative<ArrayTypeDesc>(descPayload);
        }
        auto IsStruct() const noexcept -> bool
        {
            return std::holds_alternative<StructTypeDesc>(descPayload);
        }

        auto IsScalarBool() const noexcept -> bool
        {
            return this == Type::GetBuiltinType(GlslBuiltinType::Ty_bool);
        }

        auto IsOpaque() const noexcept -> bool
        {
            // Note this is pre-computed to avoid recursion during lookup.
            return containsOpaqueType;
        }

        auto IsComposite() const noexcept -> bool
        {
            return IsArray() || IsStruct();
        }
        auto IsBuiltin() const noexcept -> bool
        {
            return !IsError() && !IsComposite();
        }

        // Arithmetic types support per-component arithmetic operations, including scalar, vector and matrix types.
        auto IsArithmetic() const noexcept -> bool
        {
            return IsScalar() || IsVector() || IsMatrix();
        }

        // Integral types support bitwise operations, including scalar and vector types.
        auto IsIntegral() const noexcept -> bool
        {
            if (auto desc = GetScalarDesc()) {
                return IsIntegralScalarType(desc->type);
            }
            else if (auto desc = GetVectorDesc()) {
                return IsIntegralScalarType(desc->scalarType);
            }

            return false;
        }

        auto GetElementScalarType() const noexcept -> const Type*
        {
            if (auto desc = GetScalarDesc()) {
                return GetScalarType(desc->type);
            }
            else if (auto desc = GetVectorDesc()) {
                return GetScalarType(desc->scalarType);
            }
            else if (auto desc = GetMatrixDesc()) {
                return GetScalarType(desc->scalarType);
            }
            else {
                return nullptr;
            }
        }

        auto GetDimension() const noexcept -> ValueDimension
        {
            if (auto desc = GetVectorDesc()) {
                return {1, desc->vectorSize};
            }
            else if (auto desc = GetMatrixDesc()) {
                return {desc->dimRow, desc->dimCol};
            }
            else {
                return {1, 1};
            }
        }

        auto GetDebugName() const noexcept -> StringView
        {
            return debugName;
        }

        auto GetScalarDesc() const noexcept -> const ScalarTypeDesc*
        {
            return std::get_if<ScalarTypeDesc>(&descPayload);
        }
        auto GetVectorDesc() const noexcept -> const VectorTypeDesc*
        {
            return std::get_if<VectorTypeDesc>(&descPayload);
        }
        auto GetMatrixDesc() const noexcept -> const MatrixTypeDesc*
        {
            return std::get_if<MatrixTypeDesc>(&descPayload);
        }
        auto GetSamplerDesc() const noexcept -> const SamplerTypeDesc*
        {
            return std::get_if<SamplerTypeDesc>(&descPayload);
        }
        auto GetArrayDesc() const noexcept -> const ArrayTypeDesc*
        {
            return std::get_if<ArrayTypeDesc>(&descPayload);
        }
        auto GetStructDesc() const noexcept -> const StructTypeDesc*
        {
            return std::get_if<StructTypeDesc>(&descPayload);
        }

        // Returns true if this type is the same with the given builtin type.
        auto IsSameWith(GlslBuiltinType type) const -> bool;

        // Returns true if this type is the same with the given type.
        auto IsSameWith(const Type* other) const -> bool;

        // True if this type is implicitly convertible to the given type.
        auto IsConvertibleTo(const Type* to) const -> bool;

        auto IsEqualComparableTo(const Type* to) const -> bool
        {
            return !IsOpaque() && !to->IsOpaque() && (IsConvertibleTo(to) || to->IsConvertibleTo(this));
        }
        auto IsOrderingComparableTo(const Type* to) const -> bool
        {
            return !IsScalar() && !to->IsScalar() && (IsConvertibleTo(to) || to->IsConvertibleTo(this));
        }

        // True if conversion from this type to `lhsTo` is better than that to `rhsTo`.
        auto HasBetterConversion(const Type* lhsTo, const Type* rhsTo) const -> bool;
    };

} // namespace glsld
