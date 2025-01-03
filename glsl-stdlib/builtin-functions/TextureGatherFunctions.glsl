vec4 textureGather(sampler2D sampler_,vec2 P);
vec4 textureGather(sampler2D sampler_,vec2 P,int comp);
ivec4 textureGather(isampler2D sampler_,vec2 P);
ivec4 textureGather(isampler2D sampler_,vec2 P,int comp);
uvec4 textureGather(usampler2D sampler_,vec2 P);
uvec4 textureGather(usampler2D sampler_,vec2 P,int comp);
vec4 textureGather(samplerCube sampler_,vec3 P);
vec4 textureGather(samplerCube sampler_,vec3 P,int comp);
ivec4 textureGather(isamplerCube sampler_,vec3 P);
ivec4 textureGather(isamplerCube sampler_,vec3 P,int comp);
uvec4 textureGather(usamplerCube sampler_,vec3 P);
uvec4 textureGather(usamplerCube sampler_,vec3 P,int comp);
vec4 textureGather(sampler2DRect sampler_,vec2 P);
vec4 textureGather(sampler2DRect sampler_,vec2 P,int comp);
ivec4 textureGather(isampler2DRect sampler_,vec2 P);
ivec4 textureGather(isampler2DRect sampler_,vec2 P,int comp);
uvec4 textureGather(usampler2DRect sampler_,vec2 P);
uvec4 textureGather(usampler2DRect sampler_,vec2 P,int comp);
vec4 textureGather(sampler2DArray sampler_,vec3 P);
vec4 textureGather(sampler2DArray sampler_,vec3 P,int comp);
ivec4 textureGather(isampler2DArray sampler_,vec3 P);
ivec4 textureGather(isampler2DArray sampler_,vec3 P,int comp);
uvec4 textureGather(usampler2DArray sampler_,vec3 P);
uvec4 textureGather(usampler2DArray sampler_,vec3 P,int comp);
vec4 textureGather(samplerCubeArray sampler_,vec4 P);
vec4 textureGather(samplerCubeArray sampler_,vec4 P,int comp);
ivec4 textureGather(isamplerCubeArray sampler_,vec4 P);
ivec4 textureGather(isamplerCubeArray sampler_,vec4 P,int comp);
uvec4 textureGather(usamplerCubeArray sampler_,vec4 P);
uvec4 textureGather(usamplerCubeArray sampler_,vec4 P,int comp);
vec4 textureGather(sampler2DShadow sampler_,vec2 P,float refZ);
vec4 textureGather(samplerCubeShadow sampler_,vec3 P,float refZ);
vec4 textureGather(sampler2DRectShadow sampler_,vec2 P,float refZ);
vec4 textureGather(sampler2DArrayShadow sampler_,vec3 P,float refZ);
vec4 textureGather(samplerCubeArrayShadow sampler_,vec4 P,float refZ);
vec4 textureGatherOffset(sampler2D sampler_,vec2 P,ivec2 offset);
vec4 textureGatherOffset(sampler2D sampler_,vec2 P,ivec2 offset,int comp);
ivec4 textureGatherOffset(isampler2D sampler_,vec2 P,ivec2 offset);
ivec4 textureGatherOffset(isampler2D sampler_,vec2 P,ivec2 offset,int comp);
uvec4 textureGatherOffset(usampler2D sampler_,vec2 P,ivec2 offset);
uvec4 textureGatherOffset(usampler2D sampler_,vec2 P,ivec2 offset,int comp);
vec4 textureGatherOffset(sampler2DRect sampler_,vec2 P,ivec2 offset);
vec4 textureGatherOffset(sampler2DRect sampler_,vec2 P,ivec2 offset,int comp);
ivec4 textureGatherOffset(isampler2DRect sampler_,vec2 P,ivec2 offset);
ivec4 textureGatherOffset(isampler2DRect sampler_,vec2 P,ivec2 offset,int comp);
uvec4 textureGatherOffset(usampler2DRect sampler_,vec2 P,ivec2 offset);
uvec4 textureGatherOffset(usampler2DRect sampler_,vec2 P,ivec2 offset,int comp);
vec4 textureGatherOffset(sampler2DArray sampler_,vec3 P,ivec2 offset);
vec4 textureGatherOffset(sampler2DArray sampler_,vec3 P,ivec2 offset,int comp);
ivec4 textureGatherOffset(isampler2DArray sampler_,vec3 P,ivec2 offset);
ivec4 textureGatherOffset(isampler2DArray sampler_,vec3 P,ivec2 offset,int comp);
uvec4 textureGatherOffset(usampler2DArray sampler_,vec3 P,ivec2 offset);
uvec4 textureGatherOffset(usampler2DArray sampler_,vec3 P,ivec2 offset,int comp);
vec4 textureGatherOffset(sampler2DShadow sampler_,vec2 P,float refZ,ivec2 offset);
vec4 textureGatherOffset(sampler2DRectShadow sampler_,vec2 P,float refZ,ivec2 offset);
vec4 textureGatherOffset(sampler2DArrayShadow sampler_,vec3 P,float refZ,ivec2 offset);
vec4 textureGatherOffsets(sampler2D sampler_,vec2 P,ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2D sampler_,vec2 P,ivec2 offsets[4],int comp);
ivec4 textureGatherOffsets(isampler2D sampler_,vec2 P,ivec2 offsets[4]);
ivec4 textureGatherOffsets(isampler2D sampler_,vec2 P,ivec2 offsets[4],int comp);
uvec4 textureGatherOffsets(usampler2D sampler_,vec2 P,ivec2 offsets[4]);
uvec4 textureGatherOffsets(usampler2D sampler_,vec2 P,ivec2 offsets[4],int comp);
vec4 textureGatherOffsets(sampler2DRect sampler_,vec2 P,ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DRect sampler_,vec2 P,ivec2 offsets[4],int comp);
ivec4 textureGatherOffsets(isampler2DRect sampler_,vec2 P,ivec2 offsets[4]);
ivec4 textureGatherOffsets(isampler2DRect sampler_,vec2 P,ivec2 offsets[4],int comp);
uvec4 textureGatherOffsets(usampler2DRect sampler_,vec2 P,ivec2 offsets[4]);
uvec4 textureGatherOffsets(usampler2DRect sampler_,vec2 P,ivec2 offsets[4],int comp);
vec4 textureGatherOffsets(sampler2DArray sampler_,vec3 P,ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DArray sampler_,vec3 P,ivec2 offsets[4],int comp);
ivec4 textureGatherOffsets(isampler2DArray sampler_,vec3 P,ivec2 offsets[4]);
ivec4 textureGatherOffsets(isampler2DArray sampler_,vec3 P,ivec2 offsets[4],int comp);
uvec4 textureGatherOffsets(usampler2DArray sampler_,vec3 P,ivec2 offsets[4]);
uvec4 textureGatherOffsets(usampler2DArray sampler_,vec3 P,ivec2 offsets[4],int comp);
vec4 textureGatherOffsets(sampler2DShadow sampler_,vec2 P,float refZ,ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DRectShadow sampler_,vec2 P,float refZ,ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DArrayShadow sampler_,vec3 P,float refZ,ivec2 offsets[4]);

#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE

f16vec4 textureGather(f16sampler2D sampler_,vec2 P);
f16vec4 textureGather(f16sampler2D sampler_,vec2 P,int comp);
f16vec4 textureGather(f16sampler2D sampler_,f16vec2 P);
f16vec4 textureGather(f16sampler2D sampler_,f16vec2 P,int comp);
f16vec4 textureGather(f16samplerCube sampler_,vec3 P);
f16vec4 textureGather(f16samplerCube sampler_,vec3 P,int comp);
f16vec4 textureGather(f16samplerCube sampler_,f16vec3 P);
f16vec4 textureGather(f16samplerCube sampler_,f16vec3 P,int comp);
f16vec4 textureGather(f16sampler2DRect sampler_,vec2 P);
f16vec4 textureGather(f16sampler2DRect sampler_,vec2 P,int comp);
f16vec4 textureGather(f16sampler2DRect sampler_,f16vec2 P);
f16vec4 textureGather(f16sampler2DRect sampler_,f16vec2 P,int comp);
f16vec4 textureGather(f16sampler2DArray sampler_,vec3 P);
f16vec4 textureGather(f16sampler2DArray sampler_,vec3 P,int comp);
f16vec4 textureGather(f16sampler2DArray sampler_,f16vec3 P);
f16vec4 textureGather(f16sampler2DArray sampler_,f16vec3 P,int comp);
f16vec4 textureGather(f16samplerCubeArray sampler_,vec4 P);
f16vec4 textureGather(f16samplerCubeArray sampler_,vec4 P,int comp);
f16vec4 textureGather(f16samplerCubeArray sampler_,f16vec4 P);
f16vec4 textureGather(f16samplerCubeArray sampler_,f16vec4 P,int comp);
f16vec4 textureGather(f16sampler2DShadow sampler_,vec2 P,float refZ);
f16vec4 textureGather(f16sampler2DShadow sampler_,f16vec2 P,float refZ);
f16vec4 textureGather(f16samplerCubeShadow sampler_,vec3 P,float refZ);
f16vec4 textureGather(f16samplerCubeShadow sampler_,f16vec3 P,float refZ);
f16vec4 textureGather(f16sampler2DRectShadow sampler_,vec2 P,float refZ);
f16vec4 textureGather(f16sampler2DRectShadow sampler_,f16vec2 P,float refZ);
f16vec4 textureGather(f16sampler2DArrayShadow sampler_,vec3 P,float refZ);
f16vec4 textureGather(f16sampler2DArrayShadow sampler_,f16vec3 P,float refZ);
f16vec4 textureGather(f16samplerCubeArrayShadow sampler_,vec4 P,float refZ);
f16vec4 textureGather(f16samplerCubeArrayShadow sampler_,f16vec4 P,float refZ);
f16vec4 textureGatherOffset(f16sampler2D sampler_,vec2 P,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2D sampler_,vec2 P,ivec2 offset,int comp);
f16vec4 textureGatherOffset(f16sampler2D sampler_,f16vec2 P,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2D sampler_,f16vec2 P,ivec2 offset,int comp);
f16vec4 textureGatherOffset(f16sampler2DRect sampler_,vec2 P,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DRect sampler_,vec2 P,ivec2 offset,int comp);
f16vec4 textureGatherOffset(f16sampler2DRect sampler_,f16vec2 P,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DRect sampler_,f16vec2 P,ivec2 offset,int comp);
f16vec4 textureGatherOffset(f16sampler2DArray sampler_,vec3 P,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DArray sampler_,vec3 P,ivec2 offset,int comp);
f16vec4 textureGatherOffset(f16sampler2DArray sampler_,f16vec3 P,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DArray sampler_,f16vec3 P,ivec2 offset,int comp);
f16vec4 textureGatherOffset(f16sampler2DShadow sampler_,vec2 P,float refZ,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DShadow sampler_,f16vec2 P,float refZ,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DRectShadow sampler_,vec2 P,float refZ,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DRectShadow sampler_,f16vec2 P,float refZ,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DArrayShadow sampler_,vec3 P,float refZ,ivec2 offset);
f16vec4 textureGatherOffset(f16sampler2DArrayShadow sampler_,f16vec3 P,float refZ,ivec2 offset);
f16vec4 textureGatherOffsets(f16sampler2D sampler_,vec2 P,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2D sampler_,vec2 P,ivec2 offsets[4],int comp);
f16vec4 textureGatherOffsets(f16sampler2D sampler_,f16vec2 P,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2D sampler_,f16vec2 P,ivec2 offsets[4],int comp);
f16vec4 textureGatherOffsets(f16sampler2DRect sampler_,vec2 P,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DRect sampler_,vec2 P,ivec2 offsets[4],int comp);
f16vec4 textureGatherOffsets(f16sampler2DRect sampler_,f16vec2 P,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DRect sampler_,f16vec2 P,ivec2 offsets[4],int comp);
f16vec4 textureGatherOffsets(f16sampler2DArray sampler_,vec3 P,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DArray sampler_,vec3 P,ivec2 offsets[4],int comp);
f16vec4 textureGatherOffsets(f16sampler2DArray sampler_,f16vec3 P,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DArray sampler_,f16vec3 P,ivec2 offsets[4],int comp);
f16vec4 textureGatherOffsets(f16sampler2DShadow sampler_,vec2 P,float refZ,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DShadow sampler_,f16vec2 P,float refZ,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DRectShadow sampler_,vec2 P,float refZ,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DRectShadow sampler_,f16vec2 P,float refZ,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DArrayShadow sampler_,vec3 P,float refZ,ivec2 offsets[4]);
f16vec4 textureGatherOffsets(f16sampler2DArrayShadow sampler_,f16vec3 P,float refZ,ivec2 offsets[4]);

#endif