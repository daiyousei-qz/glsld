#include "Compiler/AstEval.h"

namespace glsld
{
    auto EvaluateAstExpr(const AstExpr* expr) -> ConstValue
    {
        if (!expr->IsConst()) {
            return ConstValue{};
        }

        if (auto literalExpr = expr->As<AstLiteralExpr>()) {
            return literalExpr->GetValue().Clone();
        }

        // FIXME: implement this
        return ConstValue{};
    }
} // namespace glsld