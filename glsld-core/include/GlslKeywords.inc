
// clang-format off
// DECL_KEYWORD(/* glsl keyword */)

#if !defined(DECL_KEYWORD)
#error "missing DECL_KEYWORD macro"
#else

DECL_KEYWORD(const)
DECL_KEYWORD(uniform)
DECL_KEYWORD(buffer)
DECL_KEYWORD(shared)
DECL_KEYWORD(attribute)
DECL_KEYWORD(varying)

DECL_KEYWORD(invariant)
DECL_KEYWORD(precise)

DECL_KEYWORD(coherent)
DECL_KEYWORD(volatile)
DECL_KEYWORD(restrict)
DECL_KEYWORD(readonly)
DECL_KEYWORD(writeonly)

DECL_KEYWORD(layout)

DECL_KEYWORD(centroid)
DECL_KEYWORD(flat)
DECL_KEYWORD(smooth)
DECL_KEYWORD(noperspective)

DECL_KEYWORD(patch)
DECL_KEYWORD(sample)

DECL_KEYWORD(subroutine)

DECL_KEYWORD(in)
DECL_KEYWORD(out)
DECL_KEYWORD(inout)

DECL_KEYWORD(lowp)
DECL_KEYWORD(mediump)
DECL_KEYWORD(highp)
DECL_KEYWORD(precision)

DECL_KEYWORD(struct)

DECL_KEYWORD(while)
DECL_KEYWORD(break)
DECL_KEYWORD(continue)
DECL_KEYWORD(do)
DECL_KEYWORD(else)
DECL_KEYWORD(for)
DECL_KEYWORD(if)
DECL_KEYWORD(discard)
DECL_KEYWORD(return)
DECL_KEYWORD(switch)
DECL_KEYWORD(case)
DECL_KEYWORD(default)

DECL_KEYWORD(true)
DECL_KEYWORD(false)

// Extension: raytracing
DECL_KEYWORD(rayPayloadNV)
DECL_KEYWORD(rayPayloadEXT)
DECL_KEYWORD(rayPayloadInNV)
DECL_KEYWORD(rayPayloadInEXT)
DECL_KEYWORD(hitAttributeNV)
DECL_KEYWORD(hitAttributeEXT)
DECL_KEYWORD(callableDataNV)
DECL_KEYWORD(callableDataEXT)
DECL_KEYWORD(callableDataInNV)
DECL_KEYWORD(callableDataInEXT)

// Extension: mesh shader
DECL_KEYWORD(perprimitiveNV)
DECL_KEYWORD(perviewNV)
DECL_KEYWORD(taskNV)
DECL_KEYWORD(taskPayloadSharedEXT)
DECL_KEYWORD(perprimitiveEXT)

// Builtin Types
#define DECL_BUILTIN_TYPE(KEYWORD, ...) DECL_KEYWORD(KEYWORD)
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE

#endif

// clang-format on