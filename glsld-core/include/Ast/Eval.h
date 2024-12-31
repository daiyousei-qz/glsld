#pragma once
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

namespace glsld
{
    auto IsConstEvalFunction(StringView name) -> bool;

    // Evaluates an expression Ast node into a compile-time primitive.
    // Returns errorous value if the expression is not a compile-time primitive.
    auto EvalAstExpr(const AstExpr& expr) -> ConstValue;

    // Evaluates an initializer into a compile-time primitive as if it's an implicit cast expression.
    auto EvalAstInitializer(const AstInitializer& init, const Type* type) -> ConstValue;
} // namespace glsld