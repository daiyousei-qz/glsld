#pragma once
#include "Basic/Common.h"

#include <optional>
#include <variant>
#include <vector>

namespace glsld
{
    class Type;

    enum class ScalarType
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

    // Excluding bool type
    inline auto IsIntegralScalarType(ScalarType type) -> bool
    {
        switch (type) {
        case ScalarType::Int:
        case ScalarType::Uint:
        case ScalarType::Int8:
        case ScalarType::Int16:
        case ScalarType::Int64:
        case ScalarType::Uint8:
        case ScalarType::Uint16:
        case ScalarType::Uint64:
            return true;
        default:
            return false;
        }
    }

    inline auto IsFloatPointScalarType(ScalarType type) -> bool
    {
        switch (type) {
        case ScalarType::Float:
        case ScalarType::Double:
        case ScalarType::Float16:
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
        ScalarType type;
    };
    struct VectorTypeDesc
    {
        ScalarType scalarType;
        int vectorSize;
    };
    struct MatrixTypeDesc
    {
        ScalarType scalarType;
        int dimRow;
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
        // FIXME: what should be in a struct type desc?
        AstDecl* decl;
        std::string name;
        std::vector<std::pair<std::string, const Type*>> members;
    };

    struct FunctionTypeDesc
    {
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

        Type(std::string name, DescPayloadType payload) : debugName(std::move(name)), descPayload(payload)
        {
        }

        // Get a globally unique type instance for error type
        static auto GetErrorType() -> const Type*;
        // Get a globally unique type instance for the particular builtin type
        static auto GetBuiltinType(GlslBuiltinType tag) -> const Type*;

        auto IsError() const noexcept -> bool
        {
            return std::holds_alternative<ErrorTypeDesc>(descPayload);
        }
        auto IsBuiltin() const noexcept -> bool
        {
            return !IsError() && !IsStruct();
        }
        auto IsComposite() const noexcept -> bool
        {
            return IsArray() || IsStruct();
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

        auto IsIntegralScalarType() const noexcept -> bool
        {
            return IsSameWith(GlslBuiltinType::Ty_int) || IsSameWith(GlslBuiltinType::Ty_uint);
        }

        // Returns true if this type is the same with the given builtin type
        auto IsSameWith(GlslBuiltinType type) const -> bool;

        // Returns true if this type is the same with the given type
        auto IsSameWith(const Type* other) const -> bool;

        // Returns true if this type is convertible to the given type
        auto IsConvertibleTo(const Type* to) const -> bool;

        // Returns true if conversion from this type to `lhsTo` is better than that to `rhsTo`
        auto HasBetterConversion(const Type* lhsTo, const Type* rhsTo) const -> bool;

    private:
        // FIXME: is this for debug only?
        std::string debugName = "<FIXME>";

        DescPayloadType descPayload;
    };

    inline auto TypeDescToString(const Type* desc) -> std::string
    {
        if (desc && !desc->GetDebugName().Empty()) {
            return std::string{desc->GetDebugName()};
        }
        else {
            return fmt::format("{}", static_cast<const void*>(desc));
        }
    }

    // Get a globally unique type instance for the particular vector type, if any
    // NOTE if the vector type doesn't exist in glsl language, returns nullptr
    auto GetVectorTypeDesc(ScalarType type, size_t dim) -> const Type*;
} // namespace glsld
