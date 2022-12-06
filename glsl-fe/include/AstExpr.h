#pragma once
#include "Common.h"
#include "AstBase.h"
#include "Semantic.h"

#include <array>
#include <string_view>
#include <vector>
#include <string>
#include <iterator>

#include <fmt/format.h>

namespace glsld
{
    class MSVC_EMPTY_BASES AstExpr : public AstNodeBase, public AstPayload<AstExpr>
    {
    };

    class MSVC_EMPTY_BASES AstErrorExpr final : public AstExpr, public AstPayload<AstErrorExpr>
    {
    public:
        AstErrorExpr()
        {
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
    };

    class MSVC_EMPTY_BASES AstConstantExpr final : public AstExpr, public AstPayload<AstConstantExpr>
    {
    public:
        AstConstantExpr(SyntaxToken tok) : valueToken(tok)
        {
        }

        auto GetToken() -> const SyntaxToken&
        {
            return valueToken;
        }

        template <typename Visitor>
        auto Traverse(Visitor& visitor) -> void
        {
        }

        auto DumpNodeData() const -> std::string
        {
            return fmt::format("LiteralText: {}", valueToken.text.StrView());
        }

    private:
        // FIXME: we don't need SyntaxToken::range here
        SyntaxToken valueToken;
    };

    class MSVC_EMPTY_BASES AstNameAccessExpr final : public AstExpr, public AstPayload<AstNameAccessExpr>
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

    class MSVC_EMPTY_BASES AstUnaryExpr final : public AstExpr, public AstPayload<AstUnaryExpr>
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
    class MSVC_EMPTY_BASES AstBinaryExpr final : public AstExpr, public AstPayload<AstBinaryExpr>
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

        template <typename Visitor>
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
    class MSVC_EMPTY_BASES AstSelectExpr final : public AstExpr, public AstPayload<AstSelectExpr>
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

        auto DumpNodeData() const -> std::string
        {
            return "";
        }

    private:
        AstExpr* predicate;
        AstExpr* ifBranch;
        AstExpr* elseBranch;
    };

    class MSVC_EMPTY_BASES AstInvokeExpr final : public AstExpr, public AstPayload<AstInvokeExpr>
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

        template <typename Visitor>
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

    class MSVC_EMPTY_BASES AstIndexAccessExpr final : public AstExpr, public AstPayload<AstIndexAccessExpr>
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

        template <typename Visitor>
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