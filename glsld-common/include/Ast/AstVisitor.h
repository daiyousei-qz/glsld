#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"
#include "Compiler/AstContext.h"

#include <type_traits>

namespace glsld
{
    enum class AstVisitPolicy
    {
        // Visit the node and traverse its children.
        Traverse,

        // Visit the node, but not traverse its children.
        Visit,

        // Leave the node without visit and traversal.
        Leave,

        // Halt the traversal immediately.
        Halt,
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

        // Returns false if the traversal is halted.
        auto Traverse(const AstNode& astNode) -> bool
        {
            GLSLD_ASSERT(astNode.GetTag() != AstNodeTag::Invalid);

            auto& visitor = static_cast<Derived&>(*this);
            switch (astNode.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
    {                                                                                                                  \
        auto& dispatchedNode = static_cast<const TYPE&>(astNode);                                                      \
        auto visitPolicy     = AstVisitPolicy::Traverse;                                                               \
        /* Enter */                                                                                                    \
        if constexpr (requires { visitor.Enter##TYPE(dispatchedNode); }) {                                             \
            visitPolicy = visitor.Enter##TYPE(dispatchedNode);                                                         \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstNode(dispatchedNode); }) {                                       \
            visitPolicy = visitor.EnterAstNode(dispatchedNode);                                                        \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Halt) {                                                                     \
            return false;                                                                                              \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Leave) {                                                                    \
            break;                                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        /* Visit */                                                                                                    \
        if constexpr (requires { visitor.Visit##TYPE(dispatchedNode); }) {                                             \
            visitor.Visit##TYPE(dispatchedNode);                                                                       \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstNode(dispatchedNode); }) {                                       \
            visitor.VisitAstNode(dispatchedNode);                                                                      \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Visit) {                                                                    \
            break;                                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        /* Traverse */                                                                                                 \
        if (!dispatchedNode.DoTraverse(visitor)) {                                                                     \
            return false;                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        /* Exit */                                                                                                     \
        if constexpr (requires { visitor.Exit##TYPE(dispatchedNode); }) {                                              \
            visitor.Exit##TYPE(dispatchedNode);                                                                        \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstNode(dispatchedNode); }) {                                        \
            visitor.ExitAstNode(dispatchedNode);                                                                       \
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

            // By default, continue traversal after visiting the node.
            return true;
        }
    };
} // namespace glsld