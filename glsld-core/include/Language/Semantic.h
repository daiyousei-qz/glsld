#pragma once
#include "Basic/Common.h"
#include "Support/StringView.h"

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

    class SwizzleDesc
    {
    private:
        std::array<uint8_t, 4> data = {255, 255, 255, 255};

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

        auto GetIndices() const noexcept -> ArrayView<uint8_t>
        {
            return {data.data(), GetDimension()};
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
    };

    struct QualifierGroup
    {
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

        auto HasStorageQual() const noexcept -> bool
        {
            return qConst || qIn || qOut || qInout || qAttribute || qUniform || qVarying || qBuffer || qShared;
        }

        auto CanDeclareInterfaceBlock() const noexcept -> bool
        {
            return qIn || qOut || qUniform || qBuffer;
        }

        auto ToString() const -> std::string
        {
            std::string buffer;
            if (qHighp) {
                buffer += "highp ";
            }
            if (qMediump) {
                buffer += "mediump ";
            }
            if (qLowp) {
                buffer += "lowp ";
            }
            if (qConst) {
                buffer += "const ";
            }
            if (qIn) {
                buffer += "in ";
            }
            if (qOut) {
                buffer += "out ";
            }
            if (qInout) {
                buffer += "inout ";
            }
            if (qAttribute) {
                buffer += "attribute ";
            }
            if (qUniform) {
                buffer += "uniform ";
            }
            if (qVarying) {
                buffer += "varying ";
            }
            if (qBuffer) {
                buffer += "buffer ";
            }
            if (qShared) {
                buffer += "shared ";
            }
            if (qCentroid) {
                buffer += "centroid ";
            }
            if (qSample) {
                buffer += "sample ";
            }
            if (qPatch) {
                buffer += "patch ";
            }
            if (qSmooth) {
                buffer += "smooth ";
            }
            if (qFlat) {
                buffer += "flat ";
            }
            if (qNoperspective) {
                buffer += "noperspective ";
            }
            if (qInvariant) {
                buffer += "invariant ";
            }
            if (qPrecise) {
                buffer += "precise ";
            }
            if (qCoherent) {
                buffer += "coherent ";
            }
            if (qVolatile) {
                buffer += "volatile ";
            }
            if (qRestrict) {
                buffer += "restrict ";
            }
            if (qReadonly) {
                buffer += "readonly ";
            }
            if (qWriteonly) {
                buffer += "writeonly ";
            }
            if (qRayPayloadEXT) {
                buffer += "rayPayloadEXT ";
            }
            if (qRayPayloadInEXT) {
                buffer += "rayPayloadInEXT ";
            }
            if (qHitAttributeEXT) {
                buffer += "hitAttributeEXT ";
            }
            if (qCallableDataEXT) {
                buffer += "callableDataEXT ";
            }
            if (qCallableDataInEXT) {
                buffer += "callableDataInEXT ";
            }
            if (qPerprimitiveNV) {
                buffer += "perprimitiveNV ";
            }
            if (qPerviewNV) {
                buffer += "perviewNV ";
            }
            if (qTaskNV) {
                buffer += "taskNV ";
            }
            if (qTaskPayloadSharedEXT) {
                buffer += "taskPayloadSharedEXT ";
            }

            // Remove trailing space
            if (!buffer.empty()) {
                buffer.pop_back();
            }
            return buffer;
        }

        auto operator==(const QualifierGroup& other) const noexcept -> bool = default;
    };

} // namespace glsld