from StdlibBuilder import StdlibBuilder

def addBuiltinConstant(builder: StdlibBuilder):
    builder.addSnippet("""
const int gl_MaxVertexAttribs = 16;
const int gl_MaxVertexUniformVectors = 256;
const int gl_MaxVertexUniformComponents = 1024;
const int gl_MaxVertexOutputComponents = 64;
const int gl_MaxVaryingComponents = 60;
const int gl_MaxVaryingVectors = 15;
const int gl_MaxVertexTextureImageUnits = 16;
const int gl_MaxVertexImageUniforms = 0;
const int gl_MaxVertexAtomicCounters = 0;
const int gl_MaxVertexAtomicCounterBuffers = 0;
const int gl_MaxTessPatchComponents = 120;
const int gl_MaxPatchVertices = 32;
const int gl_MaxTessGenLevel = 64;
const int gl_MaxTessControlInputComponents = 128;
const int gl_MaxTessControlOutputComponents = 128;
const int gl_MaxTessControlTextureImageUnits = 16;
const int gl_MaxTessControlUniformComponents = 1024;
const int gl_MaxTessControlTotalOutputComponents = 4096;
const int gl_MaxTessControlImageUniforms = 0;
const int gl_MaxTessControlAtomicCounters = 0;
const int gl_MaxTessControlAtomicCounterBuffers = 0;
const int gl_MaxTessEvaluationInputComponents = 128;
const int gl_MaxTessEvaluationOutputComponents = 128;
const int gl_MaxTessEvaluationTextureImageUnits = 16;
const int gl_MaxTessEvaluationUniformComponents = 1024;
const int gl_MaxTessEvaluationImageUniforms = 0;
const int gl_MaxTessEvaluationAtomicCounters = 0;
const int gl_MaxTessEvaluationAtomicCounterBuffers = 0;
const int gl_MaxGeometryInputComponents = 64;
const int gl_MaxGeometryOutputComponents = 128;
const int gl_MaxGeometryImageUniforms = 0;
const int gl_MaxGeometryTextureImageUnits = 16;
const int gl_MaxGeometryOutputVertices = 256;
const int gl_MaxGeometryTotalOutputComponents = 1024;
const int gl_MaxGeometryUniformComponents = 1024;
const int gl_MaxGeometryVaryingComponents = 64; // deprecated
const int gl_MaxGeometryAtomicCounters = 0;
const int gl_MaxGeometryAtomicCounterBuffers = 0;
const int gl_MaxFragmentImageUniforms = 8;
const int gl_MaxFragmentInputComponents = 128;
const int gl_MaxFragmentUniformVectors = 256;
const int gl_MaxFragmentUniformComponents = 1024;
const int gl_MaxFragmentAtomicCounters = 8;
const int gl_MaxFragmentAtomicCounterBuffers = 1;
const int gl_MaxDrawBuffers = 8;
const int gl_MaxTextureImageUnits = 16;
const int gl_MinProgramTexelOffset = -8;
const int gl_MaxProgramTexelOffset = 7;
const int gl_MaxImageUnits = 8;
const int gl_MaxSamples = 4;
const int gl_MaxImageSamples = 0;
const int gl_MaxClipDistances = 8;
const int gl_MaxCullDistances = 8;
const int gl_MaxViewports = 16;
const int gl_MaxComputeImageUniforms = 8;
const ivec3 gl_MaxComputeWorkGroupCount = { 65535, 65535, 65535 };
const ivec3 gl_MaxComputeWorkGroupSize = { 1024, 1024, 64 };
const int gl_MaxComputeUniformComponents = 1024;
const int gl_MaxComputeTextureImageUnits = 16;
const int gl_MaxComputeAtomicCounters = 8;
const int gl_MaxComputeAtomicCounterBuffers = 8;
const int gl_MaxCombinedTextureImageUnits = 96;
const int gl_MaxCombinedImageUniforms = 48;
const int gl_MaxCombinedImageUnitsAndFragmentOutputs = 8; // deprecated
const int gl_MaxCombinedShaderOutputResources = 16;
const int gl_MaxCombinedAtomicCounters = 8;
const int gl_MaxCombinedAtomicCounterBuffers = 1;
const int gl_MaxCombinedClipAndCullDistances = 8;
const int gl_MaxAtomicCounterBindings = 1;
const int gl_MaxAtomicCounterBufferSize = 32;
const int gl_MaxTransformFeedbackBuffers = 4;
const int gl_MaxTransformFeedbackInterleavedComponents = 64;
                       
#if __GLSLD_TARGET_API_VULKAN
const highp int gl_MaxInputAttachments = 1; // only present when targeting Vulkan
#endif
""")
    
def addBuiltinVariablesForVertexShader(builder: StdlibBuilder):
    builder.addSnippet("""
#if __GLSLD_SHADER_STAGE_VERTEX

#if __GLSLD_TARGET_API_OPENGL
in int gl_VertexID;
in int gl_InstanceID;
#endif

#if __GLSLD_TARGET_API_VULKAN
in int gl_VertexIndex;
in int gl_InstanceIndex;
#endif

in int gl_DrawID;
in int gl_BaseVertex;
in int gl_BaseInstance;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};

#endif
""")
    
def addBuiltinVariablesForFragmentShader(builder: StdlibBuilder):
    # FIXME: correct qualifiers
    builder.addSnippet("""

#if __GLSLD_SHADER_STAGE_FRAGMENT

in vec4  gl_FragCoord;
in bool  gl_FrontFacing;
in float gl_ClipDistance[];
in float gl_CullDistance[];
in vec2 gl_PointCoord;
flat in int gl_PrimitiveID;
flat in  int  gl_SampleID;
in  vec2 gl_SamplePosition;
flat in  int  gl_SampleMaskIn[];
flat in int gl_Layer;
flat in int gl_ViewportIndex;
in bool gl_HelperInvocation;

out float gl_FragDepth;
out int  gl_SampleMask[];

#endif
""")
    
def addBuiltinVariablesForComputeShader(builder: StdlibBuilder):
    builder.addSnippet("""
#if __GLSLD_SHADER_STAGE_COMPUTE

in    highp uvec3 gl_NumWorkGroups;
const highp uvec3 gl_WorkGroupSize = uvec3(1,1,1);
in highp uvec3 gl_WorkGroupID;
in highp uvec3 gl_LocalInvocationID;
in highp uvec3 gl_GlobalInvocationID;
in highp uint gl_LocalInvocationIndex;

#endif
""")
    
def addBuiltinVariablesForGeometryShader(builder: StdlibBuilder):
    builder.addSnippet("""
#if __GLSLD_SHADER_STAGE_GEOMETRY

void EmitStreamVertex(int stream);
void EndStreamPrimitive(int stream);
void EmitVertex();
void EndPrimitive();

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
    // vec4 gl_SecondaryPositionNV;
    // vec4 gl_PositionPerViewNV[];
} gl_in[];
in int gl_PrimitiveIDIn;
out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};
out int gl_PrimitiveID;
out int gl_Layer;
out int gl_ViewportIndex;

#endif
""")
    
def addBuiltinVariablesForTessControlShader(builder: StdlibBuilder):
    builder.addSnippet("""
#if __GLSLD_SHADER_STAGE_TESS_CTRL

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_in[gl_MaxPatchVertices];

in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
in int gl_InvocationID;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
    // int  gl_ViewportMask[];
    // vec4 gl_SecondaryPositionNV;
    // int  gl_SecondaryViewportMaskNV[];
    // vec4 gl_PositionPerViewNV[];
    // int  gl_ViewportMaskPerViewNV[];
} gl_out[];
patch out float gl_TessLevelOuter[4];
patch out float gl_TessLevelInner[2];

#endif
""")
    
def addBuiltinVariablesForTessEvaluationShader(builder: StdlibBuilder):
    builder.addSnippet("""
#if __GLSLD_SHADER_STAGE_TESS_EVAL

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_in[gl_MaxPatchVertices];

in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
in vec3 gl_TessCoord;
patch in float gl_TessLevelOuter[4];
patch in float gl_TessLevelInner[2];

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};

#endif
""")
    
def addBuiltinVariablesForRayTracingEXTShader(builder: StdlibBuilder):
    builder.addSnippet("""
#if __GLSLD_FEATURE_ENABLE_RAY_TRACING_EXT

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT || __GLSLD_SHADER_STAGE_RAY_MISS || __GLSLD_SHADER_STAGE_RAY_CALLABLE
const uint gl_RayFlagsNoneEXT = 0U;
const uint gl_RayFlagsOpaqueEXT = 1U;
const uint gl_RayFlagsNoOpaqueEXT = 2U;
const uint gl_RayFlagsTerminateOnFirstHitEXT = 4U;
const uint gl_RayFlagsSkipClosestHitShaderEXT = 8U;
const uint gl_RayFlagsCullBackFacingTrianglesEXT = 16U;
const uint gl_RayFlagsCullFrontFacingTrianglesEXT = 32U;
const uint gl_RayFlagsCullOpaqueEXT = 64U;
const uint gl_RayFlagsCullNoOpaqueEXT = 128U;
const uint gl_RayFlagsSkipTrianglesEXT = 256U;
const uint gl_RayFlagsSkipAABBEXT = 512U;
const uint gl_RayFlagsForceOpacityMicromap2StateEXT = 1024U;
const uint gl_HitKindFrontFacingTriangleEXT = 254U;
const uint gl_HitKindBackFacingTriangleEXT = 255U;

in    uvec3  gl_LaunchIDEXT;
in    uvec3  gl_LaunchSizeEXT;
#endif

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_MISS || __GLSLD_SHADER_STAGE_RAY_CALLABLE
void executeCallableEXT(uint sbtRecordIndex, int callable);
#endif

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_MISS
void traceRayEXT(accelerationStructureEXT topLevel,uint rayFlags,uint cullMask,uint sbtRecordOffset,uint sbtRecordStride,uint missIndex,vec3 origin,float tMin,vec3 direction,float tMax,int payload);
#endif

#if __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT || __GLSLD_SHADER_STAGE_RAY_MISS
in    vec3   gl_WorldRayOriginEXT;
in    vec3   gl_WorldRayDirectionEXT;
in    float  gl_RayTminEXT;
in    float  gl_RayTmaxEXT;
in    uint   gl_IncomingRayFlagsEXT;
in    uint   gl_CullMaskEXT;
#endif

#if __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT
in     int   gl_PrimitiveID;
in     int   gl_InstanceID;
in     int   gl_InstanceCustomIndexEXT;
in     int   gl_GeometryIndexEXT;
in    vec3   gl_ObjectRayOriginEXT;
in    vec3   gl_ObjectRayDirectionEXT;
in    float  gl_HitTEXT;
in    uint   gl_HitKindEXT;
in    mat4x3 gl_ObjectToWorldEXT;
in    mat3x4 gl_ObjectToWorld3x4EXT;
in    mat4x3 gl_WorldToObjectEXT;
in    mat3x4 gl_WorldToObject3x4EXT;
#endif

#if __GLSLD_SHADER_STAGE_RAY_INTERSECT
bool reportIntersectionEXT(float hitT, uint hitKind);
#endif

#endif
""")
    
def addBuiltinVariablesForRayTracingNVShader(builder: StdlibBuilder):
    builder.addSnippet("""

#if __GLSLD_FEATURE_ENABLE_RAY_TRACING_NV

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT || __GLSLD_SHADER_STAGE_RAY_MISS || __GLSLD_SHADER_STAGE_RAY_CALLABLE
const uint gl_RayFlagsNoneNV = 0U;
const uint gl_RayFlagsOpaqueNV = 1U;
const uint gl_RayFlagsNoOpaqueNV = 2U;
const uint gl_RayFlagsTerminateOnFirstHitNV = 4U;
const uint gl_RayFlagsSkipClosestHitShaderNV = 8U;
const uint gl_RayFlagsCullBackFacingTrianglesNV = 16U;
const uint gl_RayFlagsCullFrontFacingTrianglesNV = 32U;
const uint gl_RayFlagsCullOpaqueNV = 64U;
const uint gl_RayFlagsCullNoOpaqueNV = 128U;

in    uvec3  gl_LaunchIDNV;
in    uvec3  gl_LaunchSizeNV;
#endif

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_MISS || __GLSLD_SHADER_STAGE_RAY_CALLABLE
void executeCallableNV(uint sbtRecordIndex, int callable);
#endif

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_MISS
void traceNV(accelerationStructureNV topLevel,uint rayFlags,uint cullMask,uint sbtRecordOffset,uint sbtRecordStride,uint missIndex,vec3 origin,float tMin,vec3 direction,float tMax,int payload);
void traceRayMotionNV(accelerationStructureNV topLevel,uint rayFlags,uint cullMask,uint sbtRecordOffset,uint sbtRecordStride,uint missIndex,vec3 origin,float tMin,vec3 direction,float tMax,float currentTime,int payload);
#endif

#if __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT || __GLSLD_SHADER_STAGE_RAY_MISS
in    vec3   gl_WorldRayOriginNV;
in    vec3   gl_WorldRayDirectionNV;
in    vec3   gl_ObjectRayOriginNV;
in    vec3   gl_ObjectRayDirectionNV;
in    float  gl_RayTminNV;
in    float  gl_RayTmaxNV;
in    uint   gl_IncomingRayFlagsNV;
in    float  gl_CurrentRayTimeNV;
#endif

#if __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT
in     int   gl_PrimitiveID;
in     int   gl_InstanceID;
in     int   gl_InstanceCustomIndexNV;
in    float  gl_HitTNV;
in    uint   gl_HitKindNV;
in    mat4x3 gl_ObjectToWorldNV;
in    mat4x3 gl_WorldToObjectNV;
#endif

#if __GLSLD_SHADER_STAGE_ANY_HIT
void ignoreIntersectionNV();
void terminateRayNV();
#endif


#if __GLSLD_SHADER_STAGE_RAY_INTERSECT
bool reportIntersectionNV(float hitT, uint hitKind);
#endif

#endif
""")