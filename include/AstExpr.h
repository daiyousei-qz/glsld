#pragma once
#include "Common.h"
#include "SyntaxToken.h"
#include "Typing.h"
#include "Semantic.h"

#include <array>
#include <span>
#include <string_view>
#include <vector>
#include <iterator>

namespace glsld
{
    //
    enum class ExprOp
    {
#define DECL_EXPROP(OPNAME, ...) OPNAME,
#include "GlslExprOp.inc"
#undef DECL_EXPROP
    };

    inline constexpr auto ExprOpToString(ExprOp op) noexcept -> std::string_view
    {
        switch (op) {
#define DECL_EXPROP(OPNAME, ...)                                                                                       \
    case ExprOp::OPNAME:                                                                                               \
        return #OPNAME;
#include "GlslExprOp.inc"
#undef DECL_EXPROP
        default:
            GLSLD_UNREACHABLE();
        }
    }

    class AstExpr;
    class AstErrorExpr;
    class AstConstantExpr;
    class AstVarAccessExpr;
    class AstUnaryExpr;
    class AstBinaryExpr;
    class AstSelectExpr;
    class AstInvokeExpr;

    class AstExpr : public SyntaxNode
    {
    public:
        AstExpr(ExprOp op) : op(op)
        {
        }

        auto GetOp() -> ExprOp
        {
            return op;
        }

        auto IsStmt() -> bool
        {
            // FIXME: implement this
            return false;
        }

    private:
        ExprOp op;

        // Ast
        //

        TypeDesc* deducedType = nullptr;
    };

    template <size_t N>
    class AstExprImpl : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return children;
        }

    protected:
        std::array<AstExpr*, N> children = {};
    };
    template <>
    class AstExprImpl<0> : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return {};
        }
    };
    class AstCompoundExprImpl : public AstExpr
    {
    public:
        using AstExpr::AstExpr;

        auto GetChildren() -> std::span<AstExpr*>
        {
            return children;
        }

    protected:
        std::vector<AstExpr*> children;
    };

    class AstErrorExpr final : public AstExprImpl<0>
    {
    public:
        AstErrorExpr() : AstExprImpl(ExprOp::Error)
        {
        }

    private:
    };
    class AstConstantExpr final : public AstExprImpl<0>
    {
    public:
        AstConstantExpr(LexString value) : AstExprImpl(ExprOp::Const), value(value)
        {
        }

    private:
        // The actual memory is hosted in the LexContext
        LexString value;
    };
    class AstVarAccessExpr final : public AstExprImpl<1>
    {
    public:
        AstVarAccessExpr(AstExpr* accessChain, LexString accessName)
            : AstExprImpl(ExprOp::VarAccess), accessName(accessName)
        {
        }
        AstVarAccessExpr(LexString accessName) : AstExprImpl(ExprOp::VarAccess), accessName(accessName)
        {
        }

        auto GetAccessChain() -> AstExpr*
        {
            return children[0];
        }
        auto GetAccessName() -> LexString
        {
            return accessName;
        }

    private:
        LexString accessName;
    };

    class AstUnaryExpr final : public AstExprImpl<1>
    {
    public:
        AstUnaryExpr(ExprOp op, AstExpr* operand) : AstExprImpl(op)
        {
            // TODO: assert op is unary
            children[0] = operand;
        }

        auto GetOperandExpr() -> AstExpr*
        {
            return children[0];
        }

    private:
    };
    class AstBinaryExpr final : public AstExprImpl<2>
    {
    public:
        AstBinaryExpr(ExprOp op, AstExpr* lhs, AstExpr* rhs) : AstExprImpl(op)
        {
            // TODO: assert op is binary
            children[0] = lhs;
            children[1] = rhs;
        }

        auto GetLeftOperandExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetRightOperandExpr() -> AstExpr*
        {
            return children[1];
        }

    private:
    };
    class AstSelectExpr final : public AstExprImpl<3>
    {
    public:
        AstSelectExpr(AstExpr* predicate, AstExpr* positive, AstExpr* negative) : AstExprImpl(ExprOp::Select)
        {
            children[0] = predicate;
            children[1] = positive;
            children[2] = negative;
        }

        auto GetConditionExpr() -> AstExpr*
        {
            return children[0];
        }
        auto GetPositiveExpr() -> AstExpr*
        {
            return children[1];
        }
        auto GetNegativeExpr() -> AstExpr*
        {
            return children[2];
        }

    private:
    };

    class AstInvokeExpr final : public AstCompoundExprImpl
    {
    public:
        AstInvokeExpr(InvocationType type, AstExpr* invokedExpr, std::vector<AstExpr*> args)
            : AstCompoundExprImpl(type == InvocationType::FunctionCall ? ExprOp::FunctionCall : ExprOp::IndexAccess)
        {
            children.push_back(invokedExpr);
            std::ranges::copy(args, std::back_inserter(children));
        }

        auto GetInvokedExpr() -> AstExpr*
        {
            return children[0];
        }

        auto GetArguments() -> std::span<AstExpr*>
        {
            return std::span<AstExpr*>(children).subspan(1);
        }

    private:
    };
} // namespace glsld