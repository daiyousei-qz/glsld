#pragma once
#include "Basic/Common.h"
#include "Basic/StringView.h"
#include "Basic/StringMap.h"
#include "Ast/Base.h"
#include "Language/Typing.h"

#include <variant>
#include <vector>

namespace glsld
{
    class Type;

    // Bottom type
    struct ErrorTypeDesc
    {
    };

    // Void type
    struct VoidTypeDesc
    {
    };

    struct ScalarTypeDesc
    {
        ScalarKind type;
    };

    // Note this is the column vector
    struct VectorTypeDesc
    {
        ScalarKind scalarType;
        int vectorSize;
    };
    struct MatrixTypeDesc
    {
        // Scalar type of the matrix.
        ScalarKind scalarType;

        // Dimension of the row vector, i.e. the number of columns.
        int dimRow;

        // Dimension of the columns vector, i.e. the number of rows.
        int dimCol;
    };

    enum class SamplingDim
    {
        One,
        Two,
        Three,
        Rect,
        Cube,
        Buffer,
    };
    struct SamplerTypeDesc
    {
        SamplingDim dim;
    };

    struct ArrayTypeDesc
    {
        // Note this should not be an array type.
        const Type* elementType;

        // Note dim size of 0 means unsized/error-sized dimension
        std::vector<size_t> dimSizes;
    };

    struct StructTypeDesc
    {
        // The linkage name of the struct type.
        std::string name;

        std::vector<std::pair<std::string, const Type*>> members;

        // The AST node of the (first) declaration of this type.
        const AstDecl* decl;

        UnorderedStringMap<DeclView> memberDeclLookup;
    };

    struct ValueDimension
    {
        int dimRow;
        int dimCol;

        constexpr auto operator==(const ValueDimension& rhs) const noexcept -> bool
        {
            return dimRow == rhs.dimRow && dimCol == rhs.dimCol;
        }
    };

    // Type is a wrapper of type descriptor
    // For types except arrays and structs, we have globally unique type instances
    // For array and struct typs, the type instances are managed by AstContext
    class Type
    {
    public:
        // Do we need function type?
        using DescPayloadType = std::variant<ErrorTypeDesc, VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc,
                                             MatrixTypeDesc, SamplerTypeDesc, ArrayTypeDesc, StructTypeDesc>;

    private:
        // FIXME: is this for debug only?
        std::string debugName = "__FIXME";

        DescPayloadType descPayload;

        bool containsOpaqueType = false;

    public:
        Type(std::string name, DescPayloadType payload) : debugName(std::move(name)), descPayload(std::move(payload))
        {
            if (IsSampler()) {
                containsOpaqueType = true;
            }
            else if (auto desc = GetArrayDesc()) {
                containsOpaqueType = desc->elementType->containsOpaqueType;
            }
            else if (auto desc = GetStructDesc()) {
                for (const auto& [_, type] : desc->members) {
                    if (type->containsOpaqueType) {
                        containsOpaqueType = true;
                        break;
                    }
                }
            }
        }

        // Get a globally unique type instance for error type
        static auto GetErrorType() -> const Type*;

        // Get a globally unique type instance for the particular builtin type
        static auto GetBuiltinType(GlslBuiltinType tag) -> const Type*;

        // Get a globally unique type instance for the particular scalar type, if any
        // NOTE if the scalar type doesn't exist in glsl language, returns nullptr
        static auto GetScalarType(ScalarKind kind) -> const Type*;

        // Get a globally unique type instance for the particular vector type, if any
        // NOTE if the vector type doesn't exist in glsl language, returns nullptr
        static auto GetVectorType(ScalarKind kind, size_t dim) -> const Type*;

        static auto GetMatrixType(ScalarKind kind, size_t dimRow, size_t dimCol) -> const Type*;

        auto IsError() const noexcept -> bool
        {
            return std::holds_alternative<ErrorTypeDesc>(descPayload);
        }
        auto IsVoid() const noexcept -> bool
        {
            return std::holds_alternative<VoidTypeDesc>(descPayload);
        }
        auto IsScalar() const noexcept -> bool
        {
            return std::holds_alternative<ScalarTypeDesc>(descPayload);
        }
        auto IsVector() const noexcept -> bool
        {
            return std::holds_alternative<VectorTypeDesc>(descPayload);
        }
        auto IsMatrix() const noexcept -> bool
        {
            return std::holds_alternative<MatrixTypeDesc>(descPayload);
        }
        auto IsSampler() const noexcept -> bool
        {
            return std::holds_alternative<SamplerTypeDesc>(descPayload);
        }
        auto IsArray() const noexcept -> bool
        {
            return std::holds_alternative<ArrayTypeDesc>(descPayload);
        }
        auto IsStruct() const noexcept -> bool
        {
            return std::holds_alternative<StructTypeDesc>(descPayload);
        }

        auto IsScalarBool() const noexcept -> bool
        {
            return this == Type::GetBuiltinType(GlslBuiltinType::Ty_bool);
        }

        auto IsOpaque() const noexcept -> bool
        {
            // Note this is pre-computed to avoid recursion during lookup.
            return containsOpaqueType;
        }

        auto IsComposite() const noexcept -> bool
        {
            return IsArray() || IsStruct();
        }
        auto IsBuiltin() const noexcept -> bool
        {
            return !IsError() && !IsComposite();
        }

        // Arithmetic types support per-component arithmetic operations, including scalar, vector and matrix types.
        auto IsArithmetic() const noexcept -> bool
        {
            return IsScalar() || IsVector() || IsMatrix();
        }

        // Integral types support bitwise operations, including scalar and vector types.
        auto IsIntegral() const noexcept -> bool
        {
            if (auto desc = GetScalarDesc()) {
                return IsIntegralScalarType(desc->type);
            }
            else if (auto desc = GetVectorDesc()) {
                return IsIntegralScalarType(desc->scalarType);
            }

            return false;
        }

        auto GetElementScalarType() const noexcept -> const Type*
        {
            if (auto desc = GetScalarDesc()) {
                return GetScalarType(desc->type);
            }
            else if (auto desc = GetVectorDesc()) {
                return GetScalarType(desc->scalarType);
            }
            else if (auto desc = GetMatrixDesc()) {
                return GetScalarType(desc->scalarType);
            }
            else {
                return nullptr;
            }
        }

        auto GetDimension() const noexcept -> ValueDimension
        {
            if (auto desc = GetVectorDesc()) {
                return {1, desc->vectorSize};
            }
            else if (auto desc = GetMatrixDesc()) {
                return {desc->dimRow, desc->dimCol};
            }
            else {
                return {1, 1};
            }
        }

        auto GetDebugName() const noexcept -> StringView
        {
            return debugName;
        }

        auto GetScalarDesc() const noexcept -> const ScalarTypeDesc*
        {
            return std::get_if<ScalarTypeDesc>(&descPayload);
        }
        auto GetVectorDesc() const noexcept -> const VectorTypeDesc*
        {
            return std::get_if<VectorTypeDesc>(&descPayload);
        }
        auto GetMatrixDesc() const noexcept -> const MatrixTypeDesc*
        {
            return std::get_if<MatrixTypeDesc>(&descPayload);
        }
        auto GetSamplerDesc() const noexcept -> const SamplerTypeDesc*
        {
            return std::get_if<SamplerTypeDesc>(&descPayload);
        }
        auto GetArrayDesc() const noexcept -> const ArrayTypeDesc*
        {
            return std::get_if<ArrayTypeDesc>(&descPayload);
        }
        auto GetStructDesc() const noexcept -> const StructTypeDesc*
        {
            return std::get_if<StructTypeDesc>(&descPayload);
        }

        // Returns true if this type is the same with the given builtin type.
        auto IsSameWith(GlslBuiltinType type) const -> bool;

        // Returns true if this type is the same with the given type.
        auto IsSameWith(const Type* other) const -> bool;

        // True if this type is implicitly convertible to the given type.
        auto IsConvertibleTo(const Type* to) const -> bool;

        // True if this type could be tested for equality with the given type.
        auto IsEqualComparableTo(const Type* to) const -> bool
        {
            return !IsOpaque() && !to->IsOpaque() && (IsConvertibleTo(to) || to->IsConvertibleTo(this));
        }

        // True if this type could be ordered with the given type.
        auto IsOrderingComparableTo(const Type* to) const -> bool
        {
            return !IsScalar() && !to->IsScalar() && (IsConvertibleTo(to) || to->IsConvertibleTo(this));
        }

        // FIXME: move this out of Type. We have to handle comparing (A->B) and (C->D) case.
        // True if conversion from this type to `lhsTo` is better than that to `rhsTo`.
        auto HasBetterConversion(const Type* lhsTo, const Type* rhsTo) const -> bool;
    };
} // namespace glsld