#pragma once
#include "SyntaxToken.h"
#include <optional>
#include <variant>
#include <vector>

namespace glsld
{
    class TypeDesc;

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

    enum class BuiltinType
    {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, ...) Ty_##GLSL_TYPE,
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
    };

    inline auto GetBuiltinType(const SyntaxToken& tok) -> std::optional<BuiltinType>
    {
        switch (tok.klass) {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, ...)                                                                              \
    case TokenKlass::K_##GLSL_TYPE:                                                                                    \
        return BuiltinType::Ty_##GLSL_TYPE;
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
        default:
            return std::nullopt;
        }
    }

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
        const TypeDesc* elementType;

        // NOTE dim size of 0 means unsized/error-sized dimension
        std::vector<size_t> dimSizes;
    };

    class AstDecl;
    struct StructTypeDesc
    {
        // FIXME: what should be in a struct type desc?
        AstDecl* decl;
        std::string name;
        std::vector<std::pair<std::string, const TypeDesc*>> members;
    };
    struct FunctionTypeDesc
    {
    };

    class TypeDesc
    {
    public:
        // Do we need function type?
        using DescPayloadType = std::variant<ErrorTypeDesc, VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc,
                                             MatrixTypeDesc, SamplerTypeDesc, ArrayTypeDesc, StructTypeDesc>;

        TypeDesc(std::string name, DescPayloadType payload) : debugName(std::move(name)), descPayload(payload)
        {
        }

        auto IsError() const -> bool
        {
            return std::holds_alternative<ErrorTypeDesc>(descPayload);
        }
        auto IsBuiltin() const -> bool
        {
            return !IsError() && !IsStruct();
        }
        auto IsVoid() const -> bool
        {
            return std::holds_alternative<VoidTypeDesc>(descPayload);
        }
        auto IsScalar() const -> bool
        {
            return std::holds_alternative<ScalarTypeDesc>(descPayload);
        }
        auto IsVector() const -> bool
        {
            return std::holds_alternative<VectorTypeDesc>(descPayload);
        }
        auto IsMatrix() const -> bool
        {
            return std::holds_alternative<MatrixTypeDesc>(descPayload);
        }
        auto IsSampler() const -> bool
        {
            return std::holds_alternative<SamplerTypeDesc>(descPayload);
        }
        auto IsArray() const -> bool
        {
            return std::holds_alternative<ArrayTypeDesc>(descPayload);
        }
        auto IsStruct() const -> bool
        {
            return std::holds_alternative<StructTypeDesc>(descPayload);
        }
        // auto IsFunction() const -> bool
        // {
        //     return std::holds_alternative<FunctionTypeDesc>(descPayload);
        // }

        auto GetDebugName() const -> const std::string&
        {
            return debugName;
        }

        auto GetScalarDesc() const -> const ScalarTypeDesc*
        {
            return std::get_if<ScalarTypeDesc>(&descPayload);
        }
        auto GetVectorDesc() const -> const VectorTypeDesc*
        {
            return std::get_if<VectorTypeDesc>(&descPayload);
        }
        auto GetMatrixDesc() const -> const MatrixTypeDesc*
        {
            return std::get_if<MatrixTypeDesc>(&descPayload);
        }
        auto GetSamplerDesc() const -> const SamplerTypeDesc*
        {
            return std::get_if<SamplerTypeDesc>(&descPayload);
        }
        auto GetArrayDesc() const -> const ArrayTypeDesc*
        {
            return std::get_if<ArrayTypeDesc>(&descPayload);
        }
        auto GetStructDesc() const -> const StructTypeDesc*
        {
            return std::get_if<StructTypeDesc>(&descPayload);
        }

        auto IsIntegralScalarType() const -> bool
        {
            return IsSameWith(BuiltinType::Ty_int) || IsSameWith(BuiltinType::Ty_uint);
        }

        auto IsSameWith(BuiltinType type) const -> bool;

        auto IsSameWith(const TypeDesc* other) const -> bool;

        auto IsConvertibleTo(const TypeDesc* to) const -> bool;

        auto HasBetterConversion(const TypeDesc* lhsTo, const TypeDesc* rhsTo) const -> bool;

    private:
        // FIXME: is this for debug only?
        std::string debugName;

        DescPayloadType descPayload;
    };

    inline auto TypeDescToString(const TypeDesc* desc) -> std::string
    {
        if (desc && !desc->GetDebugName().empty()) {
            return std::string{desc->GetDebugName()};
        }
        else {
            return fmt::format("{}", static_cast<const void*>(desc));
        }
    }

    auto GetErrorTypeDesc() -> const TypeDesc*;
    auto GetBuiltinTypeDesc(BuiltinType type) -> const TypeDesc*;
    auto GetVectorTypeDesc(ScalarType type, size_t dim) -> const TypeDesc*;
} // namespace glsld
