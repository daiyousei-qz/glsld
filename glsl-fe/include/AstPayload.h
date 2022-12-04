#pragma once
#include "Semantic.h"
#include "Typing.h"

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
        auto DumpNodeData() const -> std::string
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

        auto DumpNodeData() const -> std::string
        {
            if (typeDesc && !typeDesc->GetDebugName().empty()) {
                return fmt::format("TypeDesc={}", typeDesc->GetDebugName());
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

        auto DumpNodeData() const -> std::string
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

        auto DumpNodeData() const -> std::string
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
    class AstPayload<AstExpr>
    {
    public:
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

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("DeducedType=?; ContextualType=?");
        }

    private:
        // Type of the evaluated expression
        const TypeDesc* deducedType = nullptr;

        // Type of the context where the expression is used
        const TypeDesc* contextualType = nullptr;
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

        auto GetAccessedDecl() -> AstDecl*
        {
            return decl;
        }
        auto SetAccessedDecl(AstDecl* decl) -> void
        {
            this->decl = decl;
        }

        auto DumpPayloadData() const -> std::string
        {
            // return fmt::format("{} ;Name=?");
            return "";
        }

    private:
        NameAccessType accessType = NameAccessType::Unknown;
        AstDecl* decl             = nullptr;
    };
} // namespace glsld