#pragma once
#include "AstDecl.h"
#include "SyntaxToken.h"
#include <optional>
#include <variant>
#include <vector>

namespace glsld
{
    class TypeDesc;

#define DECL_BUILTIN_TYPE(TYPE, ...) Ty_##TYPE,

    enum class BuiltinType
    {
#include "GlslType.inc"
    };

#undef DECL_BUILTIN_TYPE

    inline auto GetBuiltinType(const SyntaxToken& tok) -> std::optional<BuiltinType>
    {
        switch (tok.klass) {
#define DECL_BUILTIN_TYPE(TYPE, ...)                                                                                   \
    case TokenKlass::K_##TYPE:                                                                                         \
        return BuiltinType::Ty_##TYPE;
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
        int size;
    };
    struct VectorTypeDesc
    {
        ScalarTypeDesc scalar;
        int vectorSize;
    };
    struct MatrixTypeDesc
    {
        ScalarTypeDesc scalar;
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
    struct StructTypeDesc
    {
        AstStructDecl* decl;
        std::vector<TypeDesc*> members;
    };
    struct FunctionTypeDesc
    {
        TypeDesc* ret;
        std::vector<TypeDesc*> params;
    };

    class TypeDesc
    {
    public:
        using DescPayloadType = std::variant<ErrorTypeDesc, VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc,
                                             MatrixTypeDesc, SamplerTypeDesc, StructTypeDesc, FunctionTypeDesc>;

        TypeDesc(DescPayloadType payload) : descPayload(payload)
        {
        }

        auto IsError() const -> bool
        {
            return std::holds_alternative<ErrorTypeDesc>(descPayload);
        }
        auto IsBuiltin() const -> bool
        {
            return !IsError() && !IsStruct() && !IsFunction();
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
        auto IsStruct() const -> bool
        {
            return std::holds_alternative<StructTypeDesc>(descPayload);
        }
        auto IsFunction() const -> bool
        {
            return std::holds_alternative<FunctionTypeDesc>(descPayload);
        }

        auto GetScalarDesc() -> const ScalarTypeDesc*;

    private:
        DescPayloadType descPayload;
    };

    inline auto GetErrorTypeDesc() -> const TypeDesc*
    {
        static TypeDesc typeDesc{ErrorTypeDesc{}};
        return &typeDesc;
    }
    inline auto GetBuiltinTypeDesc(BuiltinType type) -> const TypeDesc*
    {

        switch (type) {
#define DECL_BUILTIN_TYPE(TYPE, DESC_PAYLOAD_TYPE, ...)                                                                \
    case BuiltinType::Ty_##TYPE:                                                                                       \
    {                                                                                                                  \
        static TypeDesc typeDesc{DESC_PAYLOAD_TYPE{}};                                                                 \
        return &typeDesc;                                                                                              \
    }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
        default:
            GLSLD_UNREACHABLE();
        }
    }

} // namespace glsld
