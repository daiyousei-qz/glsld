#pragma once
#include "Ast.h"
#include "AstContext.h"

#include <type_traits>

namespace glsld
{
    enum class AstVisitPolicy
    {
        // Visit the node and traverse its children
        Traverse,

        // Visit the node, but not traverse its children
        Visit,

        // Leave the node without visit and traversal
        Leave,

        // Halt,
    };

    // Enter: Decision
    // Visit: Pre-order
    // Exit: Post-order
    template <typename Derived>
    class AstVisitor
    {
    public:
        AstVisitor()
        {
            static_assert(std::is_base_of_v<AstVisitor, Derived>);
        }

        auto Traverse(AstNodeBase& astNode) -> void
        {
            TraverseInternal(astNode);
        }
        auto Traverse(AstNodeBase* astNode) -> void
        {
            if (astNode) {
                TraverseInternal(*astNode);
            }
        }

    private:
        auto TraverseInternal(AstNodeBase& astNode) -> void
        {
            GLSLD_ASSERT(astNode.GetTag() != AstNodeTag::Invalid);

            auto& visitor = static_cast<Derived&>(*this);

            switch (astNode.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
    {                                                                                                                  \
        auto& dispatchedNode = static_cast<TYPE&>(astNode);                                                            \
        auto visitPolicy     = AstVisitPolicy::Traverse;                                                               \
        /* Enter */                                                                                                    \
        if constexpr (requires { visitor.Enter##TYPE(dispatchedNode); }) {                                             \
            visitPolicy = visitor.Enter##TYPE(dispatchedNode);                                                         \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstExpr(dispatchedNode); }) {                                       \
            visitPolicy = visitor.EnterAstExpr(dispatchedNode);                                                        \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstStmt(dispatchedNode); }) {                                       \
            visitPolicy = visitor.EnterAstStmt(dispatchedNode);                                                        \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstDecl(dispatchedNode); }) {                                       \
            visitPolicy = visitor.EnterAstDecl(dispatchedNode);                                                        \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstNodeBase(dispatchedNode); }) {                                   \
            visitPolicy = visitor.EnterAstNodeBase(dispatchedNode);                                                    \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Leave) {                                                                    \
            break;                                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        /* Visit */                                                                                                    \
        if constexpr (requires { visitor.Visit##TYPE(dispatchedNode); }) {                                             \
            visitor.Visit##TYPE(dispatchedNode);                                                                       \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstExpr(dispatchedNode); }) {                                       \
            visitor.VisitAstExpr(dispatchedNode);                                                                      \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstStmt(dispatchedNode); }) {                                       \
            visitor.VisitAstStmt(dispatchedNode);                                                                      \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstDecl(dispatchedNode); }) {                                       \
            visitor.VisitAstDecl(dispatchedNode);                                                                      \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstNodeBase(dispatchedNode); }) {                                   \
            visitor.VisitAstNodeBase(dispatchedNode);                                                                  \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Visit) {                                                                    \
            break;                                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        /* Traverse */                                                                                                 \
        dispatchedNode.Traverse(visitor);                                                                              \
                                                                                                                       \
        /* Exit */                                                                                                     \
        if constexpr (requires { visitor.Exit##TYPE(dispatchedNode); }) {                                              \
            visitor.Exit##TYPE(dispatchedNode);                                                                        \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstExpr(dispatchedNode); }) {                                        \
            visitor.ExitAstExpr(dispatchedNode);                                                                       \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstStmt(dispatchedNode); }) {                                        \
            visitor.ExitAstStmt(dispatchedNode);                                                                       \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstDecl(dispatchedNode); }) {                                        \
            visitor.ExitAstDecl(dispatchedNode);                                                                       \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstNodeBase(dispatchedNode); }) {                                    \
            visitor.ExitAstNodeBase(dispatchedNode);                                                                   \
        }                                                                                                              \
        break;                                                                                                         \
    }
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
            default:
                GLSLD_UNREACHABLE();
            }
        }
    };
} // namespace glsld