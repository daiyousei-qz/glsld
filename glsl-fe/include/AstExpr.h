#pragma once
#include "Common.h"
#include "AstImpl.h"
#include "Semantic.h"

#include <array>
#include <string_view>
#include <vector>
#include <string>
#include <iterator>

namespace glsld
{
    class AstInitializer : public AstImpl<AstInitializer>
    {
    };

    class AstInitializerList final : public AstImpl<AstInitializerList>
    {
    public:
        AstInitializerList(std::vector<AstInitializer*> items) : items(items)
        {
        }

        auto GetItems() const -> ArrayView<AstInitializer*>
        {
            return items;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            for (auto node : items) {
                visitor.Traverse(*node);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        std::vector<AstInitializer*> items;
    };

    class AstExpr : public AstImpl<AstExpr>
    {
    };

    class AstErrorExpr final : public AstImpl<AstErrorExpr>
    {
    public:
        AstErrorExpr()
        {
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }
    };

    class AstConstantExpr final : public AstImpl<AstConstantExpr>
    {
    public:
        AstConstantExpr(SyntaxToken tok) : valueToken(tok)
        {
        }

        auto GetToken() -> const SyntaxToken&
        {
            return valueToken;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("LiteralText: {}", valueToken.text.StrView());
        }

    private:
        SyntaxToken valueToken;
    };

    class AstNameAccessExpr final : public AstImpl<AstNameAccessExpr>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(accessChain);
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("AccessName: {}", accessName.text.StrView());
        }

    private:
        AstExpr* accessChain;

        // NOTE this could be keyword, such as constructor
        SyntaxToken accessName;
    };

    class AstUnaryExpr final : public AstImpl<AstUnaryExpr>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(operand);
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("Op: {}", UnaryOpToString(op));
        }

    private:
        UnaryOp op;
        AstExpr* operand;
    };
    class AstBinaryExpr final : public AstImpl<AstBinaryExpr>
    {
    public:
        AstBinaryExpr(BinaryOp op, AstExpr* lhs, AstExpr* rhs) : op(op), lhs(lhs), rhs(rhs)
        {
        }

        auto GetOperator() -> BinaryOp
        {
            return op;
        }
        auto GetLeftOperandExpr() -> AstExpr*
        {
            return lhs;
        }
        auto GetRightOperandExpr() -> AstExpr*
        {
            return rhs;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor)
        {
            visitor.Traverse(*lhs);
            visitor.Traverse(*rhs);
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("Op: {}", BinaryOpToString(op));
        }

    private:
        BinaryOp op;
        AstExpr* lhs;
        AstExpr* rhs;
    };
    class AstSelectExpr final : public AstImpl<AstSelectExpr>
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

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*predicate);
            visitor.Traverse(*ifBranch);
            visitor.Traverse(*elseBranch);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstExpr* ifBranch;
        AstExpr* elseBranch;
    };

    class AstInvokeExpr final : public AstImpl<AstInvokeExpr>
    {
    public:
        AstInvokeExpr(AstExpr* invokedExpr, std::vector<AstExpr*> args)
            : invokedExpr(invokedExpr), args(std::move(args))
        {
        }

        auto GetInvokedExpr() -> AstExpr*
        {
            return invokedExpr;
        }
        auto GetArguments() -> ArrayView<AstExpr*>
        {
            return args;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*invokedExpr);
            for (auto arg : args) {
                visitor.Traverse(*arg);
            }
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* invokedExpr;
        std::vector<AstExpr*> args;
    };

    class AstIndexAccessExpr final : public AstImpl<AstIndexAccessExpr>
    {
    public:
        AstIndexAccessExpr(AstExpr* invokedExpr, AstArraySpec* arraySpec)
            : invokedExpr(invokedExpr), arraySpec(arraySpec)
        {
        }

        auto GetInvokedExpr() -> AstExpr*
        {
            return invokedExpr;
        }
        auto GetArraySpec() -> AstArraySpec*
        {
            return arraySpec;
        }

        template <AstVisitorT Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
            visitor.Traverse(*invokedExpr);
            visitor.Traverse(*arraySpec);
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* invokedExpr;
        AstArraySpec* arraySpec;
    };
} // namespace glsld