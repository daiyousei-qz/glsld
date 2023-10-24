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
        // DeclScope scope = DeclScope::Global;

    protected:
        AstDecl() = default;

        // auto SetScope(DeclScope scope) noexcept -> void
        // {
        //     this->scope = scope;
        // }
        // auto GetScope() const noexcept -> DeclScope
        // {
        //     return scope;
        // }

        template <typename Dumper>
        auto DumpPayload(Dumper d) const -> void
        {
        }
    };

    class AstErrorDecl final : public AstDecl
    {
    public:
        AstErrorDecl() = default;

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
        }
    };

    class AstEmptyDecl final : public AstDecl
    {
    public:
        AstEmptyDecl() = default;

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
        }
    };

    struct VariableDeclarator
    {
        // The identifier token that defines the symbol name
        // Maybe an error token, which means the declarator is invalid.
        SyntaxToken declTok = {};

        // Array specifier
        const AstArraySpec* arraySize = nullptr;

        // Initializer
        const AstInitializer* initializer = nullptr;

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
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

    class AstVariableDecl final : public AstDecl
    {
    private:
        AstQualType* qualType;
        std::vector<VariableDeclarator> declarators;

        // Payload:
        std::vector<const Type*> resolvedTypes;

    public:
        AstVariableDecl(AstQualType* type) : qualType(type)
        {
            // Yes, a variable decl could declare no variable
        }
        AstVariableDecl(AstQualType* type, std::vector<VariableDeclarator> declarators)
            : qualType(type), declarators(std::move(declarators))
        {
        }

        auto GetQualType() const noexcept -> const AstQualType*
        {
            return qualType;
        }
        auto GetDeclarators() const noexcept -> ArrayView<VariableDeclarator>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(qualType)) {
                return false;
            }

            for (const auto& declarator : declarators) {
                if (declarator.arraySize && !visitor.Traverse(declarator.arraySize)) {
                    return false;
                }
                if (declarator.initializer && !visitor.Traverse(declarator.initializer)) {
                    return false;
                }
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
            d.DumpChildNode("QualType", *qualType);
            for (const auto& declarator : declarators) {
                d.DumpChildItem("Declarator", [&](Dumper& d) { declarator.Dump(d); });
            }
        }
    };

    class AstStructDecl final : public AstDecl
    {
    private:
        std::optional<SyntaxToken> declTok;
        std::vector<AstVariableDecl*> members;

        // Payload:
        const Type* declaredType = nullptr;

    public:
        AstStructDecl(std::optional<SyntaxToken> declTok, std::vector<AstVariableDecl*> members)
            : declTok(declTok), members(std::move(members))
        {
        }

        auto GetDeclTok() const noexcept -> const std::optional<SyntaxToken>&
        {
            return declTok;
        }
        auto GetMembers() const noexcept -> ArrayView<const AstVariableDecl*>
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (auto member : members) {
                if (!visitor.Traverse(*member)) {
                    return false;
                }
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
            if (declTok) {
                d.DumpAttribute("DeclToken", declTok->IsIdentifier() ? declTok->text.StrView() : "<Error>");
            }
            d.DumpAttribute("ResolvedType", "FIXME");
            for (const auto& member : members) {
                d.DumpChildItem("Member", [&](Dumper& d) { member->Dump(d); });
            }
        }
    };

    class AstParamDecl final : public AstDecl
    {
    private:
        AstQualType* qualType;
        VariableDeclarator declarator;

        const Type* resolvedType = nullptr;

    public:
        AstParamDecl(AstQualType* type, VariableDeclarator declarator) : qualType(type), declarator(declarator)
        {
        }

        auto GetQualType() const noexcept -> const AstQualType*
        {
            return qualType;
        }
        auto GetDeclarator() const noexcept -> const VariableDeclarator&
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(qualType)) {
                return false;
            }
            if (declarator.arraySize && !visitor.Traverse(declarator.arraySize)) {
                return false;
            }
            if (declarator.initializer && !visitor.Traverse(declarator.initializer)) {
                return false;
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
            d.DumpAttribute("ResolvedType", "FIXME");
            d.DumpChildNode("QualType", *qualType);
            declarator.Dump(d);
        }
    };

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
        auto Traverse(Visitor& visitor) const -> bool
        {
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

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
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
        std::vector</*NotNull*/ AstVariableDecl*> members;
        std::optional<VariableDeclarator> declarator;

        // Payload:
        const Type* resolvedBlockType = nullptr;

        // Payload:
        const Type* resolvedInstanceType = nullptr;

    public:
        AstInterfaceBlockDecl(AstTypeQualifierSeq* quals, SyntaxToken declTok, std::vector<AstVariableDecl*> members,
                              std::optional<VariableDeclarator> declarator)
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
        auto GetMembers() const noexcept -> ArrayView<const AstVariableDecl*>
        {
            return members;
        }
        auto GetDeclarator() const noexcept -> std::optional<VariableDeclarator>
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (quals && !visitor.Traverse(quals)) {
                return false;
            }
            for (auto member : members) {
                if (!visitor.Traverse(*member)) {
                    return false;
                }
            }
            if (declarator) {
                if (declarator->arraySize && visitor.Traverse(declarator->arraySize)) {
                    return false;
                }
                if (declarator->initializer && visitor.Traverse(declarator->initializer)) {
                    return false;
                }
            }

            return true;
        }

        template <typename Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstDecl::DumpPayload(d);
            d.DumpAttribute("DeclToken", declTok.IsIdentifier() ? declTok.text.StrView() : "<Error>");

            d.DumpChildNode("Quals", *quals);
            for (auto member : members) {
                d.DumpChildNode("Member", *member);
            }
            if (declarator) {
                d.DumpChildItem("InstanceDeclarator", [&](Dumper& d) { declarator->Dump(d); });
            }
        }
    };

} // namespace glsld