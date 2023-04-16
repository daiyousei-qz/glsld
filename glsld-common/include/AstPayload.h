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
    class AstPayload<AstDecl>
    {
    public:
        auto DumpPayloadData() const -> std::string
        {
            return "";
        }

    private:
        // Previous declaration of the same symbol?
        // AstDecl* previousDecl = nullptr;
    };

    template <>
    class AstPayload<AstFunctionDecl>
    {
    public:
        auto GetResolvedParamTypes() const -> ArrayView<const Type*>
        {
            return resolvedParamTypes;
        }
        auto SetResolvedParamTypes(std::vector<const Type*> types) -> void
        {
            this->resolvedParamTypes = std::move(types);
        }

        auto GetResolvedReturnType() const -> const Type*
        {
            return resolvedReturnType;
        }
        auto SetResolvedReturnType(const Type* type) -> void
        {
            this->resolvedReturnType = type;
        }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("ReturnType={}", resolvedReturnType->GetDebugName());
        }

    private:
        std::vector<const Type*> resolvedParamTypes = {};
        const Type* resolvedReturnType              = GetErrorTypeDesc();
    };

    template <>
    class AstPayload<AstStructDecl>
    {
    public:
        auto GetTypeDesc() const -> const Type*
        {
            return structType;
        }
        auto SetTypeDesc(const Type* typeDesc) -> void
        {
            this->structType = typeDesc;
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
            if (structType && !structType->GetDebugName().empty()) {
                return fmt::format("Type={}", structType->GetDebugName());
            }
            else {
                return fmt::format("{}", static_cast<const void*>(structType));
            }
        }

    private:
        // Struct type that this decl introduces
        const Type* structType = nullptr;

        // Lookup table for member declarations. Mapping from member name to declaration.
        std::unordered_map<std::string, DeclView> memberLookup;
    };

    template <>
    class AstPayload<AstInterfaceBlockDecl>
    {
    public:
        auto GetBlockType() const -> const Type*
        {
            return blockType;
        }
        auto SetBlockType(const Type* typeDesc) -> void
        {
            this->blockType = typeDesc;
        }

        auto GetInstanceType() const -> const Type*
        {
            return instanceType;
        }
        auto SetInstanceType(const Type* typeDesc) -> void
        {
            instanceType = typeDesc;
        }

        auto FindMemberDecl(const std::string& name) const -> DeclView
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
            return fmt::format("BlockType={}; InstanceType={}", TypeDescToString(blockType),
                               TypeDescToString(instanceType));
        }

    private:
        // Struct type that this decl introduces
        const Type* blockType = GetErrorTypeDesc();

        // An interface block may have an instance name defined with an array specifier.
        // In this case, the instance type may add array types.
        const Type* instanceType = GetErrorTypeDesc();

        // Lookup table for member declarations. Mapping from member name to declaration.
        std::unordered_map<std::string, DeclView> memberLookup;
    };

    template <>
    class AstPayload<AstExpr>
    {
    public:
        auto GetConstValue() const -> const ConstValue&
        {
            return constValue;
        }
        auto SetConstValue(ConstValue value) -> void
        {
            constValue = std::move(value);
        }

        auto GetDeducedType() const -> const Type*
        {
            return deducedType;
        }
        auto SetDeducedType(const Type* deducedType) -> void
        {
            this->deducedType = deducedType;
        }

        // auto GetContextualType() const -> const Type*
        // {
        //     return contextualType;
        // }
        // auto SetContextualType(const Type* contextualType) -> void
        // {
        //     this->contextualType = contextualType;
        // }

        auto DumpPayloadData() const -> std::string
        {
            return fmt::format("ConstValue: {}; DeducedType: {}", GetConstValue().ToString(),
                               TypeDescToString(deducedType));
            // return fmt::format("ConstValue: {}; DeducedType: {}; ContextualType: {}", GetConstValue().ToString(),
            //                    TypeDescToString(deducedType), TypeDescToString(contextualType));
        }

    private:
        // Constant folded value
        ConstValue constValue = {};

        // Type of the evaluated expression.
        const Type* deducedType = GetErrorTypeDesc();

        // Type of the context where the expression is used.
        // NOTE our AST is not a DAG but strictly a tree since we have to keep track of the source location.
        // const Type* contextualType = GetErrorTypeDesc();
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
            return fmt::format("AccessType: {};", NameAccessTypeToString(accessType));
        }

    private:
        NameAccessType accessType = NameAccessType::Unknown;
        SwizzleDesc swizzleInfo;
        DeclView accessedDecl = {};
    };
} // namespace glsld