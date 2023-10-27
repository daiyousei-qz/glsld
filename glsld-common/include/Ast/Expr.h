#pragma once
#include "Basic/Common.h"
#include "Ast/Base.h"
#include "Language/ConstValue.h"

#include <vector>

namespace glsld
{
    // Base class of all AST nodes that could be used as an initializer, one of:
    // - initializer list
    // - expression
    class AstInitializer : public AstNode
    {
    protected:
        AstInitializer() = default;

        template <AstDumperT Dumper>
        auto DumpPayload(Dumper& d) const -> void
        {
        }
    };

    class AstInitializerList final : public AstInitializer
    {
    private:
        // [Node]
        std::vector</*NotNull*/ AstInitializer*> items;

    public:
        AstInitializerList(std::vector<AstInitializer*> items) : items(std::move(items))
        {
        }

        auto GetItems() const -> ArrayView<const AstInitializer*>
        {
            return items;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (auto item : items) {
                if (!visitor.Traverse(*item)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
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
        // Whether this expression is const, i.e. can be evaluated at compile time.
        bool isConst = false;

        // [Payload]
        // The type that this expression is deduced to.
        const Type* deducedType = nullptr;

    protected:
        AstExpr() = default;

        template <AstDumperT Dumper>
        auto DumpPayload(Dumper& d) const -> void
        {
            AstInitializer::DumpPayload(d);
            d.DumpAttribute("IsConst", IsConst());
            d.DumpAttribute("DeducedType", GetDeducedType()->GetDebugName());
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
        }
    };

    class AstLiteralExpr final : public AstExpr
    {
    private:
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Value", value.ToString());
        }
    };

    class AstNameAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        SyntaxToken accessName;

        // [Payload]
        // The declaration that this name access expression refers to.
        DeclView resolvedDecl = {};

    public:
        AstNameAccessExpr(SyntaxToken name) : accessName(name)
        {
        }

        auto GetAccessName() const noexcept -> SyntaxToken
        {
            return accessName;
        }

        auto SetResolvedDecl(DeclView decl) noexcept -> void
        {
            resolvedDecl = decl;
        }
        auto GetResolvedDecl() const noexcept -> DeclView
        {
            return resolvedDecl;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Name", accessName.IsIdentifier() ? accessName.text.StrView() : "<Error>");
            d.DumpChildItem("ResolvedDecl", [this](Dumper& d) { resolvedDecl.Dump(d); });
        }
    };

    class AstFieldAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> lhsExpr;

        // [Node]
        SyntaxToken accessName;

        // [Payload]
        // The declaration that this name access expression refers to.
        DeclView resolvedDecl = {};

    public:
        AstFieldAccessExpr(AstExpr* lhsExpr, SyntaxToken name) : lhsExpr(lhsExpr), accessName(name)
        {
        }

        auto GetLhsExpr() const noexcept -> const AstExpr*
        {
            return lhsExpr;
        }
        auto GetAccessName() const noexcept -> SyntaxToken
        {
            return accessName;
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            return visitor.Traverse(*lhsExpr);
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Name", accessName.IsIdentifier() ? accessName.text.StrView() : "<Error>");
            d.DumpChildItem("ResolvedDecl", [this](Dumper& d) { resolvedDecl.Dump(d); });
            d.DumpChildNode("LhsExpr", *lhsExpr);
        }
    };

    class AstSwizzleAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> lhsExpr;

        // [Node]
        SyntaxToken accessName;

        // [Payload]
        // The swizzle description if this is a swizzle expression.
        SwizzleDesc swizzleDesc = {};

    public:
        AstSwizzleAccessExpr(AstExpr* lhsExpr, SyntaxToken name) : lhsExpr(lhsExpr), accessName(name)
        {
        }

        auto GetLhsExpr() const noexcept -> const AstExpr*
        {
            return lhsExpr;
        }
        auto GetAccessName() const noexcept -> SyntaxToken
        {
            return accessName;
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            return visitor.Traverse(*lhsExpr);
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Name", accessName.IsIdentifier() ? accessName.text.StrView() : "<Error>");
            d.DumpAttribute("Swizzle", swizzleDesc.ToString());
            d.DumpChildNode("LhsExpr", *lhsExpr);
        }
    };

    class AstIndexAccessExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstExpr*> baseExpr;

        // [Node]
        NotNull<AstArraySpec*> indices;

    public:
        AstIndexAccessExpr(AstExpr* baseExpr, AstArraySpec* indices) : baseExpr(baseExpr), indices(indices)
        {
        }

        auto GetBaseExpr() const noexcept -> const AstExpr*
        {
            return baseExpr;
        }
        auto GetIndices() const noexcept -> const AstArraySpec*
        {
            return indices;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*baseExpr)) {
                return false;
            }
            if (!visitor.Traverse(*indices)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpChildNode("BaseExpr", *baseExpr);
            d.DumpChildNode("Indices", *indices);
        }
    };

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
        auto Traverse(Visitor& visitor) const -> bool
        {
            return visitor.Traverse(*operand);
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Opcode", UnaryOpToString(opcode));
            d.DumpChildNode("Operand", *operand);
        }
    };

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
        auto Traverse(Visitor& visitor) const -> bool
        {
            if (!visitor.Traverse(*lhsOperand)) {
                return false;
            }
            if (!visitor.Traverse(*rhsOperand)) {
                return false;
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Opcode", BinaryOpToString(opcode));
            d.DumpChildNode("LhsOperand", *lhsOperand);
            d.DumpChildNode("RhsOperand", *rhsOperand);
        }
    };

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
        auto Traverse(Visitor& visitor) const -> bool
        {
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
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpChildNode("Condition", *condition);
            d.DumpChildNode("TrueExpr", *trueExpr);
            d.DumpChildNode("FalseExpr", *falseExpr);
        }
    };

    class AstFunctionCallExpr final : public AstExpr
    {
    private:
        // [Node]
        SyntaxToken functionName;

        // [Node]
        std::vector</*NotNull*/ AstExpr*> args;

        // [Payload]
        const AstFunctionDecl* resolvedFunction = nullptr;

    public:
        AstFunctionCallExpr(SyntaxToken functionName, std::vector<AstExpr*> args)
            : functionName(functionName), args(std::move(args))
        {
        }

        auto GetFunctionName() const noexcept -> SyntaxToken
        {
            return functionName;
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
        auto Traverse(Visitor& visitor) const -> bool
        {
            for (auto arg : args) {
                if (!visitor.Traverse(*arg)) {
                    return false;
                }
            }

            return true;
        }

        template <AstDumperT Dumper>
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpAttribute("Function", functionName.IsIdentifier() ? functionName.text.StrView() : "<Error>");
            for (auto arg : args) {
                d.DumpChildNode("Arg", *arg);
            }
        }
    };

    class AstConstructorCallExpr final : public AstExpr
    {
    private:
        // [Node]
        NotNull<AstQualType*> constructedType;

        // [Node]
        std::vector</*NotNull*/ AstExpr*> args;

    public:
        AstConstructorCallExpr(AstQualType* constructedType, std::vector<AstExpr*> args)
            : constructedType(constructedType), args(std::move(args))
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
        auto Traverse(Visitor& visitor) const -> bool
        {
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
        auto Dump(Dumper& d) const -> void
        {
            AstExpr::DumpPayload(d);
            d.DumpChildNode("ConstructedType", *constructedType);
            for (auto arg : args) {
                d.DumpChildNode("Arg", *arg);
            }
        }
    };

} // namespace glsld