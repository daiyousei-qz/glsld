#pragma once
#include "Ast/Base.h"
#include "Ast/Misc.h"
#include "Ast/Type.h"
#include "Compiler/SyntaxToken.h"

#include <optional>

namespace glsld
{
    enum class DeclScope
    {
        Global,
        Function,
        Struct,
        Block,
    };

    // Base class for all AST nodes that could be used as a declaration.
    class AstDecl : public AstNode
    {
    private:
        DeclScope scope = DeclScope::Global;

    protected:
        AstDecl() = default;

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
        }

    public:
        auto SetScope(DeclScope scope) noexcept -> void
        {
            this->scope = scope;
        }
        auto GetScope() const noexcept -> DeclScope
        {
            return scope;
        }
    };

    class AstErrorDecl final : public AstDecl
    {
    public:
        AstErrorDecl() = default;

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return AstDecl::DoTraverse(visitor);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
        }
    };

    // Represents an empty declaration, such as a semicolon in the global scope.
    class AstEmptyDecl final : public AstDecl
    {
    public:
        AstEmptyDecl() = default;

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return AstDecl::DoTraverse(visitor);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
        }
    };

    // Represents a single declarator, including name, array specifier and initializer.
    struct Declarator
    {
        // The identifier token that defines the symbol name
        // Maybe an error token, which means the declarator is invalid.
        SyntaxToken declTok = {};

        // Array specifier. May be nullptr if none is specified.
        const AstArraySpec* arraySize = nullptr;

        // Initializer. May be nullptr if none is specified.
        const AstInitializer* initializer = nullptr;

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            d.DumpAttribute("DeclToken", declTok.IsIdentifier() ? declTok.text.StrView() : "<Error>");
            if (arraySize) {
                d.DumpChildNode("ArraySize", *arraySize);
            }
            if (initializer) {
                d.DumpChildNode("Initializer", *initializer);
            }
        }
    };

    // Base class for all declarations that consist of a qualified type and a list of declarators.
    class AstDeclaratorDecl : public AstDecl
    {
    private:
        // [Node]
        NotNull<AstQualType*> qualType;

        // [Node]
        ArrayView<Declarator> declarators;

        // [Payload]
        ArrayView<const Type*> resolvedTypes;

    protected:
        AstDeclaratorDecl(AstQualType* type, ArrayView<Declarator> declarators)
            : qualType(type), declarators(declarators)
        {
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*qualType)) {
                return false;
            }

            for (const auto& declarator : declarators) {
                if (declarator.arraySize && !visitor.Traverse(*declarator.arraySize)) {
                    return false;
                }
                if (declarator.initializer && !visitor.Traverse(*declarator.initializer)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
            d.DumpChildNode("QualType", *qualType);
            for (const auto& declarator : declarators) {
                d.DumpChildItem("Declarator", [&](Dumper& d) { declarator.DoDump(d); });
            }
        }

    public:
        auto GetQualType() const noexcept -> const AstQualType*
        {
            return qualType;
        }
        auto GetDeclarators() const noexcept -> ArrayView<Declarator>
        {
            return declarators;
        }

        auto SetResolvedTypes(ArrayView<const Type*> types) -> void
        {
            this->resolvedTypes = std::move(types);
        }
        auto GetResolvedTypes() const noexcept -> ArrayView<const Type*>
        {
            return resolvedTypes;
        }
    };

    // Represents a declaration of variables.
    class AstVariableDecl final : public AstDeclaratorDecl
    {
    public:
        AstVariableDecl(AstQualType* type, ArrayView<Declarator> declarators) : AstDeclaratorDecl(type, declarators)
        {
        }

        auto IsConstVariable() const -> bool
        {
            return GetQualType()->GetQualifiers() && GetQualType()->GetQualifiers()->GetQualGroup().qConst;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return AstDeclaratorDecl::DoTraverse(visitor);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDeclaratorDecl::DoDump(d);
        }
    };

    // Represents a declaration of a struct member.
    class AstFieldDecl final : public AstDeclaratorDecl
    {
    private:
        // [Payload]
        AstDecl* parentDecl = nullptr;

    public:
        AstFieldDecl(AstQualType* type, ArrayView<Declarator> declarators)
            : AstDeclaratorDecl(type, std::move(declarators))
        {
        }

        auto SetParentDecl(AstDecl* parentDecl) -> void
        {
            GLSLD_ASSERT(parentDecl && (parentDecl->Is<AstStructDecl>() || parentDecl->Is<AstInterfaceBlockDecl>()));
            this->parentDecl = parentDecl;
        }
        auto GetParentDecl() const -> const AstDecl*
        {
            return parentDecl;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return AstDeclaratorDecl::DoTraverse(visitor);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDeclaratorDecl::DoDump(d);
        }
    };

    // Represents a declaration of a struct, .e.g `struct S { ... };`
    class AstStructDecl final : public AstDecl
    {
    private:
        // [Node]
        std::optional<SyntaxToken> declTok;

        // [Node]
        ArrayView</*NotNull*/ AstFieldDecl*> members;

        // [Payload]
        const Type* declaredType = nullptr;

    public:
        AstStructDecl(std::optional<SyntaxToken> declTok, ArrayView<AstFieldDecl*> members)
            : declTok(declTok), members(std::move(members))
        {
        }

        auto GetDeclTok() const noexcept -> const std::optional<SyntaxToken>&
        {
            return declTok;
        }
        auto GetMembers() const noexcept -> ArrayView<const AstFieldDecl*>
        {
            return members;
        }

        auto SetDeclaredType(const Type* type) -> void
        {
            this->declaredType = type;
        }
        auto GetDeclaredType() const -> const Type*
        {
            return declaredType;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstDecl::DoTraverse(visitor)) {
                return false;
            }
            for (auto member : members) {
                if (!visitor.Traverse(*member)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
            if (declTok) {
                d.DumpAttribute("DeclToken", declTok->IsIdentifier() ? declTok->text.StrView() : "<Error>");
            }
            for (const auto& member : members) {
                d.DumpChildItem("Member", [&](Dumper& d) { member->DoDump(d); });
            }
            d.DumpAttribute("DeclaredType", declaredType->GetDebugName());
        }
    };

    // Represents a declaration of a function parameter, e.g. `int x` in `void foo(int x) { ... }`.
    class AstParamDecl final : public AstDecl
    {
    private:
        NotNull<AstQualType*> qualType;
        std::optional<Declarator> declarator;

        const Type* resolvedType = nullptr;

    public:
        AstParamDecl(AstQualType* type, std::optional<Declarator> declarator) : qualType(type), declarator(declarator)
        {
        }

        auto GetQualType() const noexcept -> const AstQualType*
        {
            return qualType;
        }
        auto GetDeclarator() const noexcept -> const std::optional<Declarator>&
        {
            return declarator;
        }

        auto SetResolvedType(const Type* type) -> void
        {
            this->resolvedType = type;
        }
        auto GetResolvedType() const noexcept -> const Type*
        {
            return resolvedType;
        }

        // True if this parameter is a const parameter.
        auto IsConstParam() const noexcept -> bool
        {
            return qualType->GetQualifiers() && qualType->GetQualifiers()->GetQualGroup().qConst;
        }

        // True if this parameter is an input parameter.
        auto IsInputParam() const noexcept -> bool
        {
            // Note by default, all parameters without any qualifier are input.
            return !qualType->GetQualifiers() || !qualType->GetQualifiers()->GetQualGroup().qOut;
        }

        // True if this parameter is an output parameter.
        auto IsOutputParam() const noexcept -> bool
        {
            return qualType->GetQualifiers() &&
                   (qualType->GetQualifiers()->GetQualGroup().qOut || qualType->GetQualifiers()->GetQualGroup().qInout);
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstDecl::DoTraverse(visitor)) {
                return false;
            }
            if (!visitor.Traverse(*qualType)) {
                return false;
            }
            if (declarator) {
                if (declarator->arraySize && !visitor.Traverse(*declarator->arraySize)) {
                    return false;
                }
                if (declarator->initializer && !visitor.Traverse(*declarator->initializer)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
            d.DumpChildNode("QualType", *qualType);
            if (declarator) {
                declarator->DoDump(d);
            }
            d.DumpAttribute("ResolvedType", resolvedType->GetDebugName());
        }
    };

    // Represents a declaration of a function, e.g. `void foo(int x) { ... }`.
    class AstFunctionDecl final : public AstDecl
    {
    private:
        // [Node]
        NotNull<AstQualType*> returnType;

        // [Node]
        SyntaxToken declTok;

        // [Node]
        ArrayView</*NotNull*/ AstParamDecl*> params;

        // [Node]
        AstStmt* body;

        // [Payload]
        // The first declaration of this function. Used to determine whether this is a redeclaration.
        AstFunctionDecl* firstDeclaration = nullptr;

    public:
        AstFunctionDecl(AstQualType* returnType, SyntaxToken declTok, ArrayView<AstParamDecl*> params, AstStmt* body)
            : returnType(returnType), declTok(declTok), params(params), body(body)
        {
        }

        auto GetReturnType() const noexcept -> const AstQualType*
        {
            return returnType;
        }
        auto GetDeclTok() const noexcept -> SyntaxToken
        {
            return declTok;
        }
        auto GetParams() const noexcept -> ArrayView<const AstParamDecl*>
        {
            return params;
        }
        auto GetBody() const noexcept -> const AstStmt*
        {
            return body;
        }

        auto SetFirstDeclaration(AstFunctionDecl* decl) -> void
        {
            this->firstDeclaration = decl;
        }
        auto GetFirstDeclaration() const noexcept -> const AstFunctionDecl*
        {
            return firstDeclaration;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstDecl::DoTraverse(visitor)) {
                return false;
            }
            if (!visitor.Traverse(*returnType)) {
                return false;
            }
            for (auto param : params) {
                if (!visitor.Traverse(*param)) {
                    return false;
                }
            }
            if (body && !visitor.Traverse(*body)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
            d.DumpAttribute("DeclToken", declTok.IsIdentifier() ? declTok.text.StrView() : "<Error>");

            d.DumpChildNode("ReturnType", *returnType);
            for (auto param : params) {
                d.DumpChildNode("Param", *param);
            }
            if (body) {
                d.DumpChildNode("Body", *body);
            }
        }
    };

    class AstInterfaceBlockDecl final : public AstDecl
    {
    private:
        NotNull<AstTypeQualifierSeq*> quals;
        SyntaxToken declTok;
        ArrayView</*NotNull*/ AstFieldDecl*> members;
        std::optional<Declarator> declarator;

        // Payload:
        const Type* resolvedBlockType = nullptr;

        // Payload:
        const Type* resolvedInstanceType = nullptr;

    public:
        AstInterfaceBlockDecl(AstTypeQualifierSeq* quals, SyntaxToken declTok, ArrayView<AstFieldDecl*> members,
                              std::optional<Declarator> declarator)
            : quals(quals), declTok(declTok), members(std::move(members)), declarator(declarator)
        {
        }

        auto GetQuals() const noexcept -> const AstTypeQualifierSeq*
        {
            return quals;
        }
        auto GetDeclTok() const noexcept -> SyntaxToken
        {
            return declTok;
        }
        auto GetMembers() const noexcept -> ArrayView<const AstFieldDecl*>
        {
            return members;
        }
        auto GetDeclarator() const noexcept -> std::optional<Declarator>
        {
            return declarator;
        }

        auto SetResolvedBlockType(const Type* type) -> void
        {
            this->resolvedBlockType = type;
        }
        auto GetResolvedBlockType() const noexcept -> const Type*
        {
            return resolvedBlockType;
        }
        auto SetResolvedInstanceType(const Type* type) -> void
        {
            this->resolvedInstanceType = type;
        }
        auto GetResolvedInstanceType() const noexcept -> const Type*
        {
            return resolvedInstanceType;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstDecl::DoTraverse(visitor)) {
                return false;
            }
            if (quals && !visitor.Traverse(*quals)) {
                return false;
            }
            for (auto member : members) {
                if (!visitor.Traverse(*member)) {
                    return false;
                }
            }
            if (declarator) {
                if (declarator->arraySize && !visitor.Traverse(*declarator->arraySize)) {
                    return false;
                }
                if (declarator->initializer && !visitor.Traverse(*declarator->initializer)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
            d.DumpAttribute("ResolvedBlockType", resolvedBlockType->GetDebugName());
            if (declarator) {
                d.DumpAttribute("ResolvedInstanceType", resolvedInstanceType->GetDebugName());
            }

            d.DumpAttribute("DeclToken", declTok.IsIdentifier() ? declTok.text.StrView() : "<Error>");

            d.DumpChildNode("Quals", *quals);
            for (auto member : members) {
                d.DumpChildNode("Member", *member);
            }
            if (declarator) {
                d.DumpChildItem("InstanceDeclarator", [&](Dumper& d) { declarator->DoDump(d); });
            }
        }
    };

} // namespace glsld