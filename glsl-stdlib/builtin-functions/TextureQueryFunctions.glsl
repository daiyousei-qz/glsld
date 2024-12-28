int textureSize(isampler1D sampler_,int lod);
int textureSize(sampler1D sampler_,int lod);
int textureSize(usampler1D sampler_,int lod);
ivec2 textureSize(sampler2D sampler_,int lod);
ivec2 textureSize(isampler2D sampler_,int lod);
ivec2 textureSize(usampler2D sampler_,int lod);
ivec3 textureSize(sampler3D sampler_,int lod);
ivec3 textureSize(isampler3D sampler_,int lod);
ivec3 textureSize(usampler3D sampler_,int lod);
ivec2 textureSize(samplerCube sampler_,int lod);
ivec2 textureSize(isamplerCube sampler_,int lod);
ivec2 textureSize(usamplerCube sampler_,int lod);
ivec2 textureSize(sampler2DRect sampler_);
ivec2 textureSize(isampler2DRect sampler_);
ivec2 textureSize(usampler2DRect sampler_);
int textureSize(samplerBuffer sampler_);
int textureSize(isamplerBuffer sampler_);
int textureSize(usamplerBuffer sampler_);
ivec2 textureSize(sampler1DArray sampler_,int lod);
ivec2 textureSize(isampler1DArray sampler_,int lod);
ivec2 textureSize(usampler1DArray sampler_,int lod);
ivec3 textureSize(sampler2DArray sampler_,int lod);
ivec3 textureSize(isampler2DArray sampler_,int lod);
ivec3 textureSize(usampler2DArray sampler_,int lod);
ivec3 textureSize(samplerCubeArray sampler_,int lod);
ivec3 textureSize(isamplerCubeArray sampler_,int lod);
ivec3 textureSize(usamplerCubeArray sampler_,int lod);
ivec2 textureSize(sampler2DMS sampler_);
ivec2 textureSize(isampler2DMS sampler_);
ivec2 textureSize(usampler2DMS sampler_);
ivec3 textureSize(sampler2DMSArray sampler_);
ivec3 textureSize(isampler2DMSArray sampler_);
ivec3 textureSize(usampler2DMSArray sampler_);
int textureSize(sampler1DShadow sampler_,int lod);
ivec2 textureSize(sampler2DShadow sampler_,int lod);
ivec2 textureSize(samplerCubeShadow sampler_,int lod);
ivec2 textureSize(sampler2DRectShadow sampler_); lod
ivec2 textureSize(sampler1DArrayShadow sampler_,int lod);
ivec3 textureSize(sampler2DArrayShadow sampler_,int lod);
ivec3 textureSize(samplerCubeArrayShadow sampler_,int lod);
vec2 textureQueryLod(sampler1D sampler_, float P);
vec2 textureQueryLod(isampler1D sampler_, float P);
vec2 textureQueryLod(usampler1D sampler_, float P);
vec2 textureQueryLod(sampler2D sampler_, vec2 P);
vec2 textureQueryLod(isampler2D sampler_, vec2 P);
vec2 textureQueryLod(usampler2D sampler_, vec2 P);
vec2 textureQueryLod(sampler3D sampler_, vec3 P);
vec2 textureQueryLod(isampler3D sampler_, vec3 P);
vec2 textureQueryLod(usampler3D sampler_, vec3 P);
vec2 textureQueryLod(samplerCube sampler_, vec3 P);
vec2 textureQueryLod(isamplerCube sampler_, vec3 P);
vec2 textureQueryLod(usamplerCube sampler_, vec3 P);
vec2 textureQueryLod(sampler1DArray sampler_, float P);
vec2 textureQueryLod(isampler1DArray sampler_, float P);
vec2 textureQueryLod(usampler1DArray sampler_, float P);
vec2 textureQueryLod(sampler2DArray sampler_, vec2 P);
vec2 textureQueryLod(isampler2DArray sampler_, vec2 P);
vec2 textureQueryLod(usampler2DArray sampler_, vec2 P);
vec2 textureQueryLod(samplerCubeArray sampler_, vec3 P);
vec2 textureQueryLod(isamplerCubeArray sampler_, vec3 P);
vec2 textureQueryLod(usamplerCubeArray sampler_, vec3 P);
vec2 textureQueryLod(sampler1DShadow sampler_, float P);
vec2 textureQueryLod(sampler2DShadow sampler_, vec2 P);
vec2 textureQueryLod(samplerCubeShadow sampler_, vec3 P);
vec2 textureQueryLod(sampler1DArrayShadow sampler_, float P);
vec2 textureQueryLod(sampler2DArrayShadow sampler_, vec2 P);
vec2 textureQueryLod(samplerCubeArrayShadow sampler_, vec3 P);
int textureQueryLevels(isampler1D sampler_);
int textureQueryLevels(sampler1D sampler_);
int textureQueryLevels(usampler1D sampler_);
int textureQueryLevels(sampler2D sampler_);
int textureQueryLevels(isampler2D sampler_);
int textureQueryLevels(usampler2D sampler_);
int textureQueryLevels(sampler3D sampler_);
int textureQueryLevels(isampler3D sampler_);
int textureQueryLevels(usampler3D sampler_);
int textureQueryLevels(samplerCube sampler_);
int textureQueryLevels(isamplerCube sampler_);
int textureQueryLevels(usamplerCube sampler_);
int textureQueryLevels(usampler1DArray sampler_);
int textureQueryLevels(sampler2DArray sampler_);
int textureQueryLevels(isampler2DArray sampler_);
int textureQueryLevels(usampler2DArray sampler_);
int textureQueryLevels(samplerCubeArray sampler_);
int textureQueryLevels(isamplerCubeArray sampler_);
int textureQueryLevels(usamplerCubeArray sampler_);
int textureQueryLevels(sampler2DShadow sampler_);
int textureQueryLevels(samplerCubeShadow sampler_);
int textureQueryLevels(sampler2DArrayShadow sampler_);
int textureQueryLevels(samplerCubeArrayShadow sampler_);
int textureQueryLevels(sampler1DArray sampler_);
int textureQueryLevels(isampler1DArray sampler_);
int textureQueryLevels(sampler1DShadow sampler_);
int textureQueryLevels(sampler1DArrayShadow sampler_);
int textureSamples(sampler2DMS sampler_);
int textureSamples(isampler2DMS sampler_);
int textureSamples(usampler2DMS sampler_);
int textureSamples(sampler2DMSArray sampler_);
int textureSamples(isampler2DMSArray sampler_);
int textureSamples(usampler2DMSArray sampler_);

// GL_ARB_texture_query_lod
// vec2 textureQueryLOD(sampler1D sampler_, float P);
// vec2 textureQueryLOD(isampler1D sampler_, float P);
// vec2 textureQueryLOD(usampler1D sampler_, float P);
// vec2 textureQueryLOD(sampler2D sampler_, vec2 P);
// vec2 textureQueryLOD(isampler2D sampler_, vec2 P);
// vec2 textureQueryLOD(usampler2D sampler_, vec2 P);
// vec2 textureQueryLOD(sampler3D sampler_, vec3 P);
// vec2 textureQueryLOD(isampler3D sampler_, vec3 P);
// vec2 textureQueryLOD(usampler3D sampler_, vec3 P);
// vec2 textureQueryLOD(samplerCube sampler_, vec3 P);
// vec2 textureQueryLOD(isamplerCube sampler_, vec3 P);
// vec2 textureQueryLOD(usamplerCube sampler_, vec3 P);
// vec2 textureQueryLOD(sampler1DArray sampler_, float P);
// vec2 textureQueryLOD(isampler1DArray sampler_, float P);
// vec2 textureQueryLOD(usampler1DArray sampler_, float P);
// vec2 textureQueryLOD(sampler2DArray sampler_, vec2 P);
// vec2 textureQueryLOD(isampler2DArray sampler_, vec2 P);
// vec2 textureQueryLOD(usampler2DArray sampler_, vec2 P);
// vec2 textureQueryLOD(samplerCubeArray sampler_, vec3 P);
// vec2 textureQueryLOD(isamplerCubeArray sampler_, vec3 P);
// vec2 textureQueryLOD(usamplerCubeArray sampler_, vec3 P);
// vec2 textureQueryLOD(sampler1DShadow sampler_, float P);
// vec2 textureQueryLOD(sampler2DShadow sampler_, vec2 P);
// vec2 textureQueryLOD(samplerCubeShadow sampler_, vec3 P);
// vec2 textureQueryLOD(sampler1DArrayShadow sampler_, float P);
// vec2 textureQueryLOD(sampler2DArrayShadow sampler_, vec2 P);
// vec2 textureQueryLOD(samplerCubeArrayShadow sampler_, vec3 P);

#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE

int textureSize(f16sampler1D sampler_,int lod);
ivec2 textureSize(f16sampler2D sampler_,int lod);
ivec3 textureSize(f16sampler3D sampler_,int lod);
ivec2 textureSize(f16samplerCube sampler_,int lod);
ivec2 textureSize(f16sampler2DRect sampler_);
int textureSize(f16samplerBuffer sampler_);
ivec2 textureSize(f16sampler1DArray sampler_,int lod);
ivec3 textureSize(f16sampler2DArray sampler_,int lod);
ivec3 textureSize(f16samplerCubeArray sampler_,int lod);
ivec2 textureSize(f16sampler2DMS sampler_);
ivec3 textureSize(f16sampler2DMSArray sampler_);
int textureSize(f16sampler1DShadow sampler_,int lod);
ivec2 textureSize(f16sampler2DShadow sampler_,int lod);
ivec2 textureSize(f16samplerCubeShadow sampler_,int lod);
ivec2 textureSize(f16sampler2DRectShadow sampler_); lod
ivec2 textureSize(f16sampler1DArrayShadow sampler_,int lod);
ivec3 textureSize(f16sampler2DArrayShadow sampler_,int lod);
ivec3 textureSize(f16samplerCubeArrayShadow sampler_,int lod);
vec2 textureQueryLod(f16sampler1D sampler_, float P);
vec2 textureQueryLod(f16sampler2D sampler_, vec2 P);
vec2 textureQueryLod(f16sampler3D sampler_, vec3 P);
vec2 textureQueryLod(f16samplerCube sampler_, vec3 P);
vec2 textureQueryLod(f16sampler1DArray sampler_, float P);
vec2 textureQueryLod(f16sampler2DArray sampler_, vec2 P);
vec2 textureQueryLod(f16samplerCubeArray sampler_, vec3 P);
vec2 textureQueryLod(f16sampler1DShadow sampler_, float P);
vec2 textureQueryLod(f16sampler2DShadow sampler_, vec2 P);
vec2 textureQueryLod(f16samplerCubeShadow sampler_, vec3 P);
vec2 textureQueryLod(f16sampler1DArrayShadow sampler_, float P);
vec2 textureQueryLod(f16sampler2DArrayShadow sampler_, vec2 P);
vec2 textureQueryLod(f16samplerCubeArrayShadow sampler_, vec3 P);
int textureQueryLevels(f16sampler1D sampler_);
int textureQueryLevels(f16sampler2D sampler_);
int textureQueryLevels(f16sampler3D sampler_);
int textureQueryLevels(f16samplerCube sampler_);
int textureQueryLevels(f16sampler2DArray sampler_);
int textureQueryLevels(f16samplerCubeArray sampler_);
int textureQueryLevels(f16sampler1DShadow sampler_);
int textureQueryLevels(f16sampler2DShadow sampler_);
int textureQueryLevels(f16sampler1DArrayShadow sampler_);
int textureQueryLevels(f16sampler2DArrayShadow sampler_);
int textureQueryLevels(f16samplerCubeArrayShadow sampler_);
int textureQueryLevels(f16sampler1DArray sampler_);
int textureQueryLevels(f16samplerCubeShadow sampler_);
int textureSamples(f16sampler2DMS sampler_);
int textureSamples(f16sampler2DMSArray sampler_);

// GL_ARB_texture_query_lod
// vec2 textureQueryLOD(f16sampler1D sampler_, float P);
// vec2 textureQueryLOD(f16sampler2D sampler_, vec2 P);
// vec2 textureQueryLOD(f16sampler3D sampler_, vec3 P);
// vec2 textureQueryLOD(f16samplerCube sampler_, vec3 P);
// vec2 textureQueryLOD(f16sampler1DArray sampler_, float P);
// vec2 textureQueryLOD(f16sampler2DArray sampler_, vec2 P);
// vec2 textureQueryLOD(f16samplerCubeArray sampler_, vec3 P);
// vec2 textureQueryLOD(f16sampler1DShadow sampler_, float P);
// vec2 textureQueryLOD(f16sampler2DShadow sampler_, vec2 P);
// vec2 textureQueryLOD(f16samplerCubeShadow sampler_, vec3 P);
// vec2 textureQueryLOD(f16sampler1DArrayShadow sampler_, float P);
// vec2 textureQueryLOD(f16sampler2DArrayShadow sampler_, vec2 P);
// vec2 textureQueryLOD(f16samplerCubeArrayShadow sampler_, vec3 P);

#endif