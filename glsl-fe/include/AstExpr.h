#pragma once
#include "Common.h"
#include "AstBase.h"
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

    class AstExpr : public AstNodeBase
    {
    public:
    private:
        // Ast
        //

        TypeDesc* deducedType = nullptr;
    };

    class AstErrorExpr final : public AstExpr
    {
    public:
        AstErrorExpr()
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

    private:
    };

    class AstConstantExpr final : public AstExpr
    {
    public:
        AstConstantExpr(LexString value) : value(value)
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

    private:
        // The actual memory is hosted in the LexContext
        LexString value;
    };
    class AstConstructorExpr final : public AstExpr
    {
    public:
        // FIXME: use correct expr op
        AstConstructorExpr(AstQualType* type) : type(type)
        {
            GLSLD_ASSERT(type != nullptr);
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*type);
        }

    private:
        AstQualType* type;
    };
    class AstNameAccessExpr final : public AstExpr
    {
    public:
        AstNameAccessExpr(AstExpr* accessChain, SyntaxToken accessName)
            : accessChain(accessChain), accessName(accessName)
        {
        }
        AstNameAccessExpr(SyntaxToken accessName) : accessChain(nullptr), accessName(accessName)
        {
        }

        auto GetAccessChain() -> AstExpr*
        {
            return accessChain;
        }
        auto GetAccessName() -> SyntaxToken
        {
            return accessName;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            if (accessChain) {
                Traverse(*accessChain);
            }
        }

    private:
        AstExpr* accessChain;
        SyntaxToken accessName;
    };

    class AstUnaryExpr final : public AstExpr
    {
    public:
        AstUnaryExpr(UnaryOp op, AstExpr* operand) : op(op), operand(operand)
        {
        }

        auto GetOperator() -> UnaryOp
        {
            return op;
        }

        auto GetOperandExpr() -> AstExpr*
        {
            return operand;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

    private:
        UnaryOp op;
        AstExpr* operand;
    };
    class AstBinaryExpr final : public AstExpr
    {
    public:
        AstBinaryExpr(BinaryOp op, AstExpr* lhs, AstExpr* rhs) : op(op), lhs(lhs), rhs(rhs)
        {
        }

        auto GetLeftOperandExpr() -> AstExpr*
        {
            return lhs;
        }
        auto GetRightOperandExpr() -> AstExpr*
        {
            return rhs;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor)
        {
            visitor.Traverse(*lhs);
            visitor.Traverse(*rhs);
        }

    private:
        BinaryOp op;
        AstExpr* lhs;
        AstExpr* rhs;
    };
    class AstSelectExpr final : public AstExpr
    {
    public:
        AstSelectExpr(AstExpr* predicate, AstExpr* ifBranch, AstExpr* elseBranch)
            : predicate(predicate), ifBranch(ifBranch), elseBranch(elseBranch)
        {
        }

        auto GetPredicateExpr() -> AstExpr*
        {
            return predicate;
        }
        auto GetIfBranchExpr() -> AstExpr*
        {
            return ifBranch;
        }
        auto GetElseBranchExpr() -> AstExpr*
        {
            return elseBranch;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*ifBranch);
            visitor.Traverse(*elseBranch);
        }

    private:
        AstExpr* predicate;
        AstExpr* ifBranch;
        AstExpr* elseBranch;
    };

    class AstInvokeExpr final : public AstExpr
    {
    public:
        AstInvokeExpr(InvocationType type, AstExpr* invokedExpr, std::vector<AstExpr*> args)
            : type(type), invokedExpr(invokedExpr), args(std::move(args))
        {
        }

        auto GetInvokedExpr() -> AstExpr*
        {
            return invokedExpr;
        }

        auto GetArguments() -> std::span<AstExpr* const>
        {
            return args;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*invokedExpr);
            for (auto arg : args) {
                visitor.Traverse(*arg);
            }
        }

    private:
        InvocationType type;
        AstExpr* invokedExpr;
        std::vector<AstExpr*> args;
    };
} // namespace glsld