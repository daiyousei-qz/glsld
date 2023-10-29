#pragma once
#include "Ast/Base.h"
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
        std::vector<Declarator> declarators;

        // [Payload]
        std::vector<const Type*> resolvedTypes;

    protected:
        AstDeclaratorDecl(AstQualType* type, std::vector<Declarator> declarators)
            : qualType(type), declarators(std::move(declarators))
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

        auto SetResolvedTypes(std::vector<const Type*> types) -> void
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
        AstVariableDecl(AstQualType* type, std::vector<Declarator> declarators)
            : AstDeclaratorDecl(type, std::move(declarators))
        {
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
    public:
        AstFieldDecl(AstQualType* type, std::vector<Declarator> declarators)
            : AstDeclaratorDecl(type, std::move(declarators))
        {
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
        std::vector<AstFieldDecl*> members;

        // [Payload]
        const Type* declaredType = nullptr;

    public:
        AstStructDecl(std::optional<SyntaxToken> declTok, std::vector<AstFieldDecl*> members)
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
        AstQualType* qualType;
        Declarator declarator;

        const Type* resolvedType = nullptr;

    public:
        AstParamDecl(AstQualType* type, Declarator declarator) : qualType(type), declarator(declarator)
        {
        }

        auto GetQualType() const noexcept -> const AstQualType*
        {
            return qualType;
        }
        auto GetDeclarator() const noexcept -> const Declarator&
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

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstDecl::DoTraverse(visitor)) {
                return false;
            }
            if (!visitor.Traverse(*qualType)) {
                return false;
            }
            if (declarator.arraySize && !visitor.Traverse(*declarator.arraySize)) {
                return false;
            }
            if (declarator.initializer && !visitor.Traverse(*declarator.initializer)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstDecl::DoDump(d);
            d.DumpChildNode("QualType", *qualType);
            declarator.DoDump(d);
            d.DumpAttribute("ResolvedType", resolvedType->GetDebugName());
        }
    };

    // Represents a declaration of a function, e.g. `void foo(int x) { ... }`.
    class AstFunctionDecl final : public AstDecl
    {
    private:
        NotNull<AstQualType*> returnType;
        SyntaxToken declTok;
        std::vector</*NotNull*/ AstParamDecl*> params;
        AstStmt* body;

    public:
        AstFunctionDecl(AstQualType* returnType, SyntaxToken declTok, std::vector<AstParamDecl*> params, AstStmt* body)
            : returnType(returnType), declTok(declTok), params(std::move(params)), body(body)
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
        std::vector</*NotNull*/ AstFieldDecl*> members;
        std::optional<Declarator> declarator;

        // Payload:
        const Type* resolvedBlockType = nullptr;

        // Payload:
        const Type* resolvedInstanceType = nullptr;

    public:
        AstInterfaceBlockDecl(AstTypeQualifierSeq* quals, SyntaxToken declTok, std::vector<AstFieldDecl*> members,
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
                if (declarator->arraySize && visitor.Traverse(*declarator->arraySize)) {
                    return false;
                }
                if (declarator->initializer && visitor.Traverse(*declarator->initializer)) {
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