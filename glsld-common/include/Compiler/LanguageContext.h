#pragma once
#include "Basic/Common.h"

namespace glsld
{
    enum class GlslVersion
    {
        Ver110 = 110,
        Ver120 = 120,
        Ver130 = 130,
        Ver140 = 140,
        Ver150 = 150,
        Ver330 = 330,
        Ver400 = 400,
        Ver410 = 410,
        Ver420 = 420,
        Ver430 = 430,
        Ver440 = 440,
        Ver450 = 450,
        Ver460 = 460,
    };

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
    inline constexpr auto ShaderStageToString(ShaderStage stage) -> StringView
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

    class LanguageContext
    {
    public:
    private:
        GlslVersion version = GlslVersion::Ver460;
    };
} // namespace glsld