#pragma once
#include "Semantic.h"
#include "Typing.h"
#include "ConstEvaluate.h"

namespace glsld
{
    class TypeDesc;

// Forward declaration of all Ast types
#define DECL_AST_BEGIN_BASE(TYPE) class TYPE;
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE) class TYPE;
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

    template <typename AstType>
    class AstPayload
    {
    public:
        auto DumpPayloadData() const -> std::string
        {
            return "";
        }
    };

    template <>
    class AstPayload<AstQualType>
    {
    public:
        auto GetTypeDesc() -> const TypeDesc*
        {
            return typeDesc;
        }
        auto SetTypeDesc(const TypeDesc* typeDesc) -> void
        {
            this->typeDesc = typeDesc;
        }

        auto DumpPayloadData() const -> std::string
        {
            if (typeDesc && !typeDesc->GetDebugName().empty()) {
                return fmt::format("TypeDesc: {}", typeDesc->GetDebugName());
            }
            else {
                return fmt::format("{}", static_cast<const void*>(typeDesc));
            }
        }

    private:
        // Type of this specifier
        const TypeDesc* typeDesc = nullptr;

        AstDecl* decl = nullptr;
    };

    template <>
    class AstPayload<AstStructDecl>
    {
    public:
        auto GetTypeDesc() -> const TypeDesc*
        {
            return typeDesc;
        }
        auto SetTypeDesc(const TypeDesc* typeDesc) -> void
        {
            this->typeDesc = typeDesc;
        }

        auto DumpPayloadData() const -> std::string
        {
            if (typeDesc && !typeDesc->GetDebugName().empty()) {
                return fmt::format("TypeDesc={}", typeDesc->GetDebugName());
            }
            else {
                return fmt::format("{}", static_cast<const void*>(typeDesc));
            }
        }

    private:
        // Struct type that this decl introduces
        const TypeDesc* typeDesc = nullptr;
    };

    template <>
    class AstPayload<AstInterfaceBlockDecl>
    {
    public:
        auto GetTypeDesc() -> const TypeDesc*
        {
            return typeDesc;
        }
        auto SetTypeDesc(const TypeDesc* typeDesc) -> void
        {
            this->typeDesc = typeDesc;
        }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("TypeDesc={}", TypeDescToString(typeDesc));
        }

    private:
        // Struct type that this decl introduces
        const TypeDesc* typeDesc = nullptr;
    };

    template <>
    class AstPayload<AstExpr>
    {
    public:
        auto GetConstValue() -> ConstValue
        {
            return constValue;
        }
        auto SetConstValue(ConstValue constValue) -> void
        {
            this->constValue = constValue;
        }

        auto GetDeducedType() -> const TypeDesc*
        {
            return deducedType;
        }
        auto SetDeducedType(const TypeDesc* deducedType) -> void
        {
            this->deducedType = deducedType;
        }

        auto GetContextualType() -> const TypeDesc*
        {
            return contextualType;
        }
        auto SetContextualType(const TypeDesc* contextualType) -> void
        {
            this->contextualType = contextualType;
        }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("ConstValue: {}; DeducedType: {}; ContextualType: {}", constValue.ToString(),
                               TypeDescToString(deducedType), TypeDescToString(contextualType));
        }

    private:
        // Constant folded value
        ConstValue constValue = {};

        // Type of the evaluated expression
        const TypeDesc* deducedType = GetErrorTypeDesc();

        // Type of the context where the expression is used
        const TypeDesc* contextualType = GetErrorTypeDesc();
    };

    template <>
    class AstPayload<AstNameAccessExpr>
    {
    public:
        auto GetAccessType() -> NameAccessType
        {
            return accessType;
        }
        auto SetAccessType(NameAccessType accessType) -> void
        {
            this->accessType = accessType;
        }

        auto GetSwizzleInfo() -> SwizzleDesc
        {
            return swizzleInfo;
        }
        auto SetSwizzleInfo(SwizzleDesc swizzle) -> void
        {
            this->swizzleInfo = swizzle;
        }

        auto GetAccessedDecl() -> AstDecl*
        {
            return accessedDecl;
        }
        auto SetAccessedDecl(AstDecl* decl) -> void
        {
            this->accessedDecl = decl;
        }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("AccessType: {}; Name: ?", NameAccessTypeToString(accessType));
        }

    private:
        NameAccessType accessType = NameAccessType::Unknown;
        SwizzleDesc swizzleInfo;
        AstDecl* accessedDecl = nullptr;
    };
} // namespace glsld