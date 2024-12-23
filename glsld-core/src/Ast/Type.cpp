#include "Ast/Type.h"

namespace glsld
{
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
        if (IsSameWith(to)) {
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
        // Exact match is always better than conversion
        if (IsSameWith(lhsTo)) {
            return !IsSameWith(rhsTo);
        }
        else if (IsSameWith(rhsTo)) {
            return false;
        }

        // Error never has a better conversion to anything
        if (IsError()) {
            return false;
        }

        // Assuming this type is convertible to both `lhsTo` and `rhsTo`
        // FIXME: should we assume this?
        GLSLD_ASSERT(IsConvertibleTo(lhsTo) && IsConvertibleTo(rhsTo));

        if (auto scalarDesc = GetScalarDesc()) {
            return HasBetterConversionAux(scalarDesc->type, lhsTo->GetScalarDesc()->type, rhsTo->GetScalarDesc()->type);
        }
        else if (auto vectorDesc = GetVectorDesc()) {
            return HasBetterConversionAux(vectorDesc->scalarType, lhsTo->GetVectorDesc()->scalarType,
                                          rhsTo->GetVectorDesc()->scalarType);
        }
        else if (auto matrixDesc = GetMatrixDesc()) {
            return HasBetterConversionAux(matrixDesc->scalarType, lhsTo->GetMatrixDesc()->scalarType,
                                          rhsTo->GetMatrixDesc()->scalarType);
        }
        else {
            // FIXME: implement other type conversion
            return false;
        }
    }

} // namespace glsld