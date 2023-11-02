#pragma once

namespace glsld
{
    // An enum of all builtin types in glsl language
    enum class GlslBuiltinType
    {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, ...) Ty_##GLSL_TYPE,
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
    };

} // namespace glsld