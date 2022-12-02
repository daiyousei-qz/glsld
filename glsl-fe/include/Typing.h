#pragma once
#include "AstDecl.h"
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
#define DECL_BUILTIN_TYPE(TYPE, ...) Ty_##TYPE,
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
    };

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
        ScalarType type;
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

    struct ArrayTypeDesc
    {
        TypeDesc* elementType;
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
        // Do we need function type?
        using DescPayloadType = std::variant<ErrorTypeDesc, VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc,
                                             MatrixTypeDesc, SamplerTypeDesc, StructTypeDesc>;

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
        auto IsStruct() const -> bool
        {
            return std::holds_alternative<StructTypeDesc>(descPayload);
        }
        // auto IsFunction() const -> bool
        // {
        //     return std::holds_alternative<FunctionTypeDesc>(descPayload);
        // }

        auto GetDebugName() const -> std::string_view
        {
            return debugName;
        }

        auto GetScalarDesc() const -> const ScalarTypeDesc*
        {
            return std::get_if<ScalarTypeDesc>(&descPayload);
        }

        auto IsConvertibleTo(const TypeDesc* to) const -> bool;

        auto HasBetterConversion(const TypeDesc* lhsTo, const TypeDesc* rhsTo) const -> bool;

    private:
        // FIXME: is this for debug only?
        std::string debugName;

        DescPayloadType descPayload;
    };

    inline auto GetErrorTypeDesc() -> const TypeDesc*
    {
        static TypeDesc typeDesc{"<error>", ErrorTypeDesc{}};
        return &typeDesc;
    }
    inline auto GetBuiltinTypeDesc(BuiltinType type) -> const TypeDesc*
    {

        switch (type) {
#define DECL_BUILTIN_TYPE(TYPE, DESC_PAYLOAD_TYPE, ...)                                                                \
    case BuiltinType::Ty_##TYPE:                                                                                       \
    {                                                                                                                  \
        static TypeDesc typeDesc{#TYPE, DESC_PAYLOAD_TYPE{__VA_ARGS__}};                                               \
        return &typeDesc;                                                                                              \
    }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
        default:
            GLSLD_UNREACHABLE();
        }
    }
} // namespace glsld
