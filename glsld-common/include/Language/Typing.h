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

    enum class ScalarKind
    {
        // Base language
        Bool,
        Int,
        Uint,
        Float,
        Double,

        // Extension
        Int8,
        Int16,
        Int64,
        Uint8,
        Uint16,
        Uint64,
        Float16,
    };

    // Excluding bool type
    auto IsIntegralScalarType(ScalarKind type) -> bool;

    auto IsFloatPointScalarType(ScalarKind type) -> bool;

    auto IsIntergralPromotion(ScalarKind from, ScalarKind to) -> bool;
    auto IsFPPromotion(ScalarKind from, ScalarKind to) -> bool;
    auto IsScalarPromotion(ScalarKind from, ScalarKind to) -> bool;
    // TODO: should we match exactly glslang's implementation?
    auto IsIntegralConversion(ScalarKind from, ScalarKind to) -> bool;
    auto IsFPConversion(ScalarKind from, ScalarKind to) -> bool;
    auto IsFPIntegralConversion(ScalarKind from, ScalarKind to) -> bool;
    auto IsScalarConversion(ScalarKind from, ScalarKind to) -> bool;

    // FIXME: This is the explicit arithmetic type extension logic
    //        maybe we should follow the core glsl.
    auto HasBetterConversionAux(ScalarKind from, ScalarKind lhsTo, ScalarKind rhsTo) -> bool;
} // namespace glsld
