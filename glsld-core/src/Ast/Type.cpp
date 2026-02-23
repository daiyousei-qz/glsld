#include "Ast/Type.h"

namespace glsld
{
    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const ErrorTypeDesc& desc) -> void
    {
        fmt::format_to(std::back_inserter(buf), "e");
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const VoidTypeDesc& desc) -> void
    {
        fmt::format_to(std::back_inserter(buf), "v");
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const ScalarTypeDesc& desc) -> void
    {
        switch (desc.type) {
        case ScalarKind::Bool:
            fmt::format_to(std::back_inserter(buf), "b");
            break;
        case ScalarKind::Int:
            fmt::format_to(std::back_inserter(buf), "i");
            break;
        case ScalarKind::Uint:
            fmt::format_to(std::back_inserter(buf), "u");
            break;
        case ScalarKind::Float:
            fmt::format_to(std::back_inserter(buf), "f");
            break;
        case ScalarKind::Double:
            fmt::format_to(std::back_inserter(buf), "d");
            break;
        case ScalarKind::Int8:
            fmt::format_to(std::back_inserter(buf), "i8");
            break;
        case ScalarKind::Int16:
            fmt::format_to(std::back_inserter(buf), "i16");
            break;
        case ScalarKind::Int64:
            fmt::format_to(std::back_inserter(buf), "i64");
            break;
        case ScalarKind::Uint8:
            fmt::format_to(std::back_inserter(buf), "u8");
            break;
        case ScalarKind::Uint16:
            fmt::format_to(std::back_inserter(buf), "u16");
            break;
        case ScalarKind::Uint64:
            fmt::format_to(std::back_inserter(buf), "u64");
            break;
        case ScalarKind::Float16:
            fmt::format_to(std::back_inserter(buf), "f16");
            break;
        }
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const VectorTypeDesc& desc) -> void
    {
        MangleTypeDescInto(buf, ScalarTypeDesc{desc.scalarType});
        fmt::format_to(std::back_inserter(buf), "v{}", desc.vectorSize);
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const MatrixTypeDesc& desc) -> void
    {
        MangleTypeDescInto(buf, ScalarTypeDesc{desc.scalarType});
        fmt::format_to(std::back_inserter(buf), "m{}x{}", desc.dimRow, desc.dimCol);
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const SamplerTypeDesc& desc) -> void
    {
        switch (desc.dim) {
        case SamplingDim::One:
            fmt::format_to(std::back_inserter(buf), "s1");
            break;
        case SamplingDim::Two:
            fmt::format_to(std::back_inserter(buf), "s2");
            break;
        case SamplingDim::Three:
            fmt::format_to(std::back_inserter(buf), "s3");
            break;
        case SamplingDim::Rect:
            fmt::format_to(std::back_inserter(buf), "srect");
            break;
        case SamplingDim::Cube:
            fmt::format_to(std::back_inserter(buf), "scube");
            break;
        case SamplingDim::Buffer:
            fmt::format_to(std::back_inserter(buf), "sbuffer");
            break;
        }
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const ArrayTypeDesc& desc) -> void
    {
        fmt::format_to(std::back_inserter(buf), "{}[{}]", desc.elementType->GetCanonicalName(), desc.dimSize);
    }

    static auto MangleTypeDescInto(fmt::memory_buffer& buf, const StructTypeDesc& desc) -> void
    {
        // Note struct type is nominal. The context should ensure that different struct types have different linkage
        // names.
        fmt::format_to(std::back_inserter(buf), "#{}", desc.linkageName);
    }

    Type::Type(std::string printName, DescPayloadType typeDesc)
        : printName(std::move(printName)), typeDesc(std::move(typeDesc))
    {
        canonicalName = std::visit(
            [](const auto& desc) {
                fmt::memory_buffer buf;
                MangleTypeDescInto(buf, desc);
                return std::string(buf.data(), buf.size());
            },
            typeDesc);

        if (IsSampler()) {
            containsOpaqueType = true;
        }
        else if (auto desc = GetArrayDesc()) {
            containsOpaqueType = desc->elementType->containsOpaqueType;
        }
        else if (auto desc = GetStructDesc()) {
            containsOpaqueType =
                std::ranges::any_of(desc->members, [](const StructTypeDesc::StructMemberDesc& memberDesc) {
                    return memberDesc.type->containsOpaqueType;
                });
        }
    }

    auto Type::GetErrorType() -> const Type*
    {
        static Type errorType{"__ErrorType", ErrorTypeDesc{}};
        return &errorType;
    }

    auto Type::GetBuiltinType(GlslBuiltinType tag) -> const Type*
    {
        switch (tag) {
#define DECL_BUILTIN_TYPE(GLSL_TYPE, DESC_PAYLOAD_TYPE, ...)                                                           \
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
        case ScalarKind::Int8:
            return GetBuiltinType(GlslBuiltinType::Ty_int8_t);
        case ScalarKind::Int16:
            return GetBuiltinType(GlslBuiltinType::Ty_int16_t);
        case ScalarKind::Int64:
            return GetBuiltinType(GlslBuiltinType::Ty_int64_t);
        case ScalarKind::Uint8:
            return GetBuiltinType(GlslBuiltinType::Ty_uint8_t);
        case ScalarKind::Uint16:
            return GetBuiltinType(GlslBuiltinType::Ty_uint16_t);
        case ScalarKind::Uint64:
            return GetBuiltinType(GlslBuiltinType::Ty_uint64_t);
        case ScalarKind::Float16:
            return GetBuiltinType(GlslBuiltinType::Ty_float16_t);
        }

        GLSLD_UNREACHABLE();
    }

    auto Type::GetVectorType(ScalarKind type, size_t dim) -> const Type*
    {
        if (dim < 2 || dim > 4) {
            return Type::GetErrorType();
        }

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

            // FIXME: should we support vector of extended scalar types?
        default:
            break;
        }

        return Type::GetErrorType();
    }

    auto Type::GetMatrixType(ScalarKind type, size_t dimRow, size_t dimCol) -> const Type*
    {
        if (dimRow < 2 || dimRow > 4 || dimCol < 2 || dimCol > 4) {
            return Type::GetErrorType();
        }

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

        // FIXME: should we support vector of extended scalar types?
        return Type::GetErrorType();
    }

    auto Type::GetArithmeticType(ScalarKind kind, ValueDimension dim) -> const Type*
    {
        if (dim.dimCol > 1 && dim.dimRow > 1) {
            return Type::GetMatrixType(kind, dim.dimRow, dim.dimCol);
        }
        else if (dim.dimCol > 1 && dim.dimRow == 1) {
            return Type::GetVectorType(kind, dim.dimCol);
        }
        else if (dim.dimCol == 1 && dim.dimRow == 1) {
            return Type::GetScalarType(kind);
        }

        return Type::GetErrorType();
    }

    auto Type::IsSameWith(GlslBuiltinType type) const -> bool
    {
        return IsSameWith(Type::GetBuiltinType(type));
    }

    auto Type::IsSameWith(const Type* other) const -> bool
    {
        GLSLD_ASSERT(other != nullptr);
        if (this == other) {
            // Fast path for the same type instance.
            return true;
        }
        else {
            return GetCanonicalName() == other->GetCanonicalName();
        }
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