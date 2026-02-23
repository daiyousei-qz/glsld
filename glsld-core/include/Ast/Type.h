#pragma once
#include "Ast/Base.h"
#include "Language/Typing.h"
#include "Support/StringView.h"

#include <algorithm>
#include <cstddef>
#include <optional>
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
        size_t dimSize;
    };

    struct StructTypeDesc
    {
        // The display name of the struct type.
        std::string name;

        // The linkage name of the struct type.
        std::string linkageName;

        // The AST node of the (first) declaration of this type.
        const AstDecl* decl;

        struct StructMemberDesc
        {
            size_t index;
            std::string name;
            const Type* type;
            const AstDecl* decl;
        };

        std::vector<StructMemberDesc> members;

        auto FindMember(StringView name) const -> const StructMemberDesc*
        {
            if (auto it = std::ranges::find(members, name, &StructMemberDesc::name); it != members.end()) {
                return &*it;
            }

            return nullptr;
        }
    };

    struct ValueDimension
    {
        int dimRow;
        int dimCol;

        constexpr auto operator==(const ValueDimension& rhs) const noexcept -> bool = default;
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
        // A compiler-generated name that uniquely identifies the type.
        std::string canonicalName;

        // Note this doesn't identify the type uniquely. The same canonical type may have different print names.
        std::string printName;

        // The actual type descriptor.
        DescPayloadType typeDesc;

        bool containsOpaqueType = false;

    public:
        Type(std::string printName, DescPayloadType typeDesc);

        // Get a globally unique type instance for error type
        static auto GetErrorType() -> const Type*;

        // Get a globally unique type instance for the particular builtin type
        static auto GetBuiltinType(GlslBuiltinType tag) -> const Type*;

        // Get a globally unique type instance for the particular scalar type, if any
        static auto GetScalarType(ScalarKind kind) -> const Type*;

        // Get a globally unique type instance for the particular vector type, if any
        // NOTE if the vector type doesn't exist in glsl language, returns ErrorType
        static auto GetVectorType(ScalarKind kind, size_t dim) -> const Type*;

        // Get a globally unique type instance for the particular matrix type, if any
        // NOTE if the matrix type doesn't exist in glsl language, returns ErrorType
        static auto GetMatrixType(ScalarKind kind, size_t dimRow, size_t dimCol) -> const Type*;

        // Get a globally unique type instance for the particular arithmetic type, if any
        // This includes scalar, vector and matrix types.
        // NOTE if the arithmetic type doesn't exist in glsl language, returns ErrorType
        static auto GetArithmeticType(ScalarKind kind, ValueDimension dim) -> const Type*;

        auto IsError() const noexcept -> bool
        {
            return std::holds_alternative<ErrorTypeDesc>(typeDesc);
        }
        auto IsVoid() const noexcept -> bool
        {
            return std::holds_alternative<VoidTypeDesc>(typeDesc);
        }
        auto IsScalar() const noexcept -> bool
        {
            return std::holds_alternative<ScalarTypeDesc>(typeDesc);
        }
        auto IsVector() const noexcept -> bool
        {
            return std::holds_alternative<VectorTypeDesc>(typeDesc);
        }
        auto IsMatrix() const noexcept -> bool
        {
            return std::holds_alternative<MatrixTypeDesc>(typeDesc);
        }
        auto IsSampler() const noexcept -> bool
        {
            return std::holds_alternative<SamplerTypeDesc>(typeDesc);
        }
        auto IsArray() const noexcept -> bool
        {
            return std::holds_alternative<ArrayTypeDesc>(typeDesc);
        }
        auto IsStruct() const noexcept -> bool
        {
            return std::holds_alternative<StructTypeDesc>(typeDesc);
        }

        auto IsScalarBool() const noexcept -> bool
        {
            return IsScalar() && std::get<ScalarTypeDesc>(typeDesc).type == ScalarKind::Bool;
        }
        auto IsScalarInt() const noexcept -> bool
        {
            return IsScalar() && std::get<ScalarTypeDesc>(typeDesc).type == ScalarKind::Int;
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

        // Returns the underlying scalar type of a scalar, vector or matrix type. Returns nullopt otherwise.
        auto GetElementScalarKind() const noexcept -> std::optional<ScalarKind>
        {
            if (auto desc = GetScalarDesc()) {
                return desc->type;
            }
            else if (auto desc = GetVectorDesc()) {
                return desc->scalarType;
            }
            else if (auto desc = GetMatrixDesc()) {
                return desc->scalarType;
            }
            else {
                return std::nullopt;
            }
        }

        // Returns the number of underlying scalars of a scalar, vector or matrix type. Returns nullopt otherwise.
        auto GetElementScalarCount() const noexcept -> std::optional<size_t>
        {
            if (auto desc = GetScalarDesc()) {
                return 1;
            }
            else if (auto desc = GetVectorDesc()) {
                return desc->vectorSize;
            }
            else if (auto desc = GetMatrixDesc()) {
                return desc->dimRow * desc->dimCol;
            }
            else {
                return std::nullopt;
            }
        }

        auto GetComponentType(size_t index) const noexcept -> const Type*
        {
            if (auto vectorDesc = GetVectorDesc(); vectorDesc) {
                if (index < vectorDesc->vectorSize) {
                    return GetScalarType(vectorDesc->scalarType);
                }
            }
            else if (auto matrixDesc = GetMatrixDesc(); matrixDesc) {
                if (index < matrixDesc->dimRow) {
                    return GetVectorType(matrixDesc->scalarType, matrixDesc->dimCol);
                }
            }
            else if (auto arrayDesc = GetArrayDesc(); arrayDesc) {
                if (arrayDesc->dimSize == 0 || index < arrayDesc->dimSize) {
                    return arrayDesc->elementType;
                }
            }
            else if (auto structDesc = GetStructDesc(); structDesc) {
                if (index < structDesc->members.size()) {
                    return structDesc->members[index].type;
                }
            }

            return GetErrorType();
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

        auto GetCanonicalName() const noexcept -> StringView
        {
            return canonicalName;
        }

        auto GetDebugName() const noexcept -> StringView
        {
            return !printName.empty() ? printName : "<unnamed>";
        }

        auto GetScalarDesc() const noexcept -> const ScalarTypeDesc*
        {
            return std::get_if<ScalarTypeDesc>(&typeDesc);
        }
        auto GetVectorDesc() const noexcept -> const VectorTypeDesc*
        {
            return std::get_if<VectorTypeDesc>(&typeDesc);
        }
        auto GetMatrixDesc() const noexcept -> const MatrixTypeDesc*
        {
            return std::get_if<MatrixTypeDesc>(&typeDesc);
        }
        auto GetSamplerDesc() const noexcept -> const SamplerTypeDesc*
        {
            return std::get_if<SamplerTypeDesc>(&typeDesc);
        }
        auto GetArrayDesc() const noexcept -> const ArrayTypeDesc*
        {
            return std::get_if<ArrayTypeDesc>(&typeDesc);
        }
        auto GetStructDesc() const noexcept -> const StructTypeDesc*
        {
            return std::get_if<StructTypeDesc>(&typeDesc);
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