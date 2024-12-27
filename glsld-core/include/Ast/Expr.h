#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Ast/Base.h"
#include "Ast/Decl.h"
#include "Language/ConstValue.h"

namespace glsld
{
    // Base class of all AST nodes that could be used as an initializer, one of:
    // - initializer list
    // - expression
    class AstInitializer : public AstNode
    {
    private:
        // [Payload]
        // Whether this initializer is const, i.e. can be evaluated at compile time.
        bool isConst = false;

    protected:
        AstInitializer() = default;

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            d.DumpAttribute("IsConst", IsConst());
        }

    public:
        auto SetConst(bool isConst) noexcept -> void
        {
            this->isConst = isConst;
        }
        auto IsConst() const noexcept -> bool
        {
            return isConst;
        }
    };

    // Represents an initializer list like `{1, 2, 3}`.
    class AstInitializerList final : public AstInitializer
    {
    private:
        // [Node]
        ArrayView</*NotNull*/ AstInitializer*> items;

    public:
        AstInitializerList(ArrayView<AstInitializer*> items) : items(items)
        {
        }

        auto GetItems() const noexcept -> ArrayView<const AstInitializer*>
        {
            return items;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstInitializer::DoTraverse(visitor)) {
                return false;
            }

            for (auto item : items) {
                if (!visitor.Traverse(*item)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstInitializer::DoDump(d);
            for (auto item : items) {
                d.DumpChildNode("Item", *item);
            }
        }
    };

    // Base class of all AST nodes that could be used as an expression.
    class AstExpr : public AstInitializer
    {
    private:
        // [Payload]
        // The type that this expression is deduced to.
        const Type* deducedType = nullptr;

    protected:
        AstExpr() = default;

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            return AstInitializer::DoTraverse(visitor);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstInitializer::DoDump(d);
            d.DumpAttribute("DeducedType", GetDeducedType()->GetDebugName());
        }

    public:
        auto SetDeducedType(const Type* deducedType) noexcept -> void
        {
            this->deducedType = deducedType;
        }
        auto GetDeducedType() const noexcept -> const Type*
        {
            return deducedType;
        }
    };

    class AstErrorExpr final : public AstExpr
    {
    public:
        AstErrorExpr() = default;

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
        }
    };

    // Represents a constant literal like `1`, `1.0`, `true`.
    class AstLiteralExpr final : public AstExpr
    {
    private:
        // [Node]
        ConstValue value;

    public:
        AstLiteralExpr(ConstValue value) : value(std::move(value))
        {
        }

        auto GetValue() const noexcept -> const ConstValue&
        {
            return value;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpAttribute("Value", value.ToString());
        }
    };

    // Represents a name access expression like `foo`.
    class AstNameAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        AstSyntaxToken nameToken;

        // [Payload]
        // The declaration that this name access expression refers to.
        DeclView resolvedDecl = {};

    public:
        AstNameAccessExpr(AstSyntaxToken name) : nameToken(name)
        {
        }

        auto GetNameToken() const noexcept -> AstSyntaxToken
        {
            return nameToken;
        }

        auto SetResolvedDecl(DeclView decl) noexcept -> void
        {
            resolvedDecl = decl;
        }
        auto GetResolvedDecl() const noexcept -> DeclView
        {
            return resolvedDecl;
        }

        auto IsVariable() const noexcept -> bool
        {
            return resolvedDecl.IsValid() && resolvedDecl.GetDecl()->Is<AstVariableDecl>();
        }
        auto IsParameter() const noexcept -> bool
        {
            return resolvedDecl.IsValid() && resolvedDecl.GetDecl()->Is<AstParamDecl>();
        }
        auto IsInterfaceBlockInstance() const noexcept -> bool
        {
            return resolvedDecl.IsValid() && resolvedDecl.GetDecl()->Is<AstInterfaceBlockDecl>();
        }
        auto IsInterfaceBlockMember() const noexcept -> bool
        {
            return resolvedDecl.IsValid() && resolvedDecl.GetDecl()->Is<AstFieldDecl>();
        }

        // True if this name is declared with a `const` qualifier in the declaration.
        auto IsConstNameAccess() const noexcept -> bool
        {
            if (resolvedDecl.IsValid()) {
                if (auto paramDecl = resolvedDecl.GetDecl()->As<AstParamDecl>()) {
                    if (paramDecl->IsConstParam()) {
                        return true;
                    }
                }
                else if (auto varDecl = resolvedDecl.GetDecl()->As<AstVariableDecl>()) {
                    if (varDecl->IsConstVariable()) {
                        return true;
                    }
                }
            }

            return false;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpAttribute("Name", nameToken.IsIdentifier() ? nameToken.text.StrView() : "<Error>");
            d.DumpChildItem("ResolvedDecl", [this](Dumper& d) { resolvedDecl.DoDump(d); });
        }
    };

    // Represents a field access expression like `foo.bar`.
    class AstFieldAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        // The base expression evaluating to the object of the field access.
        NotNull<AstExpr*> baseExpr;

        // [Node]
        // The identifier token of the field name or an invalid token.
        AstSyntaxToken nameToken;

        // [Payload]
        // The field declaration that this name access expression refers to.
        DeclView resolvedDecl = {};

    public:
        AstFieldAccessExpr(AstExpr* baseExpr, AstSyntaxToken name) : baseExpr(baseExpr), nameToken(name)
        {
        }

        auto GetBaseExpr() const noexcept -> const AstExpr*
        {
            return baseExpr;
        }
        auto GetNameToken() const noexcept -> AstSyntaxToken
        {
            return nameToken;
        }

        auto SetResolvedDecl(DeclView decl) noexcept -> void
        {
            this->resolvedDecl = decl;
        }
        auto GetResolvedDecl() const noexcept -> DeclView
        {
            return resolvedDecl;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return visitor.Traverse(*baseExpr);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpChildNode("BaseExpr", *baseExpr);
            d.DumpAttribute("Name", nameToken.IsIdentifier() ? nameToken.text.StrView() : "<Error>");
            d.DumpChildItem("ResolvedDecl", [this](Dumper& d) { resolvedDecl.DoDump(d); });
        }
    };

    // Represents a swizzle access expression like `foo.xyz`.
    class AstSwizzleAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        // The base expression evaluating to the object of the swizzle access.
        NotNull<AstExpr*> baseExpr;

        // [Node]
        // The identifier token of the swizzle name or an invalid token.
        AstSyntaxToken nameToken;

        // [Payload]
        // The swizzle description parsed from the access name.
        SwizzleDesc swizzleDesc = {};

    public:
        AstSwizzleAccessExpr(AstExpr* baseExpr, AstSyntaxToken name) : baseExpr(baseExpr), nameToken(name)
        {
        }

        auto GetBaseExpr() const noexcept -> const AstExpr*
        {
            return baseExpr;
        }
        auto GetNameToken() const noexcept -> AstSyntaxToken
        {
            return nameToken;
        }

        auto SetSwizzleDesc(SwizzleDesc swizzleDesc) noexcept -> void
        {
            this->swizzleDesc = swizzleDesc;
        }
        auto GetSwizzleDesc() const noexcept -> SwizzleDesc
        {
            return swizzleDesc;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return visitor.Traverse(*baseExpr);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpChildNode("BaseExpr", *baseExpr);
            d.DumpAttribute("Name", nameToken.IsIdentifier() ? nameToken.text.StrView() : "<Error>");
            d.DumpAttribute("Swizzle", swizzleDesc.ToString());
        }
    };

    // Represents an array access expression like `foo[1]`.
    class AstIndexAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        // The base expression of the array access.
        NotNull<AstExpr*> baseExpr;

        // [Node]
        // The index expression of the array access.
        NotNull<AstExpr*> indexExpr;

    public:
        AstIndexAccessExpr(AstExpr* baseExpr, AstExpr* indexExpr) : baseExpr(baseExpr), indexExpr(indexExpr)
        {
        }

        auto GetBaseExpr() const noexcept -> const AstExpr*
        {
            return baseExpr;
        }
        auto GetIndexExpr() const noexcept -> const AstExpr*
        {
            return indexExpr;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            if (!visitor.Traverse(*baseExpr)) {
                return false;
            }
            if (!visitor.Traverse(*indexExpr)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpChildNode("BaseExpr", *baseExpr);
            d.DumpChildNode("IndexExpr", *indexExpr);
        }
    };

    // Represents a unary expression like `!foo`.
    class AstUnaryExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> operand;

        // [Node]
        UnaryOp opcode;

    public:
        AstUnaryExpr(AstExpr* operand, UnaryOp opcode) : operand(operand), opcode(opcode)
        {
        }

        auto GetOperand() const noexcept -> const AstExpr*
        {
            return operand;
        }
        auto GetOpcode() const noexcept -> UnaryOp
        {
            return opcode;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return visitor.Traverse(*operand);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpAttribute("Opcode", UnaryOpToString(opcode));
            d.DumpChildNode("Operand", *operand);
        }
    };

    // Represents a binary expression like `foo + bar`.
    class AstBinaryExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> lhsOperand;

        // [Node]
        NotNull<AstExpr*> rhsOperand;

        // [Node]
        BinaryOp opcode;

    public:
        AstBinaryExpr(AstExpr* lhsOperand, AstExpr* rhsOperand, BinaryOp opcode)
            : lhsOperand(lhsOperand), rhsOperand(rhsOperand), opcode(opcode)
        {
        }

        auto GetLhsOperand() const noexcept -> const AstExpr*
        {
            return lhsOperand;
        }
        auto GetRhsOperand() const noexcept -> const AstExpr*
        {
            return rhsOperand;
        }
        auto GetOpcode() const noexcept -> BinaryOp
        {
            return opcode;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            if (!visitor.Traverse(*lhsOperand)) {
                return false;
            }
            if (!visitor.Traverse(*rhsOperand)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpAttribute("Opcode", BinaryOpToString(opcode));
            d.DumpChildNode("LhsOperand", *lhsOperand);
            d.DumpChildNode("RhsOperand", *rhsOperand);
        }
    };

    // Represents a ternary expression like `foo ? bar : baz`.
    class AstSelectExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> condition;

        // [Node]
        NotNull<AstExpr*> trueExpr;

        // [Node]
        NotNull<AstExpr*> falseExpr;

    public:
        AstSelectExpr(AstExpr* condition, AstExpr* trueExpr, AstExpr* falseExpr)
            : condition(condition), trueExpr(trueExpr), falseExpr(falseExpr)
        {
        }

        auto GetCondition() const noexcept -> const AstExpr*
        {
            return condition;
        }
        auto GetTrueExpr() const noexcept -> const AstExpr*
        {
            return trueExpr;
        }
        auto GetFalseExpr() const noexcept -> const AstExpr*
        {
            return falseExpr;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            if (!visitor.Traverse(*condition)) {
                return false;
            }
            if (!visitor.Traverse(*trueExpr)) {
                return false;
            }
            if (!visitor.Traverse(*falseExpr)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpChildNode("Condition", *condition);
            d.DumpChildNode("TrueExpr", *trueExpr);
            d.DumpChildNode("FalseExpr", *falseExpr);
        }
    };

    // Represents an implicit cast. This is a virtual node that is not present in the source code.
    class AstImplicitCastExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> operand;

        // Note the cast target type is AstExpr::deducedType.

    public:
        AstImplicitCastExpr(AstExpr* operand) : operand(operand)
        {
        }

        auto GetOperand() const noexcept -> const AstExpr*
        {
            return operand;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            return visitor.Traverse(*operand);
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpChildNode("Operand", *operand);
        }
    };

    // Represents a function call expression like `foo(bar, baz)`.
    class AstFunctionCallExpr final : public AstExpr
    {
    private:
        // [Node]
        AstSyntaxToken nameToken;

        // [Node]
        ArrayView</*NotNull*/ AstExpr*> args;

        // [Payload]
        const AstFunctionDecl* resolvedFunction = nullptr;

    public:
        AstFunctionCallExpr(AstSyntaxToken nameToken, ArrayView<AstExpr*> args) : nameToken(nameToken), args(args)
        {
        }

        auto GetNameToken() const noexcept -> AstSyntaxToken
        {
            return nameToken;
        }
        auto GetArgs() const noexcept -> ArrayView<const AstExpr*>
        {
            return args;
        }

        auto SetResolvedFunction(const AstFunctionDecl* decl)
        {
            this->resolvedFunction = decl;
        }
        auto GetResolvedFunction() const noexcept
        {
            return resolvedFunction;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            for (auto arg : args) {
                if (!visitor.Traverse(*arg)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpAttribute("Function", nameToken.IsIdentifier() ? nameToken.text.StrView() : "<Error>");
            for (auto arg : args) {
                d.DumpChildNode("Arg", *arg);
            }
        }
    };

    // Represents a constructor call expression like `vec3(1, 2, 3)`.
    class AstConstructorCallExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstQualType*> constructedType;

        // [Node]
        ArrayView</*NotNull*/ AstExpr*> args;

    public:
        AstConstructorCallExpr(AstQualType* constructedType, ArrayView<AstExpr*> args)
            : constructedType(constructedType), args(args)
        {
        }

        auto GetConstructedType() const noexcept -> const AstQualType*
        {
            return constructedType;
        }
        auto GetArgs() const noexcept -> ArrayView<const AstExpr*>
        {
            return args;
        }

        template <AstVisitorT Visitor>
        auto DoTraverse(Visitor& visitor) const -> bool
        {
            if (!AstExpr::DoTraverse(visitor)) {
                return false;
            }

            if (!visitor.Traverse(*constructedType)) {
                return false;
            }
            for (auto arg : args) {
                if (!visitor.Traverse(*arg)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto DoDump(Dumper& d) const -> void
        {
            AstExpr::DoDump(d);
            d.DumpChildNode("ConstructedType", *constructedType);
            for (auto arg : args) {
                d.DumpChildNode("Arg", *arg);
            }
        }
    };

} // namespace glsld