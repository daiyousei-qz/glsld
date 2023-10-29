#include "Language/Typing.h"

namespace glsld
{
    constexpr auto IsIntergralPromotion(ScalarKind from, ScalarKind to) -> bool
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
    constexpr auto IsFPPromotion(ScalarKind from, ScalarKind to) -> bool
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
    constexpr auto IsScalarPromotion(ScalarKind from, ScalarKind to) -> bool
    {
        return IsIntergralPromotion(from, to) || IsFPPromotion(from, to);
    }

    // TODO: should we match exactly glslang's implementation?
    constexpr auto IsIntegralConversion(ScalarKind from, ScalarKind to) -> bool
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
    constexpr auto IsFPConversion(ScalarKind from, ScalarKind to) -> bool
    {
        if (to == ScalarKind::Float && from == ScalarKind::Float16) {
            return true;
        }
        else {
            return false;
        }
    }
    constexpr auto IsFPIntegralConversion(ScalarKind from, ScalarKind to) -> bool
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
    constexpr auto IsScalarConversion(ScalarKind from, ScalarKind to) -> bool
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
            return lhsTo.type == ScalarKind::Float && rhsTo.type != ScalarKind::Float;
        }

        // FIXME: optimize the logic
        GLSLD_ASSERT(false && "We expect both are at least convertible");
        return false;
    }

    auto Type::GetErrorType() -> const Type*
    {
        static Type errorType{"__ErrorType", ErrorTypeDesc{}};
        return &errorType;
    }
    auto Type::GetBuiltinType(GlslBuiltinType tag) -> const Type*
    {
        switch (tag) {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, CPP_TYPE, DESC_PAYLOAD_TYPE, ...)                                                 \
    case GlslBuiltinType::Ty_##GLSL_TYPE:                                                                              \
    {                                                                                                                  \
        static Type typeDesc{#GLSL_TYPE, DESC_PAYLOAD_TYPE{__VA_ARGS__}};                                              \
        return &typeDesc;                                                                                              \
    }
#include "GlslType.inc"
#undef DECL_BUILTIN_TYPE
        }

        GLSLD_UNREACHABLE();
    }

    auto Type::GetScalarType(ScalarKind kind) -> const Type*
    {
        switch (kind) {
        case ScalarKind::Bool:
            return GetBuiltinType(GlslBuiltinType::Ty_bool);
        case ScalarKind::Int:
            return GetBuiltinType(GlslBuiltinType::Ty_int);
        case ScalarKind::Uint:
            return GetBuiltinType(GlslBuiltinType::Ty_uint);
        case ScalarKind::Float:
            return GetBuiltinType(GlslBuiltinType::Ty_float);
        case ScalarKind::Double:
            return GetBuiltinType(GlslBuiltinType::Ty_double);
        default:
            // FIXME: extension types
            return GetErrorType();
        }
    }

    auto Type::GetVectorType(ScalarKind type, size_t dim) -> const Type*
    {
        switch (type) {
        case ScalarKind::Bool:
            switch (dim) {
            case 2:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_bvec2);
            case 3:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_bvec3);
            case 4:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_bvec4);
            }
            break;
        case ScalarKind::Int:
            switch (dim) {
            case 2:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_ivec2);
            case 3:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_ivec3);
            case 4:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_ivec4);
            }
            break;
        case ScalarKind::Uint:
            switch (dim) {
            case 2:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_uvec2);
            case 3:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_uvec3);
            case 4:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_uvec4);
            }
            break;
        case ScalarKind::Float:
            switch (dim) {
            case 2:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_vec2);
            case 3:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_vec3);
            case 4:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_vec4);
            }
            break;
        case ScalarKind::Double:
            switch (dim) {
            case 2:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_dvec2);
            case 3:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_dvec3);
            case 4:
                return Type::GetBuiltinType(GlslBuiltinType::Ty_dvec4);
            }
            break;

        default:
            break;
        }

        return Type::GetErrorType();
    }

    auto Type::GetMatrixType(ScalarKind type, size_t dimRow, size_t dimCol) -> const Type*
    {
        if (type == ScalarKind::Float) {
            switch (dimRow) {
            case 2:
                switch (dimCol) {
                case 2:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat2);
                case 3:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat2x3);
                case 4:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat2x4);
                }
                break;
            case 3:
                switch (dimCol) {
                case 2:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat3x2);
                case 3:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat3);
                case 4:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat3x4);
                }
                break;
            case 4:
                switch (dimCol) {
                case 2:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat4x2);
                case 3:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat4x3);
                case 4:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_mat4);
                }
                break;
            }
        }
        else if (type == ScalarKind::Double) {
            switch (dimRow) {
            case 2:
                switch (dimCol) {
                case 2:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat2);
                case 3:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat2x3);
                case 4:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat2x4);
                }
                break;
            case 3:
                switch (dimCol) {
                case 2:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat3x2);
                case 3:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat3);
                case 4:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat3x4);
                }
                break;
            case 4:
                switch (dimCol) {
                case 2:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat4x2);
                case 3:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat4x3);
                case 4:
                    return Type::GetBuiltinType(GlslBuiltinType::Ty_dmat4);
                }
                break;
            }
        }

        return Type::GetErrorType();
    }

    auto Type::IsSameWith(GlslBuiltinType type) const -> bool
    {
        return this == Type::GetBuiltinType(type);
    }

    auto Type::IsSameWith(const Type* other) const -> bool
    {
        return this == other;
        // if (this == other) {
        //     return true;
        // }
        // else if (IsArray() && other->IsArray()) {
        //     // NOTE different modules could instantiate array types that's essentially the same
        //     return GetArrayDesc()->elementType->IsSameWith(other->GetArrayDesc()->elementType);
        // }
        // else {
        //     return false;
        // }
    }

    auto Type::IsConvertibleTo(const Type* to) const -> bool
    {
        // Exact match
        if (this->IsSameWith(to)) {
            return true;
        }

        // Bottom type
        if (IsError()) {
            return true;
        }

        if (IsScalar() && to->IsScalar()) {
            const auto& fromDesc = *GetScalarDesc();
            const auto& toDesc   = *to->GetScalarDesc();
            return IsScalarPromotion(fromDesc.type, toDesc.type) || IsScalarConversion(fromDesc.type, toDesc.type);
        }
        else if (IsVector() && to->IsVector()) {
            const auto& fromDesc = *GetVectorDesc();
            const auto& toDesc   = *to->GetVectorDesc();
            if (fromDesc.vectorSize == toDesc.vectorSize) {
                return IsScalarPromotion(fromDesc.scalarType, toDesc.scalarType) ||
                       IsScalarConversion(fromDesc.scalarType, toDesc.scalarType);
            }
        }
        else if (IsMatrix() && to->IsMatrix()) {
            const auto& fromDesc = *GetMatrixDesc();
            const auto& toDesc   = *to->GetMatrixDesc();
            if (fromDesc.dimCol == toDesc.dimCol && fromDesc.dimRow == toDesc.dimRow) {
                return toDesc.scalarType == ScalarKind::Double;
            }
        }

        return false;
    }

    auto Type::HasBetterConversion(const Type* lhsTo, const Type* rhsTo) const -> bool
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
            if (this->IsSameWith(lhsTo)) {
                return !this->IsSameWith(rhsTo);
            }
            else {
                return false;
            }
        }
    }

} // namespace glsld