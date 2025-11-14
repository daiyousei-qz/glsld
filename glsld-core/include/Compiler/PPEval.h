#pragma once
#include "Compiler/SyntaxToken.h"

namespace glsld
{
    // This class implements a simple expression evaluator for preprocessor expressions.
    class PPExprEvaluator
    {
    private:
        // See https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.pdf, section 3.3
        enum class PPOperator
        {
            // Parentheses are handled specially; assigned precedence 999 in implementation.
            LParen,

            // Precedence 2
            Identity,
            Negate,
            BitNot,
            LogicalNot,

            // Precedence 3
            Multiply,
            Divide,
            Modulo,

            // Precedence 4
            Add,
            Subtract,

            // Precedence 5
            LShift,
            RShift,

            // Precedence 6
            Less,
            LessEq,
            Greater,
            GreaterEq,

            // Precedence 7
            Equal,
            NotEqual,

            // Precedence 8
            BitAnd,

            // Precedence 9
            BitXor,

            // Precedence 10
            BitOr,

            // Precedence 11
            LogicalAnd,

            // Precedence 12
            LogicalOr,
        };

        struct PPOperatorInfo
        {
            PPOperator op;
            int precedence;
            bool isUnary;

            auto IsLeftAssociative() const -> bool
            {
                return !isUnary;
            }

            auto HasHigherPrecedenceThan(const PPOperatorInfo& other) const -> bool
            {
                return precedence < other.precedence;
            }
        };

        std::vector<int64_t> valueStack;
        std::vector<PPOperatorInfo> operatorStack;

    public:
        auto Evaluate(ArrayView<PPToken> tokens) -> std::optional<int64_t>;

    private:
        static auto GetParenPPOperator() -> PPOperatorInfo;
        static auto ParseUnaryPPOperator(const PPToken& token) -> std::optional<PPOperatorInfo>;
        static auto ParseBinaryPPOperator(const PPToken& token) -> std::optional<PPOperatorInfo>;

        // For parentheses, this function returns 0.
        // For unary operators, the right-hand side is NOT used.
        // This function returns 0 for invalid operations such as division by zero.
        static auto EvaluatePPOperator(PPOperator op, int64_t lhs, int64_t rhs = 0) -> int64_t;

        auto EvalOperatorOnTop() -> bool;

        auto PushOperator(PPOperatorInfo op) -> bool;

        auto PushOperand(int64_t value) -> bool;
    };
} // namespace glsld