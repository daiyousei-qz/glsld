#pragma once
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"

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

    struct AstVisitorConfig
    {
        bool traceTraversalPath = false;
    };

    namespace detail
    {
        struct AstVisitorPluginEmpty
        {
        };

        class AstVisitorPluginTrackPath
        {
        private:
            std::vector<const AstNode*> path;

        public:
            AstVisitorPluginTrackPath() = default;

            auto OnEnterAst(const AstNode& node) -> void
            {
                path.push_back(&node);
            }
            auto OnExitAst(const AstNode& node) -> void
            {
                path.pop_back();
            }

            auto GetTraversalPath() const -> ArrayView<const AstNode*>
            {
                return path;
            }

            auto GetTraversalDepth() const -> size_t
            {
                return path.size();
            }
        };

        template <AstVisitorConfig Config>
        class AstVisitorBase
            : protected std::conditional_t<Config.traceTraversalPath, AstVisitorPluginTrackPath, AstVisitorPluginEmpty>
        {
        public:
            auto OnEnterAst(const AstNode& node) -> void
            {
                if constexpr (Config.traceTraversalPath) {
                    AstVisitorPluginTrackPath::OnEnterAst(node);
                }
            }

            auto OnExitAst(const AstNode& node) -> void
            {
                if constexpr (Config.traceTraversalPath) {
                    AstVisitorPluginTrackPath::OnExitAst(node);
                }
            }
        };
    } // namespace detail

#if defined(GLSLD_DEBUG)
// We use virtual functions in debug mode to enable compile-time type checking and easier debugging for AstVisitor.
#define GLSLD_AST_VISITOR_USE_VTABLE 1
#endif

#if defined(GLSLD_AST_VISITOR_USE_VTABLE)
#define GLSLD_AST_VISITOR_OVERRIDE override
#else
#define GLSLD_AST_VISITOR_OVERRIDE
#endif

    // Enter: Decision
    // Visit: Pre-order
    // Exit: Post-order
    template <typename Derived, typename ResultType = void, AstVisitorConfig Config = {}>
    class AstVisitor : protected detail::AstVisitorBase<Config>
    {
    private:
        using BaseType = detail::AstVisitorBase<Config>;

        auto OnEnterAst(const AstNode& node) -> void
        {
            BaseType::OnEnterAst(node);
        }

        auto OnExitAst(const AstNode& node) -> void
        {
            BaseType::OnExitAst(node);
        }

    public:
        AstVisitor()
        {
            static_assert(std::is_base_of_v<AstVisitor, Derived>);
        }

#if defined(GLSLD_AST_VISITOR_USE_VTABLE)
        virtual ~AstVisitor() = default;
#endif

#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    auto Traverse(const TYPE& node) -> bool                                                                            \
    {                                                                                                                  \
        auto& visitor    = static_cast<Derived&>(*this);                                                               \
        auto visitPolicy = AstVisitPolicy::Traverse;                                                                   \
        /* Enter */                                                                                                    \
        if constexpr (requires { visitor.Enter##TYPE(node); }) {                                                       \
            visitPolicy = visitor.Enter##TYPE(node);                                                                   \
        }                                                                                                              \
        else if constexpr (requires { visitor.EnterAstNode(node); }) {                                                 \
            visitPolicy = visitor.EnterAstNode(node);                                                                  \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Halt) {                                                                     \
            return false;                                                                                              \
        }                                                                                                              \
        else if (visitPolicy == AstVisitPolicy::Leave) {                                                               \
            return true;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        /* Visit */                                                                                                    \
        if constexpr (requires { visitor.Visit##TYPE(node); }) {                                                       \
            visitor.Visit##TYPE(node);                                                                                 \
        }                                                                                                              \
        else if constexpr (requires { visitor.VisitAstNode(node); }) {                                                 \
            visitor.VisitAstNode(node);                                                                                \
        }                                                                                                              \
        if (visitPolicy == AstVisitPolicy::Visit) {                                                                    \
            return true;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        /* Traverse */                                                                                                 \
        visitor.OnEnterAst(node);                                                                                      \
        bool continueTraversal = node.DoTraverse(visitor);                                                             \
        visitor.OnExitAst(node);                                                                                       \
        if (!continueTraversal) {                                                                                      \
            return false;                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        /* Exit */                                                                                                     \
        if constexpr (requires { visitor.Exit##TYPE(node); }) {                                                        \
            visitor.Exit##TYPE(node);                                                                                  \
        }                                                                                                              \
        else if constexpr (requires { visitor.ExitAstNode(node); }) {                                                  \
            visitor.ExitAstNode(node);                                                                                 \
        }                                                                                                              \
                                                                                                                       \
        /* By default, continue traversal after visiting the node. */                                                  \
        return true;                                                                                                   \
    }
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

        // Returns false if the traversal is halted.
        auto Traverse(const AstNode& node) -> bool
        {
            switch (node.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
    {                                                                                                                  \
        return Traverse(static_cast<const TYPE&>(node));                                                               \
    }
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE
            case AstNodeTag::Invalid:
                GLSLD_UNREACHABLE();
            }

            // By default, continue traversal after visiting the node.
            return true;
        }

#if defined(GLSLD_AST_VISITOR_USE_VTABLE)
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_END_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    virtual auto Enter##TYPE(const TYPE& node) -> AstVisitPolicy                                                       \
    {                                                                                                                  \
        return EnterAstNode(node);                                                                                     \
    }                                                                                                                  \
    virtual auto Visit##TYPE(const TYPE& node) -> void                                                                 \
    {                                                                                                                  \
        return VisitAstNode(node);                                                                                     \
    }                                                                                                                  \
    virtual auto Exit##TYPE(const TYPE& node) -> void                                                                  \
    {                                                                                                                  \
        return ExitAstNode(node);                                                                                      \
    }
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_END_BASE
#undef DECL_AST_TYPE

        virtual auto EnterAstNode(const AstNode&) -> AstVisitPolicy
        {
            return AstVisitPolicy::Traverse;
        }
        virtual auto VisitAstNode(const AstNode&) -> void
        {
        }
        virtual auto ExitAstNode(const AstNode&) -> void
        {
        }

        virtual auto Finish() -> ResultType
        {
            if constexpr (std::is_same_v<ResultType, void>) {
                return;
            }
            else {
                return ResultType{};
            }
        }
#endif
    };

    template <AstVisitorT VisitorType>
    inline auto TraverseAst(VisitorType&& visitor, const AstNode& node)
    {
        visitor.Traverse(node);

        if constexpr (requires { visitor.Finish(); }) {
            return visitor.Finish();
        }
    }

    // Dispatch the given AST node to its real type and invoke the given function with the node as the first argument.
    template <typename F, typename... Args>
    inline auto InvokeAstDispatched(AstNode& node, F&& f, Args&&... args) -> decltype(auto)
    {
        switch (node.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return std::forward<F>(f)(static_cast<TYPE&>(node), std::forward<Args>(args)...);
#define DECL_AST_END_BASE(TYPE)
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_TYPE
#undef DECL_AST_END_BASE
        default:
            GLSLD_UNREACHABLE();
        }
    }

    // Dispatch the given AST node to its real type and invoke the given function with the node as the first argument.
    template <typename F, typename... Args>
    inline auto InvokeAstDispatched(const AstNode& node, F&& f, Args&&... args) -> decltype(auto)
    {
        switch (node.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return std::forward<F>(f)(static_cast<const TYPE&>(node), std::forward<Args>(args)...);
#define DECL_AST_END_BASE(TYPE)
#include "GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_TYPE
#undef DECL_AST_END_BASE
        default:
            GLSLD_UNREACHABLE();
        }
    }

} // namespace glsld