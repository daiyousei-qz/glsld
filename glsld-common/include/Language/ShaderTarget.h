#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"

namespace glsld
{
    enum class GlslVersion
    {
        Ver110 = 110,
        Ver120 = 120,
        Ver130 = 130,
        Ver140 = 140,
        Ver150 = 150,
        Ver300 = 300,
        Ver310 = 310,
        Ver330 = 330,
        Ver400 = 400,
        Ver410 = 410,
        Ver420 = 420,
        Ver430 = 430,
        Ver440 = 440,
        Ver450 = 450,
        Ver460 = 460,
    };

    enum class GlslProfile
    {
        Core,
        Compatibility,
        Es,
    };

    enum class GlslShaderStage
    {
        Unknown        = 0,
        Vertex         = 1,
        Fragment       = 2,
        Geometry       = 3,
        TessControl    = 4,
        TessEvaluation = 5,
        Compute        = 6,

        RayGeneration   = 7,
        RayIntersection = 8,
        RayAnyHit       = 9,
        RayClosestHit   = 10,
        RayMiss         = 11,
        RayCallable     = 12,

        Task = 13,
        Mesh = 14,
    };

    class GlslShaderStageBits
    {
    private:
        uint32_t bits = 0;

    public:
        GlslShaderStageBits() = default;

        auto TestBit(GlslShaderStage stage) const noexcept -> bool
        {
            return (bits & (1 << static_cast<uint32_t>(stage))) != 0;
        }

        auto SetBit(GlslShaderStage stage) noexcept -> void
        {
            bits |= 1 << static_cast<uint32_t>(stage);
        }
        auto ClearBit(GlslShaderStage stage) noexcept -> void
        {
            bits &= ~(1 << static_cast<uint32_t>(stage));
        }
        auto ClearAll() noexcept -> void
        {
            bits = 0;
        }
    };

    inline constexpr auto ShaderStageToString(GlslShaderStage stage) -> StringView
    {
        switch (stage) {
        case GlslShaderStage::Unknown:
            return "Unknown";
        case GlslShaderStage::Vertex:
            return "Vertex";
        case GlslShaderStage::Fragment:
            return "Fragment";
        case GlslShaderStage::Geometry:
            return "Geometry";
        case GlslShaderStage::TessControl:
            return "TessControl";
        case GlslShaderStage::TessEvaluation:
            return "TessEvaluation";
        case GlslShaderStage::Compute:
            return "Compute";
        case GlslShaderStage::RayGeneration:
            return "RayGeneration";
        case GlslShaderStage::RayIntersection:
            return "RayIntersection";
        case GlslShaderStage::RayAnyHit:
            return "RayAnyHit";
        case GlslShaderStage::RayClosestHit:
            return "RayClosestHit";
        case GlslShaderStage::RayMiss:
            return "RayMiss";
        case GlslShaderStage::Task:
            return "Task";
        case GlslShaderStage::Mesh:
            return "Mesh";
        }

        GLSLD_UNREACHABLE();
    }
} // namespace glsld