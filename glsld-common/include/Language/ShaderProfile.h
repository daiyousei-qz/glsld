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

    enum class GlslVersionProfile
    {
        Core,
        Compatibility,
        Es,
    };

    enum class GlslShaderStage
    {
        Unknown,
        Vertex,
        Fragment,
        Geometry,
        TessControl,
        TessEvaluation,
        Compute,

        RayGeneration,
        RayIntersection,
        RayAnyHit,
        RayClosestHit,
        RayMiss,

        Task,
        Mesh,
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

    class LanguageContext
    {
    public:
    private:
        GlslVersion version = GlslVersion::Ver460;
    };
} // namespace glsld