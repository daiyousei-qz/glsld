#pragma once
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Ast/Stmt.h"
#include "Ast/Decl.h"
#include "Ast/Misc.h"

namespace glsld
{
    template <typename F, typename... Args>
    auto InvokeAstDispatched(AstNode& node, F&& f, Args&&... args) -> decltype(auto)
    {
        switch (node.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return std::forward<F>(f)(static_cast<TYPE&>(node), std::forward<Args>(args)...);
#define DECL_AST_END_BASE(TYPE)
#include "Ast/GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_TYPE
#undef DECL_AST_END_BASE
        default:
            GLSLD_UNREACHABLE();
        }
    }

    template <typename F, typename... Args>
    auto InvokeAstDispatched(const AstNode& node, F&& f, Args&&... args) -> decltype(auto)
    {
        switch (node.GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return std::forward<F>(f)(static_cast<const TYPE&>(node), std::forward<Args>(args)...);
#define DECL_AST_END_BASE(TYPE)
#include "Ast/GlslAst.inc"
#undef DECL_AST_BEGIN_BASE
#undef DECL_AST_TYPE
#undef DECL_AST_END_BASE
        default:
            GLSLD_UNREACHABLE();
        }
    }
} // namespace glsld