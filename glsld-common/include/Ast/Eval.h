#pragma once
#include "Ast/Expr.h"
#include "Language/ConstValue.h"

namespace glsld
{
    // struct ConstEvalFunctionInfo
    // {
    //     StringView name;
    //     size_t numArgs;
    //     void* evalFunc;

    //     ConstEvalFunctionInfo(StringView name, auto (*evalFunc)(ConstValue)->ConstValue)
    //         : name(name), numArgs(0), evalFunc(static_cast<void*>(evalFunc))
    //     {
    //     }
    // };

    // Evaluates an expression Ast node into a compile-time primitive.
    // Returns errorous value if the expression is not a compile-time primitive.
    auto EvalAstExpr(const AstExpr& expr) -> ConstValue;
} // namespace glsld