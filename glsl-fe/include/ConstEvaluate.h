#pragma once
#include "Semantic.h"
#include "Typing.h"

#include <cstdint>

namespace glsld
{
    enum class ConstValueType
    {
        Error,
        Bool,
        Int32,
        UInt32,
        Float,
        Double,
    };

    // FIXME:
    class ConstValue
    {
    public:
        ConstValue() = default;
        ConstValue(bool value) : valueType(ConstValueType::Bool), boolValue(value)
        {
        }
        ConstValue(int32_t value) : valueType(ConstValueType::Int32), intValue(value)
        {
        }
        ConstValue(uint32_t value) : valueType(ConstValueType::UInt32), uintValue(value)
        {
        }
        ConstValue(float value) : valueType(ConstValueType::Float), floatValue(value)
        {
        }
        ConstValue(double value) : valueType(ConstValueType::Double), doubleValue(value)
        {
        }

        auto IsErrorValue() const -> bool
        {
            return valueType == ConstValueType::Error;
        }
        auto GetValueType() const -> ConstValueType
        {
            return valueType;
        }

        auto GetBoolValue() const -> bool
        {
            GLSLD_ASSERT(valueType == ConstValueType::Bool);
            return boolValue;
        }

        auto GetIntValue() const -> int
        {
            GLSLD_ASSERT(valueType == ConstValueType::Int32);
            return intValue;
        }

        auto GetDoubleValue() const -> double
        {
            GLSLD_ASSERT(valueType == ConstValueType::Double);
            return doubleValue;
        }

        auto ToString() const -> std::string
        {
            switch (valueType) {
            case ConstValueType::Error:
                return fmt::format("<error>");
            case ConstValueType::Bool:
                return fmt::format("{}", boolValue);
            case ConstValueType::Int32:
                return fmt::format("{}", intValue);
            case ConstValueType::UInt32:
                return fmt::format("{}", uintValue);
            case ConstValueType::Float:
                return fmt::format("{}", floatValue);
            case ConstValueType::Double:
                return fmt::format("{}", doubleValue);
            }

            GLSLD_UNREACHABLE();
        }

        auto GetTypeDesc() const -> const TypeDesc*
        {
            switch (valueType) {
            case ConstValueType::Error:
                return GetErrorTypeDesc();
            case ConstValueType::Bool:
                return GetBuiltinTypeDesc(BuiltinType::Ty_bool);
            case ConstValueType::Int32:
                return GetBuiltinTypeDesc(BuiltinType::Ty_int);
            case ConstValueType::UInt32:
                return GetBuiltinTypeDesc(BuiltinType::Ty_uint);
            case ConstValueType::Float:
                return GetBuiltinTypeDesc(BuiltinType::Ty_float);
            case ConstValueType::Double:
                return GetBuiltinTypeDesc(BuiltinType::Ty_double);
            }

            GLSLD_UNREACHABLE();
        }

    private:
        ConstValueType valueType = ConstValueType::Error;

        union
        {
            bool boolValue;
            int32_t intValue;
            uint32_t uintValue;

            float floatValue;
            double doubleValue;
        };
    };

    inline auto EvaluateUnaryOp(UnaryOp op, ConstValue operand) -> ConstValue
    {
        switch (op) {
        case UnaryOp::Identity:
            return operand;
        case UnaryOp::Nagate:
            switch (operand.GetValueType()) {
            case ConstValueType::Int32:
            case ConstValueType::UInt32:
            case ConstValueType::Float:
            case ConstValueType::Double:
                return ConstValue{};
            default:
                return ConstValue{};
            }
        case UnaryOp::BitwiseNot:
            switch (operand.GetValueType()) {
            case ConstValueType::Int32:
            case ConstValueType::UInt32:
                return ConstValue{};
            default:
                return ConstValue{};
            }
        case UnaryOp::LogicalNot:
            switch (operand.GetValueType()) {
            case ConstValueType::Bool:
                return ConstValue{!operand.GetBoolValue()};
            default:
                return ConstValue{};
            }
        case UnaryOp::PrefixInc:
        case UnaryOp::PrefixDec:
        case UnaryOp::PostfixInc:
        case UnaryOp::PostfixDec:
            // Const value is immutable
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }
    inline auto EvaluateBinaryOp(BinaryOp op, ConstValue lhs, ConstValue rhs) -> ConstValue
    {
        switch (op) {
        case BinaryOp::Plus:
        case BinaryOp::Minus:
        case BinaryOp::Mul:
        case BinaryOp::Div:
        case BinaryOp::Modulo:
        case BinaryOp::Equal:
        case BinaryOp::NotEqual:
        case BinaryOp::Less:
        case BinaryOp::LessEq:
        case BinaryOp::Greater:
        case BinaryOp::GreaterEq:
        case BinaryOp::BitwiseAnd:
        case BinaryOp::BitwiseOr:
        case BinaryOp::BitwiseXor:
        case BinaryOp::LogicalAnd:
        case BinaryOp::LogicalOr:
        case BinaryOp::LogicalXor:
        case BinaryOp::ShiftLeft:
        case BinaryOp::ShiftRight:
            return ConstValue{};

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
            // These ops cannot operate on a const value
            return ConstValue{};
        }

        GLSLD_UNREACHABLE();
    }
    inline auto EvaluateSelectOp(ConstValue predicate, ConstValue ifBranchVal, ConstValue elseBranchVal) -> ConstValue
    {
        // Predicate must be bool
        if (predicate.GetValueType() != ConstValueType::Bool) {
            return ConstValue{};
        }

        // FIXME: type check if branch and else branch
        if (predicate.GetBoolValue()) {
            return ifBranchVal;
        }
        else {
            return elseBranchVal;
        }
    }
} // namespace glsld