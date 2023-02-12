#pragma once
#include "Semantic.h"
#include "Typing.h"
#include "ConstValue.h"
#include "AstBase.h"

#include <map>

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

    // A view pointer into a declaration, including an index of declarator.
    // For declaration without declarators, that index should always be zero.
    class DeclView
    {
    public:
        DeclView() = default;
        DeclView(AstDecl* decl) : decl(decl)
        {
        }
        DeclView(AstDecl* decl, size_t index) : decl(decl), index(index)
        {
        }

        auto IsValid() const -> bool
        {
            return decl != nullptr;
        }

        auto GetDecl() const -> AstDecl*
        {
            return decl;
        }
        auto GetIndex() const -> size_t
        {
            return index;
        }

        operator bool() const
        {
            return IsValid();
        }
        auto operator==(const DeclView&) const -> bool = default;

    private:
        // Referenced declaration AST.
        AstDecl* decl = nullptr;

        // Declarator index. For declarations that cannot declare multiple symbols, this must be 0.
        size_t index = 0;
    };

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
        auto GetResolvedType() const -> const TypeDesc*
        {
            return resolvedType;
        }
        auto SetResolvedType(const TypeDesc* type) -> void
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
                return fmt::format("TypeDesc: {}", resolvedType->GetDebugName());
            }
            else {
                return fmt::format("{}", static_cast<const void*>(resolvedType));
            }
        }

    private:
        // Resolved descriptor of this type
        const TypeDesc* resolvedType = GetErrorTypeDesc();

        // Resolved declaration of this type
        AstDecl* resolvedStructDecl = nullptr;
    };

    template <>
    class AstPayload<AstStructDecl>
    {
    public:
        auto GetTypeDesc() const -> const TypeDesc*
        {
            return typeDesc;
        }
        auto SetTypeDesc(const TypeDesc* typeDesc) -> void
        {
            this->typeDesc = typeDesc;
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

        std::map<std::string, DeclView> memberLookup;
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
            return fmt::format("TypeDesc={}", TypeDescToString(typeDesc));
        }

    private:
        // Struct type that this decl introduces
        const TypeDesc* typeDesc = nullptr;

        std::map<std::string, DeclView> memberLookup;
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

        auto GetDeducedType() const -> const TypeDesc*
        {
            return deducedType;
        }
        auto SetDeducedType(const TypeDesc* deducedType) -> void
        {
            this->deducedType = deducedType;
        }

        auto GetContextualType() const -> const TypeDesc*
        {
            return contextualType;
        }
        auto SetContextualType(const TypeDesc* contextualType) -> void
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