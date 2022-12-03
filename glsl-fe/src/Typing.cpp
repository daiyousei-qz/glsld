#include "Typing.h"

namespace glsld
{
    constexpr auto IsIntergralPromotion(ScalarType from, ScalarType to) -> bool
    {
        if (to == ScalarType::Int) {
            switch (from) {
            case ScalarType::Int8:
            case ScalarType::Int16:
            case ScalarType::Uint8:
            case ScalarType::Uint16:
                return true;
            default:
                break;
            }
        }

        return false;
    };
    constexpr auto IsFPPromotion(ScalarType from, ScalarType to) -> bool
    {
        if (to == ScalarType::Double) {
            switch (from) {
            case ScalarType::Float16:
            case ScalarType::Float:
                return true;
            default:
                break;
            }
        }

        return false;
    };
    constexpr auto IsScalarPromotion(ScalarType from, ScalarType to) -> bool
    {
        return IsIntergralPromotion(from, to) || IsFPPromotion(from, to);
    }

    // TODO: should we match exactly glslang's implementation?
    constexpr auto IsIntegralConversion(ScalarType from, ScalarType to) -> bool
    {
        switch (from) {
        case ScalarType::Int:
            switch (to) {
            case ScalarType::Uint:
            case ScalarType::Int64:
            case ScalarType::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Uint:
            switch (to) {
            case ScalarType::Int64:
            case ScalarType::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Int8:
            switch (to) {
            case ScalarType::Uint8:
            case ScalarType::Int16:
            case ScalarType::Uint16:
            // case ScalarType::Int:
            case ScalarType::Uint:
            case ScalarType::Int64:
            case ScalarType::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Int16:
            switch (to) {
            case ScalarType::Uint16:
            // case ScalarType::Int:
            case ScalarType::Uint:
            case ScalarType::Int64:
            case ScalarType::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Int64:
            switch (to) {
            case ScalarType::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Uint8:
            switch (to) {
            // case ScalarType::Int8:
            case ScalarType::Int16:
            case ScalarType::Uint16:
            // case ScalarType::Int:
            case ScalarType::Uint:
            case ScalarType::Int64:
            case ScalarType::Uint64:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Uint16:
            switch (to) {
            // case ScalarType::Int:
            case ScalarType::Uint:
            case ScalarType::Int64:
            case ScalarType::Uint64:
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
    constexpr auto IsFPConversion(ScalarType from, ScalarType to) -> bool
    {
        if (to == ScalarType::Float && from == ScalarType::Float16) {
            return true;
        }
        else {
            return false;
        }
    }
    constexpr auto IsFPIntegralConversion(ScalarType from, ScalarType to) -> bool
    {
        switch (from) {
        case ScalarType::Int:
        case ScalarType::Uint:
            switch (to) {
            case ScalarType::Float:
            case ScalarType::Double:
                return true;
            default:
                break;
            }
            break;

        case ScalarType::Int8:
        case ScalarType::Uint8:
        case ScalarType::Int16:
        case ScalarType::Uint16:
            switch (to) {
            case ScalarType::Float16:
            case ScalarType::Float:
            case ScalarType::Double:
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
    constexpr auto IsScalarConversion(ScalarType from, ScalarType to) -> bool
    {
        return IsIntegralConversion(from, to) || IsFPConversion(from, to) || IsFPIntegralConversion(from, to);
    }

    // FIXME: This is the explicit arithmetic type extension logic
    //        maybe we should follow the core glsl.
    constexpr auto HasBetterConversionAux(const ScalarTypeDesc& from, const ScalarTypeDesc& lhsTo,
                                          const ScalarTypeDesc& rhsTo) -> bool
    {
        // 1. Exact match
        if (from.type == lhsTo.type) {
            return from.type != rhsTo.type;
        }
        if (from.type == rhsTo.type) {
            return false;
        }

        // 2. Promotion
        auto isPromotionLhs = IsScalarPromotion(from.type, lhsTo.type);
        auto isPromotionRhs = IsScalarPromotion(from.type, rhsTo.type);
        if (isPromotionLhs) {
            return !isPromotionRhs;
        }
        if (isPromotionRhs) {
            return false;
        }

        // 3. Conversion (no FP-Integral)
        auto isConversionLhs = IsIntegralConversion(from.type, lhsTo.type) || IsFPConversion(from.type, lhsTo.type);
        auto isConversionRhs = IsIntegralConversion(from.type, rhsTo.type) || IsFPConversion(from.type, rhsTo.type);

        if (isConversionLhs) {
            return !isConversionRhs;
        }
        if (isConversionRhs) {
            return false;
        }

        // 4. FP-Integral
        auto isFPIntegralConversionLhs = IsFPIntegralConversion(from.type, lhsTo.type);
        auto isFPIntegralConversionRhs = IsFPIntegralConversion(from.type, lhsTo.type);

        if (isFPIntegralConversionLhs && isFPIntegralConversionRhs) {
            // float is better
            return lhsTo.type == ScalarType::Float && rhsTo.type != ScalarType::Float;
        }

        // FIXME: optimize the logic
        GLSLD_ASSERT(false && "We expect both are at least convertible");
        return false;
    }

    auto TypeDesc::IsConvertibleTo(const TypeDesc* to) const -> bool
    {
        // Exact match
        if (this == to) {
            return true;
        }

        // Bottom type
        if (IsError()) {
            return true;
        }

        if (descPayload.index() == to->descPayload.index()) {
            if (IsScalar()) {
                const auto& fromDesc = *GetScalarDesc();
                const auto& toDesc   = *to->GetScalarDesc();
                if (IsScalarPromotion(fromDesc.type, toDesc.type) || IsScalarConversion(fromDesc.type, toDesc.type)) {
                    return true;
                }
            }

            // FIXME: other conversion
        }

        return false;
    }

    auto TypeDesc::HasBetterConversion(const TypeDesc* lhsTo, const TypeDesc* rhsTo) const -> bool
    {
        // Error never has a better conversion to anything
        if (IsError()) {
            return false;
        }

        // Assuming this type is convertible to both `lhsTo` and `rhsTo`
        GLSLD_ASSERT(descPayload.index() == lhsTo->descPayload.index() &&
                     descPayload.index() == rhsTo->descPayload.index());

        if (auto scalarDesc = GetScalarDesc()) {
            return HasBetterConversionAux(*scalarDesc, *lhsTo->GetScalarDesc(), *rhsTo->GetScalarDesc());
        }
        else {
            // FIXME: implement other type conversion
            if (this == lhsTo) {
                return this != rhsTo;
            }
            else {
                return false;
            }
        }
    }
} // namespace glsld