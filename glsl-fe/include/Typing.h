#pragma once
#include "SyntaxToken.h"
#include <optional>
#include <variant>

namespace glsld
{
    // bottom type
    struct ErrorTypeDesc
    {
    };

    struct VoidTypeDesc
    {
    };

    struct ScalarTypeDesc
    {
        int size;
    };
    struct VectorTypeDesc
    {
        ScalarTypeDesc scalar;
        int vectorSize;
    };
    struct Matrix2DTypeDesc
    {
        ScalarTypeDesc scalar;
        int dimRow;
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
    struct StructTypeDesc
    {
    };

    class TypeDesc
    {
    public:
        auto IsError() -> bool;
        auto IsBuiltin() -> bool;
        auto IsVoid() -> bool;
        auto IsScalar() -> bool;
        auto IsVector() -> bool;
        auto IsMatrix() -> bool;
        auto IsSampler() -> bool;
        auto IsStruct() -> bool;

        auto GetScalarDesc() -> const ScalarTypeDesc*;

    private:
        std::variant<ErrorTypeDesc, VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc, Matrix2DTypeDesc, SamplerTypeDesc,
                     StructTypeDesc>
            typeDesc;
    };

#define DEFINE_VECTOR_TYPE(KW, SCALAR_KW, SIZE)

    DEFINE_VECTOR_TYPE(vec2, float, 2)
    DEFINE_VECTOR_TYPE(vec3, float, 3)
    DEFINE_VECTOR_TYPE(vec4, float, 4)

    enum class BuiltinType
    {
        // Void
        Ty_void,

        // Scalar
        Ty_bool,
        Ty_float,
        Ty_int,
        Ty_uint,
        Ty_double,

        // Misc
        Ty_atomic_uint,

        // Vector
        Ty_bvec2,
        Ty_bvec3,
        Ty_bvec4,
        Ty_ivec2,
        Ty_ivec3,
        Ty_ivec4,
        Ty_uvec2,
        Ty_uvec3,
        Ty_uvec4,
        Ty_vec2,
        Ty_vec3,
        Ty_vec4,
        Ty_dvec2,
        Ty_dvec3,
        Ty_dvec4,

        // Matrix
        Ty_mat2,
        Ty_mat3,
        Ty_mat4,
        Ty_mat2x2,
        Ty_mat2x3,
        Ty_mat2x4,
        Ty_mat3x2,
        Ty_mat3x3,
        Ty_mat3x4,
        Ty_mat4x2,
        Ty_mat4x3,
        Ty_mat4x4,
        Ty_dmat2,
        Ty_dmat3,
        Ty_dmat4,
        Ty_dmat2x2,
        Ty_dmat2x3,
        Ty_dmat2x4,
        Ty_dmat3x2,
        Ty_dmat3x3,
        Ty_dmat3x4,
        Ty_dmat4x2,
        Ty_dmat4x3,
        Ty_dmat4x4,

        // Combined Sampler
        Ty_sampler1D,
        Ty_sampler1DShoadow,
        Ty_sampler1DArray,
        Ty_sampler1DArrayShadow,
        Ty_isampler1D,
        Ty_isampler1DArray,
        Ty_usampler1D,
        Ty_usampler1DArray,
        Ty_sampler2D,
        Ty_sampler2DShoadow,
        Ty_sampler2DArray,
        Ty_sampler2DArrayShadow,
        Ty_isampler2D,
        Ty_isampler2DArray,
        Ty_usampler2D,
        Ty_usampler2DArray,
        Ty_sampler2DRect,
        Ty_sampler2DRectShadow,
        Ty_isampler2DRect,
        Ty_usampler2DRect,
        Ty_sampler2DMS,
        Ty_isampler2DMS,
        Ty_usampler2DMS,
        Ty_sampler2DMSArray,
        Ty_isampler2DMSArray,
        Ty_usampler2DMSArray,
        Ty_sampler3D,
        Ty_isampler3D,
        Ty_usampler3D,
        Ty_samplerCube,
        Ty_samplerCubeShadow,
        Ty_isamplerCube,
        Ty_usamplerCube,
        Ty_samplerCubeArray,
        Ty_samplerCubeArrayShadow,
        Ty_isamplerCubeArray,
        Ty_usamplerCubeArray,
        Ty_samplerBuffer,
        Ty_isamplerBuffer,
        Ty_usamplerBuffer,

        // Image
        Ty_image1D,
        Ty_image1DArray,
        Ty_iimage1D,
        Ty_iimage1DArray,
        Ty_uimage1D,
        Ty_uimage1DArray,
        Ty_image2D,
        Ty_image2DArray,
        Ty_iimage2D,
        Ty_iimage2DArray,
        Ty_uimage2D,
        Ty_uimage2DArray,
        Ty_image2DRect,
        Ty_iimage2DRect,
        Ty_uimage2DRect,
        Ty_image2DMS,
        Ty_iimage2DMS,
        Ty_uimage2DMS,
        Ty_image2DMSArray,
        Ty_iimage2DMSArray,
        Ty_uimage2DMSArray,
        Ty_image3D,
        Ty_iimage3D,
        Ty_uimage3D,
        Ty_imageCube,
        Ty_iimageCube,
        Ty_uimageCube,
        Ty_imageCubeArray,
        Ty_iimageCubeArray,
        Ty_uimageCubeArray,
        Ty_imageBuffer,
        Ty_iimageBuffer,
        Ty_uimageBuffer,

        // Separate Sampler (Vulkan-only)
        Ty_sampler,
        Ty_samplerShadow,

        // Separate Texture (Vulkan-only)
        Ty_texture1D,
        Ty_texture1DArray,
        Ty_itexture1D,
        Ty_itexture1DArray,
        Ty_utexture1D,
        Ty_utexture1DArray,
        Ty_texture2D,
        Ty_texture2DArray,
        Ty_itexture2D,
        Ty_itexture2DArray,
        Ty_utexture2D,
        Ty_utexture2DArray,
        Ty_texture2DRect,
        Ty_itexture2DRect,
        Ty_utexture2DRect,
        Ty_texture2DMS,
        Ty_itexture2DMS,
        Ty_utexture2DMS,
        Ty_texture2DMSArray,
        Ty_itexture2DMSArray,
        Ty_utexture2DMSArray,
        Ty_texture3D,
        Ty_itexture3D,
        Ty_utexture3D,
        Ty_textureCube,
        Ty_itextureCube,
        Ty_utextureCube,
        Ty_textureCubeArray,
        Ty_itextureCubeArray,
        Ty_utextureCubeArray,
        Ty_textureBuffer,
        Ty_itextureBuffer,
        Ty_utextureBuffer,

        // Subpass (Vulkan-only)
        Ty_subpassInput,
        Ty_isubpassInput,
        Ty_usubpassInput,
        Ty_subpassInputMS,
        Ty_isubpassInputMS,
        Ty_usubpassInputMS,
    };

    inline auto GetBuiltinType(const SyntaxToken& tok) -> std::optional<BuiltinType>
    {
#define DEFINE_CASE(KEYWORD)                                                                                           \
    case TokenKlass::K_##KEYWORD:                                                                                      \
        return BuiltinType::Ty_##KEYWORD;

        switch (tok.klass) {
            DEFINE_CASE(void)
            DEFINE_CASE(bool)
            DEFINE_CASE(float)
            DEFINE_CASE(int)
            DEFINE_CASE(uint)
            DEFINE_CASE(double)
            DEFINE_CASE(atomic_uint)
            DEFINE_CASE(bvec2)
            DEFINE_CASE(bvec3)
            DEFINE_CASE(bvec4)
            DEFINE_CASE(ivec2)
            DEFINE_CASE(ivec3)
            DEFINE_CASE(ivec4)
            DEFINE_CASE(uvec2)
            DEFINE_CASE(uvec3)
            DEFINE_CASE(uvec4)
            DEFINE_CASE(vec2)
            DEFINE_CASE(vec3)
            DEFINE_CASE(vec4)
            DEFINE_CASE(dvec2)
            DEFINE_CASE(dvec3)
            DEFINE_CASE(dvec4)
            DEFINE_CASE(mat2)
            DEFINE_CASE(mat3)
            DEFINE_CASE(mat4)
            DEFINE_CASE(mat2x2)
            DEFINE_CASE(mat2x3)
            DEFINE_CASE(mat2x4)
            DEFINE_CASE(mat3x2)
            DEFINE_CASE(mat3x3)
            DEFINE_CASE(mat3x4)
            DEFINE_CASE(mat4x2)
            DEFINE_CASE(mat4x3)
            DEFINE_CASE(mat4x4)
            DEFINE_CASE(dmat2)
            DEFINE_CASE(dmat3)
            DEFINE_CASE(dmat4)
            DEFINE_CASE(dmat2x2)
            DEFINE_CASE(dmat2x3)
            DEFINE_CASE(dmat2x4)
            DEFINE_CASE(dmat3x2)
            DEFINE_CASE(dmat3x3)
            DEFINE_CASE(dmat3x4)
            DEFINE_CASE(dmat4x2)
            DEFINE_CASE(dmat4x3)
            DEFINE_CASE(dmat4x4)
            DEFINE_CASE(sampler1D)
            DEFINE_CASE(sampler1DShoadow)
            DEFINE_CASE(sampler1DArray)
            DEFINE_CASE(sampler1DArrayShadow)
            DEFINE_CASE(isampler1D)
            DEFINE_CASE(isampler1DArray)
            DEFINE_CASE(usampler1D)
            DEFINE_CASE(usampler1DArray)
            DEFINE_CASE(sampler2D)
            DEFINE_CASE(sampler2DShoadow)
            DEFINE_CASE(sampler2DArray)
            DEFINE_CASE(sampler2DArrayShadow)
            DEFINE_CASE(isampler2D)
            DEFINE_CASE(isampler2DArray)
            DEFINE_CASE(usampler2D)
            DEFINE_CASE(usampler2DArray)
            DEFINE_CASE(sampler2DRect)
            DEFINE_CASE(sampler2DRectShadow)
            DEFINE_CASE(isampler2DRect)
            DEFINE_CASE(usampler2DRect)
            DEFINE_CASE(sampler2DMS)
            DEFINE_CASE(isampler2DMS)
            DEFINE_CASE(usampler2DMS)
            DEFINE_CASE(sampler2DMSArray)
            DEFINE_CASE(isampler2DMSArray)
            DEFINE_CASE(usampler2DMSArray)
            DEFINE_CASE(sampler3D)
            DEFINE_CASE(isampler3D)
            DEFINE_CASE(usampler3D)
            DEFINE_CASE(samplerCube)
            DEFINE_CASE(samplerCubeShadow)
            DEFINE_CASE(isamplerCube)
            DEFINE_CASE(usamplerCube)
            DEFINE_CASE(samplerCubeArray)
            DEFINE_CASE(samplerCubeArrayShadow)
            DEFINE_CASE(isamplerCubeArray)
            DEFINE_CASE(usamplerCubeArray)
            DEFINE_CASE(samplerBuffer)
            DEFINE_CASE(isamplerBuffer)
            DEFINE_CASE(usamplerBuffer)
            DEFINE_CASE(image1D)
            DEFINE_CASE(image1DArray)
            DEFINE_CASE(iimage1D)
            DEFINE_CASE(iimage1DArray)
            DEFINE_CASE(uimage1D)
            DEFINE_CASE(uimage1DArray)
            DEFINE_CASE(image2D)
            DEFINE_CASE(image2DArray)
            DEFINE_CASE(iimage2D)
            DEFINE_CASE(iimage2DArray)
            DEFINE_CASE(uimage2D)
            DEFINE_CASE(uimage2DArray)
            DEFINE_CASE(image2DRect)
            DEFINE_CASE(iimage2DRect)
            DEFINE_CASE(uimage2DRect)
            DEFINE_CASE(image2DMS)
            DEFINE_CASE(iimage2DMS)
            DEFINE_CASE(uimage2DMS)
            DEFINE_CASE(image2DMSArray)
            DEFINE_CASE(iimage2DMSArray)
            DEFINE_CASE(uimage2DMSArray)
            DEFINE_CASE(image3D)
            DEFINE_CASE(iimage3D)
            DEFINE_CASE(uimage3D)
            DEFINE_CASE(imageCube)
            DEFINE_CASE(iimageCube)
            DEFINE_CASE(uimageCube)
            DEFINE_CASE(imageCubeArray)
            DEFINE_CASE(iimageCubeArray)
            DEFINE_CASE(uimageCubeArray)
            DEFINE_CASE(imageBuffer)
            DEFINE_CASE(iimageBuffer)
            DEFINE_CASE(uimageBuffer)
            // DEFINE_CASE(sampler)
            // DEFINE_CASE(samplerShadow)
            DEFINE_CASE(texture1D)
            DEFINE_CASE(texture1DArray)
            DEFINE_CASE(itexture1D)
            DEFINE_CASE(itexture1DArray)
            DEFINE_CASE(utexture1D)
            DEFINE_CASE(utexture1DArray)
            DEFINE_CASE(texture2D)
            DEFINE_CASE(texture2DArray)
            DEFINE_CASE(itexture2D)
            DEFINE_CASE(itexture2DArray)
            DEFINE_CASE(utexture2D)
            DEFINE_CASE(utexture2DArray)
            DEFINE_CASE(texture2DRect)
            DEFINE_CASE(itexture2DRect)
            DEFINE_CASE(utexture2DRect)
            DEFINE_CASE(texture2DMS)
            DEFINE_CASE(itexture2DMS)
            DEFINE_CASE(utexture2DMS)
            DEFINE_CASE(texture2DMSArray)
            DEFINE_CASE(itexture2DMSArray)
            DEFINE_CASE(utexture2DMSArray)
            DEFINE_CASE(texture3D)
            DEFINE_CASE(itexture3D)
            DEFINE_CASE(utexture3D)
            DEFINE_CASE(textureCube)
            DEFINE_CASE(itextureCube)
            DEFINE_CASE(utextureCube)
            DEFINE_CASE(textureCubeArray)
            DEFINE_CASE(itextureCubeArray)
            DEFINE_CASE(utextureCubeArray)
            DEFINE_CASE(textureBuffer)
            DEFINE_CASE(itextureBuffer)
            DEFINE_CASE(utextureBuffer)
            DEFINE_CASE(subpassInput)
            DEFINE_CASE(isubpassInput)
            DEFINE_CASE(usubpassInput)
            DEFINE_CASE(subpassInputMS)
            DEFINE_CASE(isubpassInputMS)
            DEFINE_CASE(usubpassInputMS)

        default:
            return std::nullopt;
        }
#undef DEFINE_CASE
    }
} // namespace glsld
