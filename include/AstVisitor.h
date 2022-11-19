#pragma once
#include "SyntaxTree.h"

#include <type_traits>

namespace glsld
{
    enum class AstVisitPolicy
    {
        Traverse,
        Visit,
        Leave,
    };

    // Enter
    // Visit
    // Exit
    template <typename Derived> class AstVisitor
    {
    public:
        AstVisitor()
        {
            static_assert(std::is_base_of_v<AstVisitor, Derived>);
        }

    public:
        auto TraverseInternal(AstExpr* expr) -> void
        {
            if (expr == nullptr)
            {
                return;
            }

            auto& visitor = static_cast<Derived&>(*this);

            switch (expr->GetOp())
            {
                // TODO: warn misuse
#define DECL_EXPROP(OPNAME, AST_TYPE)                                                                                  \
    case ExprOp::OPNAME:                                                                                               \
    {                                                                                                                  \
        auto dispatchedExpr = static_cast<AST_TYPE*>(expr);                                                            \
        auto visitPolicy    = AstVisitPolicy::Traverse;                                                                \
        if constexpr (requires { visitor.Enter##AST_TYPE(dispatchedExpr); })                                           \
        {                                                                                                              \
            visitPolicy = visitor.Enter##AST_TYPE(dispatchedExpr);                                                     \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstExpr(expr); })                                                   \
        {                                                                                                              \
            visitPolicy = visitor.EnterAstExpr(expr);                                                                  \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Leave)                                                                      \
        {                                                                                                              \
            break;                                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        if constexpr (requires { visitor.Visit##AST_TYPE(dispatchedExpr); })                                           \
        {                                                                                                              \
            visitor.Visit##AST_TYPE(dispatchedExpr);                                                                   \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstExpr(expr); })                                                   \
        {                                                                                                              \
            visitor.VisitAstExpr(expr);                                                                                \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Visit)                                                                      \
        {                                                                                                              \
            break;                                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        for (auto child : dispatchedExpr->GetChildren())                                                               \
        {                                                                                                              \
            TraverseInternal(child);                                                                                   \
        }                                                                                                              \
                                                                                                                       \
        if constexpr (requires { visitor.Exit##AST_TYPE(dispatchedExpr); })                                            \
        {                                                                                                              \
            visitor.Exit##AST_TYPE(dispatchedExpr);                                                                    \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstExpr(expr); })                                                    \
        {                                                                                                              \
            visitor.ExitAstExpr(expr);                                                                                 \
        }                                                                                                              \
        break;                                                                                                         \
    }
#include "GlslExprOp.inc"
#undef DECL_EXPROP
            default:
                GLSLD_UNREACHABLE();
            }
        }
    }; // namespace glsld
} // namespace glsld