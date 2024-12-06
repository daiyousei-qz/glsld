#include "Ast/Eval.h"
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Ast/Dispatch.h"
#include "Language/ConstValue.h"

namespace glsld
{
    static auto EvalAstExprAux(const AstNode& node) -> ConstValue
    {
        // unreachable
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstErrorExpr& expr) -> ConstValue
    {
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstLiteralExpr& expr) -> ConstValue
    {
        return expr.GetValue().Clone();
    }
    static auto EvalAstExprAux(const AstNameAccessExpr& expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstFieldAccessExpr& expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstSwizzleAccessExpr& expr) -> ConstValue
    {
        return EvalAstExpr(*expr.GetLhsExpr()).GetSwizzle(expr.GetSwizzleDesc());
    }
    static auto EvalAstExprAux(const AstIndexAccessExpr& expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstUnaryExpr& expr) -> ConstValue
    {
        return EvalUnaryConstExpr(expr.GetOpcode(), EvalAstExpr(*expr.GetOperand()));
    }
    static auto EvalAstExprAux(const AstBinaryExpr& expr) -> ConstValue
    {
        return EvalBinaryConstExpr(expr.GetOpcode(), EvalAstExpr(*expr.GetLhsOperand()),
                                   EvalAstExpr(*expr.GetRhsOperand()));
    }
    static auto EvalAstExprAux(const AstSelectExpr& expr) -> ConstValue
    {
        return EvalSelectConstExpr(EvalAstExpr(*expr.GetCondition()), EvalAstExpr(*expr.GetTrueExpr()),
                                   EvalAstExpr(*expr.GetFalseExpr()));
    }
    static auto EvalAstExprAux(const AstImplicitCastExpr& expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstFunctionCallExpr& expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }
    static auto EvalAstExprAux(const AstConstructorCallExpr& expr) -> ConstValue
    {
        // FIXME: implement this
        return ConstValue{};
    }

    auto EvalAstExpr(const AstExpr& expr) -> ConstValue
    {
        if (!expr.IsConst()) {
            return ConstValue{};
        }

        return InvokeAstDispatched(expr, [](const auto& dispatchedExpr) { return EvalAstExprAux(dispatchedExpr); });
    }
} // namespace glsld