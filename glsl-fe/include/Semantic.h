#pragma once

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

    enum class InterfaceBlockType
    {
        In,
        Out,
        Uniform,
        Buffer,
    };
} // namespace glsld