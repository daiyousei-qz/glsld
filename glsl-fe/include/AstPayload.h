#pragma once
#include "Semantic.h"
#include "Typing.h"
#include "ConstValue.h"
#include "AstBase.h"

#include <map>

namespace glsld
{
    class Type;

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
        auto GetResolvedType() const -> const Type*
        {
            return resolvedType;
        }
        auto SetResolvedType(const Type* type) -> void
        {
            this->resolvedType = type;
        }

        auto GetResolvedStructDecl() const -> AstDecl*
        {
            return resolvedStructDecl;
        }
        auto SetResolvedStructDecl(AstDecl* decl) -> void
        {
            this->resolvedStructDecl = decl;
        }

        auto DumpPayloadData() const -> std::string
        {
            if (resolvedType && !resolvedType->GetDebugName().empty()) {
                return fmt::format("Type: {}", resolvedType->GetDebugName());
            }
            else {
                return fmt::format("{}", static_cast<const void*>(resolvedType));
            }
        }

    private:
        // Resolved descriptor of this type
        const Type* resolvedType = GetErrorTypeDesc();

        // Resolved declaration of this type
        AstDecl* resolvedStructDecl = nullptr;
    };

    template <>
    class AstPayload<AstStructDecl>
    {
    public:
        auto GetTypeDesc() const -> const Type*
        {
            return definedType;
        }
        auto SetTypeDesc(const Type* typeDesc) -> void
        {
            this->definedType = typeDesc;
        }

        auto FindMemberDecl(const std::string& name) -> DeclView
        {
            auto it = memberLookup.find(name);
            if (it != memberLookup.end()) {
                return it->second;
            }
            else {
                return DeclView{};
            }
        }
        auto AddMemberDecl(const std::string& name, DeclView decl) -> void
        {
            // FIXME: find duplicate
            memberLookup[name] = decl;
        }

        auto DumpPayloadData() const -> std::string
        {
            if (definedType && !definedType->GetDebugName().empty()) {
                return fmt::format("Type={}", definedType->GetDebugName());
            }
            else {
                return fmt::format("{}", static_cast<const void*>(definedType));
            }
        }

    private:
        // Struct type that this decl introduces
        const Type* definedType = nullptr;

        // Lookup table for member declarations. Mapping from member name to declaration.
        std::unordered_map<std::string, DeclView> memberLookup;
    };

    template <>
    class AstPayload<AstInterfaceBlockDecl>
    {
    public:
        auto GetTypeDesc() -> const Type*
        {
            return definedType;
        }
        auto SetTypeDesc(const Type* typeDesc) -> void
        {
            this->definedType = typeDesc;
        }

        auto FindMemberDecl(const std::string& name) -> DeclView
        {
            auto it = memberLookup.find(name);
            if (it != memberLookup.end()) {
                return it->second;
            }
            else {
                return DeclView{};
            }
        }
        auto AddMemberDecl(const std::string& name, DeclView decl) -> void
        {
            // FIXME: find duplicate
            memberLookup[name] = decl;
        }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("Type={}", TypeDescToString(definedType));
        }

    private:
        // Struct type that this decl introduces
        const Type* definedType = nullptr;

        // Lookup table for member declarations. Mapping from member name to declaration.
        std::unordered_map<std::string, DeclView> memberLookup;
    };

    template <>
    class AstPayload<AstExpr>
    {
    public:
        ~AstPayload()
        {
            if (constValue) {
                delete constValue;
                constValue = nullptr;
            }
        }

        auto GetConstValue() const -> const ConstValue&
        {
            static const ConstValue errorValue{};
            if (constValue) {
                return *constValue;
            }
            else {
                return errorValue;
            }
        }
        auto SetConstValue(const ConstValue& value) -> void
        {
            if (constValue) {
                delete constValue;
            }

            constValue = new ConstValue(value);
        }

        auto GetDeducedType() const -> const Type*
        {
            return deducedType;
        }
        auto SetDeducedType(const Type* deducedType) -> void
        {
            this->deducedType = deducedType;
        }

        auto GetContextualType() const -> const Type*
        {
            return contextualType;
        }
        auto SetContextualType(const Type* contextualType) -> void
        {
            this->contextualType = contextualType;
        }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("ConstValue: {}; DeducedType: {}; ContextualType: {}", GetConstValue().ToString(),
                               TypeDescToString(deducedType), TypeDescToString(contextualType));
        }

    private:
        // Constant folded value
        const ConstValue* constValue = nullptr;

        // Type of the evaluated expression.
        const Type* deducedType = GetErrorTypeDesc();

        // Type of the context where the expression is used.
        // NOTE our AST is not a DAG but strictly a tree since we have to keep track of the source location.
        const Type* contextualType = GetErrorTypeDesc();
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

        auto GetAccessedDecl() -> DeclView
        {
            return accessedDecl;
        }
        auto SetAccessedDecl(DeclView decl) -> void
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
        DeclView accessedDecl = {};
    };
} // namespace glsld