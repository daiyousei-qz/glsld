#include "Ast/Eval.h"
#include "Ast/Base.h"
#include "Ast/Expr.h"
#include "Ast/Dispatch.h"
#include "Language/ConstValue.h"

namespace glsld
{
    // Case 1: already folded into a ConstValue
    // Case 2: a constant aggregate expression a) AstInitializerList b) AstConstructorCallExpr
    class LazyConstEvalResult
    {
    private:
        std::variant<ConstValue, ArrayView<const AstInitializer*>> result;

    public:
        LazyConstEvalResult() : result(ConstValue{})
        {
        }
        LazyConstEvalResult(ConstValue value) : result(std::move(value))
        {
        }
        LazyConstEvalResult(ArrayView<const AstInitializer*> lazyAggregate) : result(lazyAggregate)
        {
        }

        // The cast here should be safe as AstExpr has only one base class, which is AstInitializer.
        // FIXME: could we do this without the cast?
        LazyConstEvalResult(ArrayView<const AstExpr*> lazyAggregate)
            : result(ArrayView<const AstInitializer*>(
                  reinterpret_cast<AstInitializer const* const*>(lazyAggregate.data()), lazyAggregate.size()))
        {
        }

        auto IsConstValue() const -> bool
        {
            return std::holds_alternative<ConstValue>(result);
        }
        auto AsConstValue() const -> ConstValue
        {
            auto value = std::get_if<ConstValue>(&result);
            return value ? value->Clone() : ConstValue{};
        }

        auto IsLazyAggregate() const -> bool
        {
            return std::holds_alternative<ArrayView<const AstInitializer*>>(result);
        }
        auto AsLazyAggregate() const -> ArrayView<const AstInitializer*>
        {
            auto items = std::get_if<ArrayView<const AstInitializer*>>(&result);
            return items ? *items : ArrayView<const AstInitializer*>{};
        }
    };

    static auto EvalAstInitializerLazy(const AstInitializer& init) -> LazyConstEvalResult;

    static auto UnwrapConstEvalResult(const LazyConstEvalResult& result, size_t index) -> LazyConstEvalResult
    {
        if (result.IsConstValue()) {
            return result.AsConstValue().GetElement(index);
        }
        else if (auto aggregate = result.AsLazyAggregate(); aggregate.size() > index) {
            return EvalAstInitializerLazy(*aggregate[index]);
        }

        return LazyConstEvalResult{};
    }

    auto EvalUnaryConstExpr(UnaryOp op, const ConstValue& operand) -> ConstValue
    {
        if (operand.IsError()) {
            return ConstValue{};
        }

        switch (op) {
        case UnaryOp::Identity:
            return operand.Clone();
        case UnaryOp::Negate:
            return operand.ElemwiseNegate();
        case UnaryOp::BitwiseNot:
            return operand.ElemwiseBitNot();
        case UnaryOp::LogicalNot:
            return operand.ElemwiseLogicalNot();
        case UnaryOp::PrefixInc:
        case UnaryOp::PrefixDec:
        case UnaryOp::PostfixInc:
        case UnaryOp::PostfixDec:
            // Fast path. Returns error since constant expression cannot have side effect.
            return ConstValue{};
        case UnaryOp::Length:
            return operand.Length();
        }

        GLSLD_UNREACHABLE();
    }

    auto EvalBinaryConstExpr(BinaryOp op, const ConstValue& lhs, const ConstValue& rhs) -> ConstValue
    {
        if (lhs.IsError() || rhs.IsError()) {
            return ConstValue{};
        }

        switch (op) {
        case BinaryOp::Plus:
            return lhs.ElemwisePlus(rhs);
        case BinaryOp::Minus:
            return lhs.ElemwiseMinus(rhs);
        case BinaryOp::Mul:
            return lhs.ElemwiseMul(rhs);
        case BinaryOp::Div:
            return lhs.ElemwiseDiv(rhs);
        case BinaryOp::Modulo:
            return lhs.ElemwiseMod(rhs);
        case BinaryOp::Equal:
            return lhs.ElemwiseEquals(rhs);
        case BinaryOp::NotEqual:
            return lhs.ElemwiseNotEquals(rhs);
        case BinaryOp::Less:
            return lhs.ElemwiseLessThan(rhs);
        case BinaryOp::LessEq:
            return lhs.ElemwiseLessThanEq(rhs);
        case BinaryOp::Greater:
            return lhs.ElemwiseGreaterThan(rhs);
        case BinaryOp::GreaterEq:
            return lhs.ElemwiseGreaterThanEq(rhs);
        case BinaryOp::BitwiseAnd:
            return lhs.ElemwiseBitAnd(rhs);
        case BinaryOp::BitwiseOr:
            return lhs.ElemwiseBitOr(rhs);
        case BinaryOp::BitwiseXor:
            return lhs.ElemwiseBitXor(rhs);
        case BinaryOp::LogicalAnd:
            return lhs.ElemwiseLogicalAnd(rhs);
        case BinaryOp::LogicalOr:
            return lhs.ElemwiseLogicalOr(rhs);
        case BinaryOp::LogicalXor:
            return lhs.ElemwiseLogicalXor(rhs);
        case BinaryOp::ShiftLeft:
            return lhs.ElemwiseShiftLeft(rhs);
        case BinaryOp::ShiftRight:
            return lhs.ElemwiseShiftRight(rhs);
        case BinaryOp::Comma:
        case BinaryOp::Assign:
        case BinaryOp::MulAssign:
        case BinaryOp::DivAssign:
        case BinaryOp::ModAssign:
        case BinaryOp::AddAssign:
        case BinaryOp::SubAssign:
        case BinaryOp::LShiftAssign:
        case BinaryOp::RShiftAssign:
        case BinaryOp::AndAssign:
        case BinaryOp::XorAssign:
        case BinaryOp::OrAssign:
            // Fast path. These ops cannot produce a constant expression.
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }

    auto EvalBuiltinFunction1(StringView funcName, const AstExpr* firstArg) -> ConstValue
    {
        auto firstArgResult = EvalAstExpr(*firstArg);
        if (firstArgResult.IsError()) {
            return ConstValue{};
        }

        if (funcName == "radians") {
            return firstArgResult.ElemwiseRadians();
        }
        else if (funcName == "degrees") {
            return firstArgResult.ElemwiseDegrees();
        }
        else if (funcName == "sin") {
            return firstArgResult.ElemwiseSin();
        }
        else if (funcName == "cos") {
            return firstArgResult.ElemwiseCos();
        }
        else if (funcName == "asin") {
            return firstArgResult.ElemwiseAsin();
        }
        else if (funcName == "acos") {
            return firstArgResult.ElemwiseAcos();
        }
        else if (funcName == "exp") {
            return firstArgResult.ElemwiseExp();
        }
        else if (funcName == "log") {
            return firstArgResult.ElemwiseLog();
        }
        else if (funcName == "exp2") {
            return firstArgResult.ElemwiseExp2();
        }
        else if (funcName == "log2") {
            return firstArgResult.ElemwiseLog2();
        }
        else if (funcName == "sqrt") {
            return firstArgResult.ElemwiseSqrt();
        }
        else if (funcName == "inversesqrt") {
            return firstArgResult.ElemwiseInverseSqrt();
        }
        else if (funcName == "abs") {
            return firstArgResult.ElemwiseAbs();
        }
        else if (funcName == "sign") {
            return firstArgResult.ElemwiseSign();
        }
        else if (funcName == "floor") {
            return firstArgResult.ElemwiseFloor();
        }
        else if (funcName == "trunc") {
            return firstArgResult.ElemwiseTrunc();
        }
        else if (funcName == "round") {
            return firstArgResult.ElemwiseRound();
        }
        else if (funcName == "ceil") {
            return firstArgResult.ElemwiseCeil();
        }

        return ConstValue{};
    }
    static auto EvalBuiltinFunction2(StringView funcName, const AstExpr* firstArg, const AstExpr* secondArg)
        -> ConstValue
    {
        auto firstArgResult  = EvalAstExpr(*firstArg);
        auto secondArgResult = EvalAstExpr(*secondArg);
        if (firstArgResult.IsError() || secondArgResult.IsError()) {
            return ConstValue{};
        }

        if (funcName == "pow") {
            return firstArgResult.ElemwisePow(secondArgResult);
        }

        return ConstValue{};
    }
    static auto EvalBuiltinFunction3(StringView funcName, const AstExpr* firstArg, const AstExpr* secondArg,
                                     const AstExpr* thirdArg) -> ConstValue
    {
        auto firstArgResult  = EvalAstExpr(*firstArg);
        auto secondArgResult = EvalAstExpr(*secondArg);
        auto thirdArgResult  = EvalAstExpr(*thirdArg);
        if (firstArgResult.IsError() || secondArgResult.IsError() || thirdArgResult.IsError()) {
            return ConstValue{};
        }

        if (funcName == "clamp") {
            return firstArgResult.ElemwiseClamp(secondArgResult, thirdArgResult);
        }

        return ConstValue{};
    }

    static auto EvalAstInitializerLazy(const AstInitializer& init) -> LazyConstEvalResult
    {
        if (!init.IsConst()) {
            return LazyConstEvalResult{};
        }

        if (auto ilist = init.As<AstInitializerList>(); ilist) {
            return LazyConstEvalResult{ilist->GetItems()};
        }
        else if (init.Is<AstErrorExpr>()) {
            return LazyConstEvalResult{};
        }
        else if (auto literalExpr = init.As<AstLiteralExpr>(); literalExpr) {
            return LazyConstEvalResult{literalExpr->GetValue().Clone()};
        }
        else if (auto nameAccessExpr = init.As<AstNameAccessExpr>(); nameAccessExpr) {
            if (auto decl = nameAccessExpr->GetResolvedDecl(); decl.IsValid()) {
                if (auto varDecl = decl.GetDecl()->As<AstVariableDecl>(); varDecl) {
                    if (auto init2 = varDecl->GetDeclarators()[decl.GetIndex()].initializer; init2) {
                        return EvalAstInitializerLazy(*init2);
                    }
                }
            }
        }
        else if (auto fieldAccessExpr = init.As<AstFieldAccessExpr>(); fieldAccessExpr) {
            if (auto decl = fieldAccessExpr->GetResolvedDecl(); decl.IsValid()) {
                if (auto fieldDecl = decl.GetDecl()->As<AstFieldDecl>(); fieldDecl) {
                    auto lhsResult = EvalAstInitializerLazy(*fieldAccessExpr->GetLhsExpr());
                    return UnwrapConstEvalResult(lhsResult, fieldDecl->GetFieldIndex());
                }
            }
        }
        else if (auto swizzleAccessExpr = init.As<AstSwizzleAccessExpr>(); swizzleAccessExpr) {
            // All swizzle access should be evaluated eagerly
            return EvalAstExpr(*swizzleAccessExpr->GetLhsExpr()).GetSwizzle(swizzleAccessExpr->GetSwizzleDesc());
        }
        else if (auto unaryExpr = init.As<AstUnaryExpr>(); unaryExpr) {
            // Because `.length()` works on aggregate as well, we need to handle it here.
            auto operandResult = EvalAstInitializerLazy(*unaryExpr->GetOperand());
            if (operandResult.IsLazyAggregate()) {
                if (unaryExpr->GetOpcode() == UnaryOp::Length) {
                    return ConstValue::CreateScalar(static_cast<int>(operandResult.AsLazyAggregate().size()));
                }
            }
            else {
                return EvalUnaryConstExpr(unaryExpr->GetOpcode(), operandResult.AsConstValue());
            }
        }
        else if (auto binaryExpr = init.As<AstBinaryExpr>(); binaryExpr) {
            // All binary expression should be evaluated eagerly.
            // That is, aggregate cannot be operand of binary expression.
            return EvalBinaryConstExpr(binaryExpr->GetOpcode(), EvalAstExpr(*binaryExpr->GetLhsOperand()),
                                       EvalAstExpr(*binaryExpr->GetRhsOperand()));
        }
        else if (auto selectExpr = init.As<AstSelectExpr>(); selectExpr) {
            auto conditionResult = EvalAstExpr(*selectExpr->GetCondition());
            if (conditionResult.IsScalarBool()) {
                return EvalAstInitializerLazy(conditionResult.GetBoolValue() ? *selectExpr->GetTrueExpr()
                                                                             : *selectExpr->GetFalseExpr());
            }
        }
        else if (auto implicitCastExpr = init.As<AstImplicitCastExpr>(); implicitCastExpr) {
            // FIXME: implment this
        }
        else if (auto fnCallExpr = init.As<AstFunctionCallExpr>(); fnCallExpr) {
            if (fnCallExpr->GetArgs().size() == 1) {
                return EvalBuiltinFunction1(fnCallExpr->GetFunctionName().text.StrView(), fnCallExpr->GetArgs()[0]);
            }
            else if (fnCallExpr->GetArgs().size() == 2) {
                return EvalBuiltinFunction2(fnCallExpr->GetFunctionName().text.StrView(), fnCallExpr->GetArgs()[0],
                                            fnCallExpr->GetArgs()[1]);
            }
            else if (fnCallExpr->GetArgs().size() == 3) {
                return EvalBuiltinFunction3(fnCallExpr->GetFunctionName().text.StrView(), fnCallExpr->GetArgs()[0],
                                            fnCallExpr->GetArgs()[1], fnCallExpr->GetArgs()[2]);
            }
        }
        else if (auto ctorCallExpr = init.As<AstConstructorCallExpr>(); ctorCallExpr) {
            std::vector<ConstValue> ctorArgs;
            for (const auto& arg : ctorCallExpr->GetArgs()) {
                ctorArgs.push_back(EvalAstExpr(*arg));
            }

            auto type = ctorCallExpr->GetConstructedType()->GetResolvedType();
            if (type->IsScalar()) {
                if (ctorCallExpr->GetArgs().empty()) {
                    // default constructor
                    // FIXME: is this legal?
                }
                else if (ctorCallExpr->GetArgs().size() == 1) {
                    // conversion constructor
                }
            }
            else if (type->IsVector()) {
            }
            else if (type->IsMatrix()) {
            }
            else if (type->IsArray() || type->IsStruct()) {
                return LazyConstEvalResult{ctorCallExpr->GetArgs()};
            }
        }

        return LazyConstEvalResult{};
    }

    auto EvalAstExpr(const AstExpr& expr) -> ConstValue
    {
        return EvalAstInitializerLazy(expr).AsConstValue();
    }
} // namespace glsld