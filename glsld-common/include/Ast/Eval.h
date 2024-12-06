#pragma once
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

namespace glsld
{
    auto EvalAstExpr(const AstExpr& expr) -> ConstValue;
}