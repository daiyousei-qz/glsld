#include "Language/Typing.h"
#include "Basic/Common.h"

namespace glsld
{
    auto IsIntegralScalarType(ScalarKind type) -> bool
    {
        switch (type) {
        case ScalarKind::Int:
        case ScalarKind::Uint:
        case ScalarKind::Int8:
        case ScalarKind::Int16:
        case ScalarKind::Int64:
        case ScalarKind::Uint8:
        case ScalarKind::Uint16:
        case ScalarKind::Uint64:
            return true;
        default:
            return false;
        }
    }

    auto IsFloatPointScalarType(ScalarKind type) -> bool
    {
        switch (type) {
        case ScalarKind::Float:
        case ScalarKind::Double:
        case ScalarKind::Float16:
            return true;
        default:
            return false;
        }
    }

    auto IsIntergralPromotion(ScalarKind from, ScalarKind to) -> bool
    {
        if (to == ScalarKind::Int) {
            switch (from) {
            case ScalarKind::Int8:
            case ScalarKind::Int16:
            case ScalarKind::Uint8:
            case ScalarKind::Uint16:
                return true;
            default:
                break;
            }
        }

        return false;
    };
    auto IsFPPromotion(ScalarKind from, ScalarKind to) -> bool
    {
        if (to == ScalarKind::Double) {
            switch (from) {
            case ScalarKind::Float16:
            case ScalarKind::Float:
                return true;
            default:
                break;
            }
        }

        return false;
    };
    auto IsScalarPromotion(ScalarKind from, ScalarKind to) -> bool
    {
        return IsIntergralPromotion(from, to) || IsFPPromotion(from, to);
    }

    // TODO: should we match exactly glslang's implementation?
    auto IsIntegralConversion(ScalarKind from, ScalarKind to) -> bool
    {
        switch (from) {
        case ScalarKind::Int:
            switch (to) {
            case ScalarKind::Uint:
            case ScalarKind::Int64:
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Uint:
            switch (to) {
            case ScalarKind::Int64:
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Int8:
            switch (to) {
            case ScalarKind::Uint8:
            case ScalarKind::Int16:
            case ScalarKind::Uint16:
            // case ScalarType::Int:
            case ScalarKind::Uint:
            case ScalarKind::Int64:
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Int16:
            switch (to) {
            case ScalarKind::Uint16:
            // case ScalarType::Int:
            case ScalarKind::Uint:
            case ScalarKind::Int64:
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Int64:
            switch (to) {
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Uint8:
            switch (to) {
            // case ScalarType::Int8:
            case ScalarKind::Int16:
            case ScalarKind::Uint16:
            // case ScalarType::Int:
            case ScalarKind::Uint:
            case ScalarKind::Int64:
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Uint16:
            switch (to) {
            // case ScalarType::Int:
            case ScalarKind::Uint:
            case ScalarKind::Int64:
            case ScalarKind::Uint64:
                return true;
            default:
                break;
            }
            break;

            // case ScalarType::Uint64:
            //     switch (to) {
            //     case ScalarType::Int64:
            //         return true;
            //     default:
            //         break;
            //     }
            //     break;

        default:
            break;
        }

        return false;
    }
    auto IsFPConversion(ScalarKind from, ScalarKind to) -> bool
    {
        if (to == ScalarKind::Float && from == ScalarKind::Float16) {
            return true;
        }
        else {
            return false;
        }
    }
    auto IsFPIntegralConversion(ScalarKind from, ScalarKind to) -> bool
    {
        switch (from) {
        case ScalarKind::Int:
        case ScalarKind::Uint:
            switch (to) {
            case ScalarKind::Float:
            case ScalarKind::Double:
                return true;
            default:
                break;
            }
            break;

        case ScalarKind::Int8:
        case ScalarKind::Uint8:
        case ScalarKind::Int16:
        case ScalarKind::Uint16:
            switch (to) {
            case ScalarKind::Float16:
            case ScalarKind::Float:
            case ScalarKind::Double:
                return true;
            default:
                break;
            }
            break;

        default:
            break;
        }

        return false;
    }
    auto IsScalarConversion(ScalarKind from, ScalarKind to) -> bool
    {
        return IsIntegralConversion(from, to) || IsFPConversion(from, to) || IsFPIntegralConversion(from, to);
    }

    // FIXME: This is the explicit arithmetic type extension logic
    //        maybe we should follow the core glsl.
    auto HasBetterConversionAux(ScalarKind from, ScalarKind lhsTo, ScalarKind rhsTo) -> bool
    {
        // Exact match should have been handled earlier
        GLSLD_ASSERT(from != lhsTo && from != rhsTo);

        // 2. Promotion
        auto isPromotionLhs = IsScalarPromotion(from, lhsTo);
        auto isPromotionRhs = IsScalarPromotion(from, rhsTo);
        if (isPromotionLhs) {
            return !isPromotionRhs;
        }
        if (isPromotionRhs) {
            return false;
        }

        // 3. Conversion (no FP-Integral)
        auto isConversionLhs = IsIntegralConversion(from, lhsTo) || IsFPConversion(from, lhsTo);
        auto isConversionRhs = IsIntegralConversion(from, rhsTo) || IsFPConversion(from, rhsTo);

        if (isConversionLhs) {
            return !isConversionRhs;
        }
        if (isConversionRhs) {
            return false;
        }

        // 4. FP-Integral
        auto isFPIntegralConversionLhs = IsFPIntegralConversion(from, lhsTo);
        auto isFPIntegralConversionRhs = IsFPIntegralConversion(from, lhsTo);

        if (isFPIntegralConversionLhs && isFPIntegralConversionRhs) {
            // float is better
            return lhsTo == ScalarKind::Float && rhsTo != ScalarKind::Float;
        }

        // FIXME: optimize the logic
        GLSLD_ASSERT(false && "We expect both are at least convertible");
        return false;
    }
} // namespace glsld