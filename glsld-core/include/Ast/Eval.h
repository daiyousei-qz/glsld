#pragma once
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

namespace glsld
{
    auto IsConstEvalFunction(StringView name) -> bool;

    // Evaluates an expression Ast node into a compile-time primitive.
    // Returns errorous value if the expression is not a compile-time primitive.
    auto EvalAstInitializer(const AstInitializer& expr) -> ConstValue;
} // namespace glsld