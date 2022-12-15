#pragma once
#include "Common.h"
#include <string_view>
#include <array>

namespace glsld
{
    enum class ShaderStage
    {
        Unknown,
        Vertex,
        Fragment,
        Geometry,
        TessControl,
        TessEvaluation,
        Compute,
    };
    inline auto ShaderStageToString(ShaderStage stage) -> std::string_view
    {
        switch (stage) {
        case ShaderStage::Unknown:
            return "Unknown";
        case ShaderStage::Vertex:
            return "Vertex";
        case ShaderStage::Fragment:
            return "Fragment";
        case ShaderStage::Geometry:
            return "Geometry";
        case ShaderStage::TessControl:
            return "TessControl";
        case ShaderStage::TessEvaluation:
            return "TessEvaluation";
        case ShaderStage::Compute:
            return "Compute";
        }

        GLSLD_UNREACHABLE();
    }

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
        Length,
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
        case UnaryOp::Length:
            return "Length";
        }

        GLSLD_UNREACHABLE();
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
        }

        GLSLD_UNREACHABLE();
    }

    enum class JumpType
    {
        Break,
        Continue,
        Discard,
    };

    enum class InterfaceBlockType
    {
        In,
        Out,
        Uniform,
        Buffer,
    };

    enum class NameAccessType
    {
        Unknown,
        Variable,
        Function,
        Constructor,
        Swizzle,
    };
    inline auto NameAccessTypeToString(NameAccessType type) -> std::string_view
    {
        switch (type) {
        case NameAccessType::Unknown:
            return "Unknown";
        case NameAccessType::Variable:
            return "Variable";
        case NameAccessType::Function:
            return "Function";
        case NameAccessType::Constructor:
            return "Constructor";
        case NameAccessType::Swizzle:
            return "Swizzle";
        }

        GLSLD_UNREACHABLE();
    }

    class SwizzleDesc
    {
    public:
        SwizzleDesc() = default;
        SwizzleDesc(uint32_t x)
        {
            GLSLD_ASSERT(x <= 3);
            xs[0] = x;
        }

        auto IsValid() const noexcept -> bool
        {
            return xs[0] > 3;
        }

    private:
        std::array<uint8_t, 4> xs = {255, 255, 255, 255};
    };

    class QualifierGroup
    {
    public:
        auto CanDeclInterfaceBlock() -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

        auto HasStorageQual() -> bool
        {
            return qConst || qIn || qOut || qInout || qAttribute || qUniform || qVarying || qBuffer || qShared;
        }

        auto CanDeclareInterfaceBlock() -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

        auto GetHighp() -> bool
        {
            return qHighp;
        }
        auto SetHighp() -> void
        {
            qHighp = true;
        }
        auto GetMediump() -> bool
        {
            return qMediump;
        }
        auto SetMediump() -> void
        {
            qMediump = true;
        }
        auto GetLowp() -> bool
        {
            return qLowp;
        }
        auto SetLowp() -> void
        {
            qLowp = true;
        }

        auto SetConst() -> void
        {
            qConst = true;
        }
        auto GetConst() -> bool
        {
            return qConst;
        }
        auto SetIn() -> void
        {
            qIn = true;
        }
        auto GetIn() -> bool
        {
            return qIn;
        }
        auto SetOut() -> void
        {
            qOut = true;
        }
        auto GetOut() -> bool
        {
            return qOut;
        }
        auto SetInout() -> void
        {
            qInout = true;
        }
        auto GetInout() -> bool
        {
            return qInout;
        }
        auto SetAttribute() -> void
        {
            qAttribute = true;
        }
        auto GetAttribute() -> bool
        {
            return qAttribute;
        }
        auto SetUniform() -> void
        {
            qUniform = true;
        }
        auto GetUniform() -> bool
        {
            return qUniform;
        }
        auto SetVarying() -> void
        {
            qVarying = true;
        }
        auto GetVarying() -> bool
        {
            return qVarying;
        }
        auto SetBuffer() -> void
        {
            qBuffer = true;
        }
        auto GetBuffer() -> bool
        {
            return qBuffer;
        }
        auto SetShared() -> void
        {
            qShared = true;
        }
        auto GetShared() -> bool
        {
            return qShared;
        }

        auto SetCentroid() -> void
        {
            qCentroid = true;
        }
        auto GetCentroid() -> bool
        {
            return qCentroid;
        };
        auto SetSample() -> void
        {
            qSample = true;
        }
        auto GetSample() -> bool
        {
            return qSample;
        };
        auto SetPatch() -> void
        {
            qPatch = true;
        }
        auto GetPatch() -> bool
        {
            return qPatch;
        };

    public:
        // Precision Qualifier
        bool qHighp : 1   = false;
        bool qMediump : 1 = false;
        bool qLowp : 1    = false;

        // Storage/Parameter qualifiers
        bool qConst : 1     = false;
        bool qIn : 1        = false;
        bool qOut : 1       = false;
        bool qInout : 1     = false;
        bool qAttribute : 1 = false;
        bool qUniform : 1   = false;
        bool qVarying : 1   = false;
        bool qBuffer : 1    = false;
        bool qShared : 1    = false;

        // Auxiliary storage qualifiers
        bool qCentroid : 1 = false;
        bool qSample : 1   = false;
        bool qPatch : 1    = false;

        // Interpolation qualifiers
        bool qSmooth : 1        = false;
        bool qFlat : 1          = false;
        bool qNoperspective : 1 = false;

        // Variance qualifier
        bool qInvariant : 1 = false;

        // Precise qualifier
        bool qPrecise : 1 = false;

        // Memory qualifiers
        bool qCoherent : 1  = false;
        bool qVolatile : 1  = false;
        bool qRestrict : 1  = false;
        bool qReadonly : 1  = false;
        bool qWriteonly : 1 = false;
    };

} // namespace glsld