#pragma once

#include "AstExpr.h"
#include "AstStmt.h"
#include "AstDecl.h"

namespace glsld
{
    template <typename F, typename... Args>
    inline auto AstNode::DispatchInvoke(F&& f, Args&&... args)
    {
        switch (GetTag()) {
#define DECL_AST_BEGIN_BASE(TYPE)
#define DECL_AST_TYPE(TYPE)                                                                                            \
    case AstNodeTag::TYPE:                                                                                             \
        return f(static_cast<TYPE&>(*this), std::forward<Args>(args)...);
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