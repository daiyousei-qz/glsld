#pragma once
#include "SyntaxToken.h"
#include <optional>
#include <variant>

namespace glsld
{
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
        using enum TokenKlass;
        using enum BuiltinType;

        switch (tok.klass) {
        // Void
        case K_void:
            return Ty_void;

        // Scalar
        case K_bool:
            return Ty_bool;
        case K_float:
            return Ty_float;
        case K_int:
            return Ty_int;
        case K_uint:
            return Ty_uint;
        case K_double:
            return Ty_double;

        // Misc
        case K_atomic_uint:
            return Ty_atomic_uint;

        // Vector
        case K_bvec2:
            return Ty_bvec2;
        case K_bvec3:
            return Ty_bvec3;
        case K_bvec4:
            return Ty_bvec4;
        case K_ivec2:
            return Ty_ivec2;
        case K_ivec3:
            return Ty_ivec3;
        case K_ivec4:
            return Ty_ivec4;
        case K_uvec2:
            return Ty_uvec2;
        case K_uvec3:
            return Ty_uvec3;
        case K_uvec4:
            return Ty_uvec4;
        case K_vec2:
            return Ty_vec2;
        case K_vec3:
            return Ty_vec3;
        case K_vec4:
            return Ty_vec4;
        case K_dvec2:
            return Ty_dvec2;
        case K_dvec3:
            return Ty_dvec3;
        case K_dvec4:
            return Ty_dvec4;

            // return Matrix;
        case K_mat2:
            return Ty_mat2;
        case K_mat3:
            return Ty_mat3;
        case K_mat4:
            return Ty_mat4;
        case K_mat2x2:
            return Ty_mat2x2;
        case K_mat2x3:
            return Ty_mat2x3;
        case K_mat2x4:
            return Ty_mat2x4;
        case K_mat3x2:
            return Ty_mat3x2;
        case K_mat3x3:
            return Ty_mat3x3;
        case K_mat3x4:
            return Ty_mat3x4;
        case K_mat4x2:
            return Ty_mat4x2;
        case K_mat4x3:
            return Ty_mat4x3;
        case K_mat4x4:
            return Ty_mat4x4;
        case K_dmat2:
            return Ty_dmat2;
        case K_dmat3:
            return Ty_dmat3;
        case K_dmat4:
            return Ty_dmat4;
        case K_dmat2x2:
            return Ty_dmat2x2;
        case K_dmat2x3:
            return Ty_dmat2x3;
        case K_dmat2x4:
            return Ty_dmat2x4;
        case K_dmat3x2:
            return Ty_dmat3x2;
        case K_dmat3x3:
            return Ty_dmat3x3;
        case K_dmat3x4:
            return Ty_dmat3x4;
        case K_dmat4x2:
            return Ty_dmat4x2;
        case K_dmat4x3:
            return Ty_dmat4x3;
        case K_dmat4x4:
            return Ty_dmat4x4;

        // Combined Sampler
        case K_sampler1D:
            return Ty_sampler1D;
        case K_sampler1DShoadow:
            return Ty_sampler1DShoadow;
        case K_sampler1DArray:
            return Ty_sampler1DArray;
        case K_sampler1DArrayShadow:
            return Ty_sampler1DArrayShadow;
        case K_isampler1D:
            return Ty_isampler1D;
        case K_isampler1DArray:
            return Ty_isampler1DArray;
        case K_usampler1D:
            return Ty_usampler1D;
        case K_usampler1DArray:
            return Ty_usampler1DArray;
        case K_sampler2D:
            return Ty_sampler2D;
        case K_sampler2DShoadow:
            return Ty_sampler2DShoadow;
        case K_sampler2DArray:
            return Ty_sampler2DArray;
        case K_sampler2DArrayShadow:
            return Ty_sampler2DArrayShadow;
        case K_isampler2D:
            return Ty_isampler2D;
        case K_isampler2DArray:
            return Ty_isampler2DArray;
        case K_usampler2D:
            return Ty_usampler2D;
        case K_usampler2DArray:
            return Ty_usampler2DArray;
        case K_sampler2DRect:
            return Ty_sampler2DRect;
        case K_sampler2DRectShadow:
            return Ty_sampler2DRectShadow;
        case K_isampler2DRect:
            return Ty_isampler2DRect;
        case K_usampler2DRect:
            return Ty_usampler2DRect;
        case K_sampler2DMS:
            return Ty_sampler2DMS;
        case K_isampler2DMS:
            return Ty_isampler2DMS;
        case K_usampler2DMS:
            return Ty_usampler2DMS;
        case K_sampler2DMSArray:
            return Ty_sampler2DMSArray;
        case K_isampler2DMSArray:
            return Ty_isampler2DMSArray;
        case K_usampler2DMSArray:
            return Ty_usampler2DMSArray;
        case K_sampler3D:
            return Ty_sampler3D;
        case K_isampler3D:
            return Ty_isampler3D;
        case K_usampler3D:
            return Ty_usampler3D;
        case K_samplerCube:
            return Ty_samplerCube;
        case K_samplerCubeShadow:
            return Ty_samplerCubeShadow;
        case K_isamplerCube:
            return Ty_isamplerCube;
        case K_usamplerCube:
            return Ty_usamplerCube;
        case K_samplerCubeArray:
            return Ty_samplerCubeArray;
        case K_samplerCubeArrayShadow:
            return Ty_samplerCubeArrayShadow;
        case K_isamplerCubeArray:
            return Ty_isamplerCubeArray;
        case K_usamplerCubeArray:
            return Ty_usamplerCubeArray;
        case K_samplerBuffer:
            return Ty_samplerBuffer;
        case K_isamplerBuffer:
            return Ty_isamplerBuffer;
        case K_usamplerBuffer:
            return Ty_usamplerBuffer;

            //  Image
        case K_image1D:
            return Ty_image1D;
        case K_image1DArray:
            return Ty_image1DArray;
        case K_iimage1D:
            return Ty_iimage1D;
        case K_iimage1DArray:
            return Ty_iimage1DArray;
        case K_uimage1D:
            return Ty_uimage1D;
        case K_uimage1DArray:
            return Ty_uimage1DArray;
        case K_image2D:
            return Ty_image2D;
        case K_image2DArray:
            return Ty_image2DArray;
        case K_iimage2D:
            return Ty_iimage2D;
        case K_iimage2DArray:
            return Ty_iimage2DArray;
        case K_uimage2D:
            return Ty_uimage2D;
        case K_uimage2DArray:
            return Ty_uimage2DArray;
        case K_image2DRect:
            return Ty_image2DRect;
        case K_iimage2DRect:
            return Ty_iimage2DRect;
        case K_uimage2DRect:
            return Ty_uimage2DRect;
        case K_image2DMS:
            return Ty_image2DMS;
        case K_iimage2DMS:
            return Ty_iimage2DMS;
        case K_uimage2DMS:
            return Ty_uimage2DMS;
        case K_image2DMSArray:
            return Ty_image2DMSArray;
        case K_iimage2DMSArray:
            return Ty_iimage2DMSArray;
        case K_uimage2DMSArray:
            return Ty_uimage2DMSArray;
        case K_image3D:
            return Ty_image3D;
        case K_iimage3D:
            return Ty_iimage3D;
        case K_uimage3D:
            return Ty_uimage3D;
        case K_imageCube:
            return Ty_imageCube;
        case K_iimageCube:
            return Ty_iimageCube;
        case K_uimageCube:
            return Ty_uimageCube;
        case K_imageCubeArray:
            return Ty_imageCubeArray;
        case K_iimageCubeArray:
            return Ty_iimageCubeArray;
        case K_uimageCubeArray:
            return Ty_uimageCubeArray;
        case K_imageBuffer:
            return Ty_imageBuffer;
        case K_iimageBuffer:
            return Ty_iimageBuffer;
        case K_uimageBuffer:
            return Ty_uimageBuffer;

        // Separate Sampler (Vulkan-only)
        // case K_sampler: return Ty_sampler;
        // case K_samplerShadow: return Ty_samplerShadow;

        //  Separate Texture (Vulkan-only)
        case K_texture1D:
            return Ty_texture1D;
        case K_texture1DArray:
            return Ty_texture1DArray;
        case K_itexture1D:
            return Ty_itexture1D;
        case K_itexture1DArray:
            return Ty_itexture1DArray;
        case K_utexture1D:
            return Ty_utexture1D;
        case K_utexture1DArray:
            return Ty_utexture1DArray;
        case K_texture2D:
            return Ty_texture2D;
        case K_texture2DArray:
            return Ty_texture2DArray;
        case K_itexture2D:
            return Ty_itexture2D;
        case K_itexture2DArray:
            return Ty_itexture2DArray;
        case K_utexture2D:
            return Ty_utexture2D;
        case K_utexture2DArray:
            return Ty_utexture2DArray;
        case K_texture2DRect:
            return Ty_texture2DRect;
        case K_itexture2DRect:
            return Ty_itexture2DRect;
        case K_utexture2DRect:
            return Ty_utexture2DRect;
        case K_texture2DMS:
            return Ty_texture2DMS;
        case K_itexture2DMS:
            return Ty_itexture2DMS;
        case K_utexture2DMS:
            return Ty_utexture2DMS;
        case K_texture2DMSArray:
            return Ty_texture2DMSArray;
        case K_itexture2DMSArray:
            return Ty_itexture2DMSArray;
        case K_utexture2DMSArray:
            return Ty_utexture2DMSArray;
        case K_texture3D:
            return Ty_texture3D;
        case K_itexture3D:
            return Ty_itexture3D;
        case K_utexture3D:
            return Ty_utexture3D;
        case K_textureCube:
            return Ty_textureCube;
        case K_itextureCube:
            return Ty_itextureCube;
        case K_utextureCube:
            return Ty_utextureCube;
        case K_textureCubeArray:
            return Ty_textureCubeArray;
        case K_itextureCubeArray:
            return Ty_itextureCubeArray;
        case K_utextureCubeArray:
            return Ty_utextureCubeArray;
        case K_textureBuffer:
            return Ty_textureBuffer;
        case K_itextureBuffer:
            return Ty_itextureBuffer;
        case K_utextureBuffer:
            return Ty_utextureBuffer;

            // Subpass (Vulkan-only)
        case K_subpassInput:
            return Ty_subpassInput;
        case K_isubpassInput:
            return Ty_isubpassInput;
        case K_usubpassInput:
            return Ty_usubpassInput;
        case K_subpassInputMS:
            return Ty_subpassInputMS;
        case K_isubpassInputMS:
            return Ty_isubpassInputMS;
        case K_usubpassInputMS:
            return Ty_usubpassInputMS;

        default:
            return std::nullopt;
        }
    }

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
        auto IsBuiltin() -> bool;
        auto IsVoid() -> bool;
        auto IsScalar() -> bool;
        auto IsVector() -> bool;
        auto IsMatrix() -> bool;
        auto IsSampler() -> bool;
        auto IsStruct() -> bool;

        auto GetScalarDesc() -> const ScalarTypeDesc*;

    private:
        std::variant<VoidTypeDesc, ScalarTypeDesc, VectorTypeDesc, Matrix2DTypeDesc, SamplerTypeDesc, StructTypeDesc>
            typeDesc;
    };
} // namespace glsld
