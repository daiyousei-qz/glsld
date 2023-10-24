#pragma once
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

namespace glsld
{
    inline auto EvaluateAstExpr(const AstExpr* expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }
} // namespace glsld