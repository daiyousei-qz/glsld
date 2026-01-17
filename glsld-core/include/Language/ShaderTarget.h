#pragma once

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
} // namespace glsld