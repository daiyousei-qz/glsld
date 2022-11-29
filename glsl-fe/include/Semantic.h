#pragma once
#include "Common.h"
#include <string_view>

namespace glsld
{
    enum class UnaryOp
    {
        Identity,
        Nagate,
        BitwiseNot,
        LogicalNot,
        PrefixInc,
        PrefixDec,
        PostfixInc,
        PostfixDec,
    };
    inline auto UnaryOpToString(UnaryOp op) -> std::string_view
    {
        switch (op) {
        case UnaryOp::Identity:
            return "Identity";
        case UnaryOp::Nagate:
            return "Nagate";
        case UnaryOp::BitwiseNot:
            return "BitwiseNot";
        case UnaryOp::LogicalNot:
            return "LogicalNot";
        case UnaryOp::PrefixInc:
            return "PrefixInc";
        case UnaryOp::PrefixDec:
            return "PrefixDec";
        case UnaryOp::PostfixInc:
            return "PostfixInc";
        case UnaryOp::PostfixDec:
            return "PostfixDec";
        default:
            GLSLD_UNREACHABLE();
        }
    }

    enum class BinaryOp
    {
        // misc
        Comma,

        // assignment ops
        Assign,
        MulAssign,
        DivAssign,
        ModAssign,
        AddAssign,
        SubAssign,
        LShiftAssign,
        RShiftAssign,
        AndAssign,
        XorAssign,
        OrAssign,

        // binary ops
        Plus,
        Minus,
        Mul,
        Div,
        Modulo,
        Equal,
        NotEqual,
        Less,
        LessEq,
        Greater,
        GreaterEq,
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        LogicalAnd,
        LogicalOr,
        LogicalXor,
        ShiftLeft,
        ShiftRight,
    };
    inline auto BinaryOpToString(BinaryOp op) -> std::string_view
    {
        switch (op) {
        case BinaryOp::Comma:
            return "Comma";
        case BinaryOp::Assign:
            return "Assign";
        case BinaryOp::MulAssign:
            return "MulAssign";
        case BinaryOp::DivAssign:
            return "DivAssign";
        case BinaryOp::ModAssign:
            return "ModAssign";
        case BinaryOp::AddAssign:
            return "AddAssign";
        case BinaryOp::SubAssign:
            return "SubAssign";
        case BinaryOp::LShiftAssign:
            return "LShiftAssign";
        case BinaryOp::RShiftAssign:
            return "RShiftAssign";
        case BinaryOp::AndAssign:
            return "AndAssign";
        case BinaryOp::XorAssign:
            return "XorAssign";
        case BinaryOp::OrAssign:
            return "OrAssign";
        case BinaryOp::Plus:
            return "Plus";
        case BinaryOp::Minus:
            return "Minus";
        case BinaryOp::Mul:
            return "Mul";
        case BinaryOp::Div:
            return "Div";
        case BinaryOp::Modulo:
            return "Modulo";
        case BinaryOp::Equal:
            return "Equal";
        case BinaryOp::NotEqual:
            return "NotEqual";
        case BinaryOp::Less:
            return "Less";
        case BinaryOp::LessEq:
            return "LessEq";
        case BinaryOp::Greater:
            return "Greater";
        case BinaryOp::GreaterEq:
            return "GreaterEq";
        case BinaryOp::BitwiseAnd:
            return "BitwiseAnd";
        case BinaryOp::BitwiseOr:
            return "BitwiseOr";
        case BinaryOp::BitwiseXor:
            return "BitwiseXor";
        case BinaryOp::LogicalAnd:
            return "LogicalAnd";
        case BinaryOp::LogicalOr:
            return "LogicalOr";
        case BinaryOp::LogicalXor:
            return "LogicalXor";
        case BinaryOp::ShiftLeft:
            return "ShiftLeft";
        case BinaryOp::ShiftRight:
            return "ShiftRight";
        default:
            GLSLD_UNREACHABLE();
        }
    }

    enum class JumpType
    {
        Break,
        Continue,
        Discard,
    };

    enum class InvocationType
    {
        FunctionCall,
        Indexing,
    };
    inline auto InvocationTypeToString(InvocationType type) -> std::string_view
    {
        switch (type) {
        case InvocationType::FunctionCall:
            return "FunctionCall";
        case InvocationType::Indexing:
            return "Indexing";
        default:
            GLSLD_UNREACHABLE();
        }
    }

    enum class InterfaceBlockType
    {
        In,
        Out,
        Uniform,
        Buffer,
    };
} // namespace glsld