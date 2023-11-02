#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

#include <algorithm>
#include <array>

namespace glsld
{
    enum class UnaryOp
    {
        Identity,
        Negate,
        BitwiseNot,
        LogicalNot,
        PrefixInc,
        PrefixDec,
        PostfixInc,
        PostfixDec,
        Length,
    };
    inline constexpr auto UnaryOpToString(UnaryOp op) -> StringView
    {
        switch (op) {
        case UnaryOp::Identity:
            return "Identity";
        case UnaryOp::Negate:
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
    inline constexpr auto IsAssignmentOp(BinaryOp op) noexcept -> bool
    {
        switch (op) {
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
            return true;
        default:
            return false;
        }
    }
    inline constexpr auto BinaryOpToString(BinaryOp op) -> StringView
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
    inline constexpr auto JumpTypeToString(JumpType type) -> StringView
    {
        switch (type) {
        case JumpType::Break:
            return "Break";
        case JumpType::Continue:
            return "Continue";
        case JumpType::Discard:
            return "Discard";
        }

        GLSLD_UNREACHABLE();
    }

    enum class InterfaceBlockType
    {
        In,
        Out,
        Uniform,
        Buffer,
    };
    inline constexpr auto InterfaceBlockTypeToString(InterfaceBlockType type) -> StringView
    {
        switch (type) {
        case InterfaceBlockType::In:
            return "In";
        case InterfaceBlockType::Out:
            return "Out";
        case InterfaceBlockType::Uniform:
            return "Uniform";
        case InterfaceBlockType::Buffer:
            return "Buffer";
        }

        GLSLD_UNREACHABLE();
    }

    enum class NameAccessType
    {
        Unknown,
        Variable,
        Function,
        Constructor,
        Swizzle,
    };
    inline constexpr auto NameAccessTypeToString(NameAccessType type) -> StringView
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
        SwizzleDesc(uint8_t x)
        {
            GLSLD_ASSERT(x <= 3);
            data[0] = x;
        }
        SwizzleDesc(ArrayView<uint8_t> xs)
        {
            GLSLD_ASSERT(xs.size() <= 4 && std::ranges::all_of(xs, [](uint8_t x) { return x <= 3; }));
            std::ranges::copy(xs, data.begin());
        }

        auto IsValid() const noexcept -> bool
        {
            return std::ranges::any_of(data, [](uint8_t x) { return x != 0xff; });
        }

        auto GetDimension() const noexcept -> size_t
        {
            return std::ranges::count_if(data, [](uint8_t x) { return x != 0xff; });
        }

        auto ToString() const -> std::string
        {
            std::string buffer;
            for (auto i : data) {
                if (i > 3) {
                    break;
                }
                buffer += "xyzw"[i];
            }

            return buffer.empty() ? "__InvalidSwizzle" : buffer;
        }

    private:
        std::array<uint8_t, 4> data = {255, 255, 255, 255};
    };

    class QualifierGroup
    {
    public:
        auto HasStorageQual() const noexcept -> bool
        {
            return qConst || qIn || qOut || qInout || qAttribute || qUniform || qVarying || qBuffer || qShared;
        }

        auto CanDeclareInterfaceBlock() const noexcept -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

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

        // Extension: raytracing
        bool qRayPayloadEXT : 1     = false;
        bool qRayPayloadInEXT : 1   = false;
        bool qHitAttributeEXT : 1   = false;
        bool qCallableDataEXT : 1   = false;
        bool qCallableDataInEXT : 1 = false;

        // Extension: mesh shader
        bool qPerprimitiveNV : 1       = false;
        bool qPerviewNV : 1            = false;
        bool qTaskNV : 1               = false;
        bool qTaskPayloadSharedEXT : 1 = false;
    };

} // namespace glsld