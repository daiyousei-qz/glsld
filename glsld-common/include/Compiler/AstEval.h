#pragma once
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

namespace glsld
{
    auto EvaluateAstExpr(const AstExpr* expr) -> ConstValue;
} // namespace glsld