// clang-format off
#include <string_view>

namespace glsld {
inline std::string_view GlslStdlibText = R"glsl(

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


#if __GLSLD_SHADER_STAGE_COMPUTE

in    highp uvec3 gl_NumWorkGroups;
const highp uvec3 gl_WorkGroupSize = uvec3(1,1,1);
in highp uvec3 gl_WorkGroupID;
in highp uvec3 gl_LocalInvocationID;
in highp uvec3 gl_GlobalInvocationID;
in highp uint gl_LocalInvocationIndex;

#endif


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


#if __GLSLD_FEATURE_ENABLE_RAY_QUERY

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

const uint gl_RayQueryCandidateIntersectionEXT = 0U;
const uint gl_RayQueryCommittedIntersectionEXT = 1U;
const uint gl_RayQueryCommittedIntersectionNoneEXT = 0U;
const uint gl_RayQueryCommittedIntersectionTriangleEXT = 1U;
const uint gl_RayQueryCommittedIntersectionGeneratedEXT = 2U;
const uint gl_RayQueryCandidateIntersectionTriangleEXT = 0U;
const uint gl_RayQueryCandidateIntersectionAABBEXT = 1U;

void rayQueryInitializeEXT(rayQueryEXT rayQuery, accelerationStructureEXT topLevel, uint rayFlags, uint cullMask, vec3 origin, float tMin, vec3 direction, float tMax);
void rayQueryTerminateEXT(rayQueryEXT rayQuery);
void rayQueryGenerateIntersectionEXT(rayQueryEXT rayQuery, float tHit);
void rayQueryConfirmIntersectionEXT(rayQueryEXT rayQuery);
bool rayQueryProceedEXT(rayQueryEXT rayQuery);
uint rayQueryGetIntersectionTypeEXT(rayQueryEXT rayQuery, bool committed);
float rayQueryGetRayTMinEXT(rayQueryEXT rayQuery);
uint rayQueryGetRayFlagsEXT(rayQueryEXT rayQuery);
vec3 rayQueryGetWorldRayOriginEXT(rayQueryEXT rayQuery);
vec3 rayQueryGetWorldRayDirectionEXT(rayQueryEXT rayQuery);
float rayQueryGetIntersectionTEXT(rayQueryEXT rayQuery, bool committed);
int rayQueryGetIntersectionInstanceCustomIndexEXT(rayQueryEXT rayQuery, bool committed);
int rayQueryGetIntersectionInstanceIdEXT(rayQueryEXT rayQuery, bool committed);
uint rayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetEXT(rayQueryEXT rayQuery, bool committed);
int rayQueryGetIntersectionGeometryIndexEXT(rayQueryEXT rayQuery, bool committed);
int rayQueryGetIntersectionPrimitiveIndexEXT(rayQueryEXT rayQuery, bool committed);
vec2 rayQueryGetIntersectionBarycentricsEXT(rayQueryEXT rayQuery, bool committed);
bool rayQueryGetIntersectionFrontFaceEXT(rayQueryEXT rayQuery, bool committed);
bool rayQueryGetIntersectionCandidateAABBOpaqueEXT(rayQueryEXT rayQuery committed);
vec3 rayQueryGetIntersectionObjectRayDirectionEXT(rayQueryEXT rayQuery, bool committed);
vec3 rayQueryGetIntersectionObjectRayOriginEXT(rayQueryEXT rayQuery, bool committed);
mat4x3 rayQueryGetIntersectionObjectToWorldEXT(rayQueryEXT rayQuery, bool committed);
mat4x3 rayQueryGetIntersectionWorldToObjectEXT(rayQueryEXT rayQuery, bool committed);

#endif


#if __GLSLD_FEATURE_ENABLE_MEMORY_SCOPE_SEMANTICS

const int gl_ScopeDevice      = 1;
const int gl_ScopeWorkgroup   = 2;
const int gl_ScopeSubgroup    = 3;
const int gl_ScopeInvocation  = 4;
const int gl_ScopeQueueFamily = 5;
const int gl_ScopeShaderCallEXT = 6;

const int gl_SemanticsRelaxed         = 0x0;
const int gl_SemanticsAcquire         = 0x2;
const int gl_SemanticsRelease         = 0x4;
const int gl_SemanticsAcquireRelease  = 0x8;
const int gl_SemanticsMakeAvailable   = 0x2000;
const int gl_SemanticsMakeVisible     = 0x4000;
const int gl_SemanticsVolatile        = 0x8000;

const int gl_StorageSemanticsNone     = 0x0;
const int gl_StorageSemanticsBuffer   = 0x40;
const int gl_StorageSemanticsShared   = 0x100;
const int gl_StorageSemanticsImage    = 0x800;
const int gl_StorageSemanticsOutput   = 0x1000;

#endif


#if __GLSLD_FEATURE_ENABLE_SHADER_SM_BUILTIN_NV

#if __GLSLD_SHADER_STAGE_VERTEX || __GLSLD_SHADER_STAGE_GEOMETRY || __GLSLD_SHADER_STAGE_TESS_CTRL || __GLSLD_SHADER_STAGE_TESS_EVAL || __GLSLD_SHADER_STAGE_COMPUTE || __GLSLD_SHADER_STAGE_TASK || __GLSLD_SHADER_STAGE_MESH
in highp   uint  gl_WarpsPerSMNV;
in highp   uint  gl_SMCountNV;
in highp   uint  gl_WarpIDNV;
in highp   uint  gl_SMIDNV;
#endif

#if __GLSLD_SHADER_STAGE_FRAGMENT
flat in highp   uint  gl_WarpsPerSMNV;
flat in highp   uint  gl_SMCountNV;
flat in highp   uint  gl_WarpIDNV;
flat in highp   uint  gl_SMIDNV;
#endif

#if __GLSLD_SHADER_STAGE_RAY_GEN || __GLSLD_SHADER_STAGE_ANY_HIT || __GLSLD_SHADER_STAGE_CLOSEST_HIT || __GLSLD_SHADER_STAGE_RAY_INTERSECT || __GLSLD_SHADER_STAGE_RAY_MISS || __GLSLD_SHADER_STAGE_RAY_CALLABLE
in highp    uint  gl_WarpsPerSMNV;
in highp    uint  gl_SMCountNV;
in highp volatile uint  gl_WarpIDNV;
in highp volatile uint  gl_SMIDNV;
#endif

#endif

// Converts degrees to radians, i.e. π / 180 * degrees.
float radians(float degrees);
// Converts radians to degrees, i.e. 180 / π * radians.
float degrees(float radians);
// The standard trigonometric sine function.
float sin(float x);
// The standard trigonometric cosine function.
float cos(float x);
// The standard trigonometric tangent function.
float tan(float x);
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
float asin(float x);
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
float acos(float x);
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
float atan(float x);
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
float sinh(float x);
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
float cosh(float x);
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
float tanh(float x);
// Arc hyperbolic sine; returns the inverse of sinh.
float asinh(float x);
// Arc hyperbolic cosine; returns the inverse of cosh.
float acosh(float x);
// Arc hyperbolic tangent; returns the inverse of tanh.
float atanh(float x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts degrees to radians, i.e. π / 180 * degrees.
float16_t radians(float16_t degrees);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts radians to degrees, i.e. 180 / π * radians.
float16_t degrees(float16_t radians);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric sine function.
float16_t sin(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric cosine function.
float16_t cos(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric tangent function.
float16_t tan(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
float16_t asin(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
float16_t acos(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
float16_t atan(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
float16_t sinh(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
float16_t cosh(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
float16_t tanh(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic sine; returns the inverse of sinh.
float16_t asinh(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic cosine; returns the inverse of cosh.
float16_t acosh(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic tangent; returns the inverse of tanh.
float16_t atanh(float16_t x);
#endif
// Converts degrees to radians, i.e. π / 180 * degrees.
vec2 radians(vec2 degrees);
// Converts radians to degrees, i.e. 180 / π * radians.
vec2 degrees(vec2 radians);
// The standard trigonometric sine function.
vec2 sin(vec2 x);
// The standard trigonometric cosine function.
vec2 cos(vec2 x);
// The standard trigonometric tangent function.
vec2 tan(vec2 x);
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
vec2 asin(vec2 x);
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
vec2 acos(vec2 x);
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
vec2 atan(vec2 x);
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
vec2 sinh(vec2 x);
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
vec2 cosh(vec2 x);
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
vec2 tanh(vec2 x);
// Arc hyperbolic sine; returns the inverse of sinh.
vec2 asinh(vec2 x);
// Arc hyperbolic cosine; returns the inverse of cosh.
vec2 acosh(vec2 x);
// Arc hyperbolic tangent; returns the inverse of tanh.
vec2 atanh(vec2 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts degrees to radians, i.e. π / 180 * degrees.
f16vec2 radians(f16vec2 degrees);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts radians to degrees, i.e. 180 / π * radians.
f16vec2 degrees(f16vec2 radians);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric sine function.
f16vec2 sin(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric cosine function.
f16vec2 cos(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric tangent function.
f16vec2 tan(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
f16vec2 asin(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
f16vec2 acos(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
f16vec2 atan(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
f16vec2 sinh(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
f16vec2 cosh(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
f16vec2 tanh(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic sine; returns the inverse of sinh.
f16vec2 asinh(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic cosine; returns the inverse of cosh.
f16vec2 acosh(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic tangent; returns the inverse of tanh.
f16vec2 atanh(f16vec2 x);
#endif
// Converts degrees to radians, i.e. π / 180 * degrees.
vec3 radians(vec3 degrees);
// Converts radians to degrees, i.e. 180 / π * radians.
vec3 degrees(vec3 radians);
// The standard trigonometric sine function.
vec3 sin(vec3 x);
// The standard trigonometric cosine function.
vec3 cos(vec3 x);
// The standard trigonometric tangent function.
vec3 tan(vec3 x);
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
vec3 asin(vec3 x);
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
vec3 acos(vec3 x);
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
vec3 atan(vec3 x);
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
vec3 sinh(vec3 x);
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
vec3 cosh(vec3 x);
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
vec3 tanh(vec3 x);
// Arc hyperbolic sine; returns the inverse of sinh.
vec3 asinh(vec3 x);
// Arc hyperbolic cosine; returns the inverse of cosh.
vec3 acosh(vec3 x);
// Arc hyperbolic tangent; returns the inverse of tanh.
vec3 atanh(vec3 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts degrees to radians, i.e. π / 180 * degrees.
f16vec3 radians(f16vec3 degrees);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts radians to degrees, i.e. 180 / π * radians.
f16vec3 degrees(f16vec3 radians);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric sine function.
f16vec3 sin(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric cosine function.
f16vec3 cos(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric tangent function.
f16vec3 tan(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
f16vec3 asin(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
f16vec3 acos(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
f16vec3 atan(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
f16vec3 sinh(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
f16vec3 cosh(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
f16vec3 tanh(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic sine; returns the inverse of sinh.
f16vec3 asinh(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic cosine; returns the inverse of cosh.
f16vec3 acosh(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic tangent; returns the inverse of tanh.
f16vec3 atanh(f16vec3 x);
#endif
// Converts degrees to radians, i.e. π / 180 * degrees.
vec4 radians(vec4 degrees);
// Converts radians to degrees, i.e. 180 / π * radians.
vec4 degrees(vec4 radians);
// The standard trigonometric sine function.
vec4 sin(vec4 x);
// The standard trigonometric cosine function.
vec4 cos(vec4 x);
// The standard trigonometric tangent function.
vec4 tan(vec4 x);
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
vec4 asin(vec4 x);
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
vec4 acos(vec4 x);
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
vec4 atan(vec4 x);
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
vec4 sinh(vec4 x);
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
vec4 cosh(vec4 x);
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
vec4 tanh(vec4 x);
// Arc hyperbolic sine; returns the inverse of sinh.
vec4 asinh(vec4 x);
// Arc hyperbolic cosine; returns the inverse of cosh.
vec4 acosh(vec4 x);
// Arc hyperbolic tangent; returns the inverse of tanh.
vec4 atanh(vec4 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts degrees to radians, i.e. π / 180 * degrees.
f16vec4 radians(f16vec4 degrees);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Converts radians to degrees, i.e. 180 / π * radians.
f16vec4 degrees(f16vec4 radians);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric sine function.
f16vec4 sin(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric cosine function.
f16vec4 cos(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// The standard trigonometric tangent function.
f16vec4 tan(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc sine. Returns an angle whose sine is x. The range of values returned by this function is [-π / 2, π / 2]. Results are undefined if |x| > 1.
f16vec4 asin(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc cosine. Returns an angle whose cosine is x. The range of values returned by this function is [0, π]. Results are undefined if |x| > 1.
f16vec4 acos(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc tangent. Returns an angle whose tangent is y / x. The signs of x and y are used to determine what quadrant the angle is in. The range of values returned by this function is [-π, π]. Results are undefined if x and y are both 0.
f16vec4 atan(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic sine function (e^x - e^-x) / 2.
f16vec4 sinh(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic cosine function (e^x + e^-x) / 2.
f16vec4 cosh(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
f16vec4 tanh(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic sine; returns the inverse of sinh.
f16vec4 asinh(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic cosine; returns the inverse of cosh.
f16vec4 acosh(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Arc hyperbolic tangent; returns the inverse of tanh.
f16vec4 atanh(f16vec4 x);
#endif
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
float pow(float x, float y);
// Returns the natural exponentiation of x, i.e., e^x.
float exp(float x);
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
float log(float x);
// Returns 2 raised to the x power, i.e., 2^x.
float exp2(float x);
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
float log2(float x);
// Returns sqrt(x). Results are undefined if x < 0.
float sqrt(float x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
float inversesqrt(float x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
float16_t pow(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural exponentiation of x, i.e., e^x.
float16_t exp(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
float16_t log(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 2 raised to the x power, i.e., 2^x.
float16_t exp2(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
float16_t log2(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns sqrt(x). Results are undefined if x < 0.
float16_t sqrt(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
float16_t inversesqrt(float16_t x);
#endif
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
vec2 pow(vec2 x, vec2 y);
// Returns the natural exponentiation of x, i.e., e^x.
vec2 exp(vec2 x);
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
vec2 log(vec2 x);
// Returns 2 raised to the x power, i.e., 2^x.
vec2 exp2(vec2 x);
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
vec2 log2(vec2 x);
// Returns sqrt(x). Results are undefined if x < 0.
vec2 sqrt(vec2 x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
vec2 inversesqrt(vec2 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
f16vec2 pow(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural exponentiation of x, i.e., e^x.
f16vec2 exp(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
f16vec2 log(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 2 raised to the x power, i.e., 2^x.
f16vec2 exp2(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
f16vec2 log2(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns sqrt(x). Results are undefined if x < 0.
f16vec2 sqrt(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
f16vec2 inversesqrt(f16vec2 x);
#endif
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
vec3 pow(vec3 x, vec3 y);
// Returns the natural exponentiation of x, i.e., e^x.
vec3 exp(vec3 x);
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
vec3 log(vec3 x);
// Returns 2 raised to the x power, i.e., 2^x.
vec3 exp2(vec3 x);
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
vec3 log2(vec3 x);
// Returns sqrt(x). Results are undefined if x < 0.
vec3 sqrt(vec3 x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
vec3 inversesqrt(vec3 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
f16vec3 pow(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural exponentiation of x, i.e., e^x.
f16vec3 exp(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
f16vec3 log(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 2 raised to the x power, i.e., 2^x.
f16vec3 exp2(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
f16vec3 log2(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns sqrt(x). Results are undefined if x < 0.
f16vec3 sqrt(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
f16vec3 inversesqrt(f16vec3 x);
#endif
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
vec4 pow(vec4 x, vec4 y);
// Returns the natural exponentiation of x, i.e., e^x.
vec4 exp(vec4 x);
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
vec4 log(vec4 x);
// Returns 2 raised to the x power, i.e., 2^x.
vec4 exp2(vec4 x);
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
vec4 log2(vec4 x);
// Returns sqrt(x). Results are undefined if x < 0.
vec4 sqrt(vec4 x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
vec4 inversesqrt(vec4 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x raised to the y power, i.e., x^y. Results are undefined if x < 0. Results are undefined if x = 0 and y <= 0.
f16vec4 pow(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural exponentiation of x, i.e., e^x.
f16vec4 exp(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the natural logarithm of x, i.e., returns the value y which satisfies the equation x = e^y. Results are undefined if x <= 0.
f16vec4 log(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 2 raised to the x power, i.e., 2^x.
f16vec4 exp2(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the base 2 logarithm of x, i.e., returns the value y which satisfies the equation x = 2^y. Results are undefined if x <= 0.
f16vec4 log2(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns sqrt(x). Results are undefined if x < 0.
f16vec4 sqrt(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
f16vec4 inversesqrt(f16vec4 x);
#endif
// Returns sqrt(x). Results are undefined if x < 0.
double sqrt(double x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
double inversesqrt(double x);
// Returns sqrt(x). Results are undefined if x < 0.
dvec2 sqrt(dvec2 x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
dvec2 inversesqrt(dvec2 x);
// Returns sqrt(x). Results are undefined if x < 0.
dvec3 sqrt(dvec3 x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
dvec3 inversesqrt(dvec3 x);
// Returns sqrt(x). Results are undefined if x < 0.
dvec4 sqrt(dvec4 x);
// Returns 1 / sqrt(x). Results are undefined if x <= 0.
dvec4 inversesqrt(dvec4 x);
// Returns x if x >= 0; otherwise it returns -x.
float abs(float x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
float sign(float x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
float16_t abs(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
float16_t sign(float16_t x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
double abs(double x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
double sign(double x);
// Returns x if x >= 0; otherwise it returns -x.
int abs(int x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
int sign(int x);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns x if x >= 0; otherwise it returns -x.
int8_t abs(int8_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
int8_t sign(int8_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
int16_t abs(int16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
int16_t sign(int16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns x if x >= 0; otherwise it returns -x.
int64_t abs(int64_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
int64_t sign(int64_t x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
vec2 abs(vec2 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
vec2 sign(vec2 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
f16vec2 abs(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
f16vec2 sign(f16vec2 x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
dvec2 abs(dvec2 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
dvec2 sign(dvec2 x);
// Returns x if x >= 0; otherwise it returns -x.
ivec2 abs(ivec2 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
ivec2 sign(ivec2 x);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i8vec2 abs(i8vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i8vec2 sign(i8vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i16vec2 abs(i16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i16vec2 sign(i16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i64vec2 abs(i64vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i64vec2 sign(i64vec2 x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
vec3 abs(vec3 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
vec3 sign(vec3 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
f16vec3 abs(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
f16vec3 sign(f16vec3 x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
dvec3 abs(dvec3 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
dvec3 sign(dvec3 x);
// Returns x if x >= 0; otherwise it returns -x.
ivec3 abs(ivec3 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
ivec3 sign(ivec3 x);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i8vec3 abs(i8vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i8vec3 sign(i8vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i16vec3 abs(i16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i16vec3 sign(i16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i64vec3 abs(i64vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i64vec3 sign(i64vec3 x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
vec4 abs(vec4 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
vec4 sign(vec4 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
f16vec4 abs(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
f16vec4 sign(f16vec4 x);
#endif
// Returns x if x >= 0; otherwise it returns -x.
dvec4 abs(dvec4 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
dvec4 sign(dvec4 x);
// Returns x if x >= 0; otherwise it returns -x.
ivec4 abs(ivec4 x);
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
ivec4 sign(ivec4 x);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i8vec4 abs(i8vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i8vec4 sign(i8vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i16vec4 abs(i16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i16vec4 sign(i16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns x if x >= 0; otherwise it returns -x.
i64vec4 abs(i64vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
i64vec4 sign(i64vec4 x);
#endif
// Returns a value equal to the nearest integer that is less than or equal to x.
float floor(float x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
float trunc(float x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
float round(float x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
float roundEven(float x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
float ceil(float x);
// Returns x - floor(x).
float fract(float x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
float mod(float x, float y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
float mod(float x, float y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
float modf(float x, out float i);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is less than or equal to x.
float16_t floor(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
float16_t trunc(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
float16_t round(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
float16_t roundEven(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is greater than or equal to x.
float16_t ceil(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x - floor(x).
float16_t fract(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
float16_t mod(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
float16_t mod(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
float16_t modf(float16_t x, out float16_t i);
#endif
// Returns a value equal to the nearest integer that is less than or equal to x.
double floor(double x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
double trunc(double x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
double round(double x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
double roundEven(double x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
double ceil(double x);
// Returns x - floor(x).
double fract(double x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
double mod(double x, double y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
double mod(double x, double y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
double modf(double x, out double i);
// Returns a value equal to the nearest integer that is less than or equal to x.
vec2 floor(vec2 x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
vec2 trunc(vec2 x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
vec2 round(vec2 x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
vec2 roundEven(vec2 x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
vec2 ceil(vec2 x);
// Returns x - floor(x).
vec2 fract(vec2 x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
vec2 mod(vec2 x, float y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
vec2 mod(vec2 x, vec2 y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
vec2 modf(vec2 x, out vec2 i);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is less than or equal to x.
f16vec2 floor(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
f16vec2 trunc(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
f16vec2 round(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
f16vec2 roundEven(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is greater than or equal to x.
f16vec2 ceil(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x - floor(x).
f16vec2 fract(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
f16vec2 mod(f16vec2 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
f16vec2 mod(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
f16vec2 modf(f16vec2 x, out f16vec2 i);
#endif
// Returns a value equal to the nearest integer that is less than or equal to x.
dvec2 floor(dvec2 x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
dvec2 trunc(dvec2 x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
dvec2 round(dvec2 x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
dvec2 roundEven(dvec2 x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
dvec2 ceil(dvec2 x);
// Returns x - floor(x).
dvec2 fract(dvec2 x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
dvec2 mod(dvec2 x, double y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
dvec2 mod(dvec2 x, dvec2 y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
dvec2 modf(dvec2 x, out dvec2 i);
// Returns a value equal to the nearest integer that is less than or equal to x.
vec3 floor(vec3 x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
vec3 trunc(vec3 x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
vec3 round(vec3 x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
vec3 roundEven(vec3 x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
vec3 ceil(vec3 x);
// Returns x - floor(x).
vec3 fract(vec3 x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
vec3 mod(vec3 x, float y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
vec3 mod(vec3 x, vec3 y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
vec3 modf(vec3 x, out vec3 i);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is less than or equal to x.
f16vec3 floor(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
f16vec3 trunc(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
f16vec3 round(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
f16vec3 roundEven(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is greater than or equal to x.
f16vec3 ceil(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x - floor(x).
f16vec3 fract(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
f16vec3 mod(f16vec3 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
f16vec3 mod(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
f16vec3 modf(f16vec3 x, out f16vec3 i);
#endif
// Returns a value equal to the nearest integer that is less than or equal to x.
dvec3 floor(dvec3 x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
dvec3 trunc(dvec3 x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
dvec3 round(dvec3 x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
dvec3 roundEven(dvec3 x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
dvec3 ceil(dvec3 x);
// Returns x - floor(x).
dvec3 fract(dvec3 x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
dvec3 mod(dvec3 x, double y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
dvec3 mod(dvec3 x, dvec3 y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
dvec3 modf(dvec3 x, out dvec3 i);
// Returns a value equal to the nearest integer that is less than or equal to x.
vec4 floor(vec4 x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
vec4 trunc(vec4 x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
vec4 round(vec4 x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
vec4 roundEven(vec4 x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
vec4 ceil(vec4 x);
// Returns x - floor(x).
vec4 fract(vec4 x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
vec4 mod(vec4 x, float y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
vec4 mod(vec4 x, vec4 y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
vec4 modf(vec4 x, out vec4 i);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is less than or equal to x.
f16vec4 floor(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
f16vec4 trunc(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
f16vec4 round(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
f16vec4 roundEven(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a value equal to the nearest integer that is greater than or equal to x.
f16vec4 ceil(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns x - floor(x).
f16vec4 fract(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
f16vec4 mod(f16vec4 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
f16vec4 mod(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
f16vec4 modf(f16vec4 x, out f16vec4 i);
#endif
// Returns a value equal to the nearest integer that is less than or equal to x.
dvec4 floor(dvec4 x);
// Returns a value equal to the nearest integer to x whose absolute value is not larger than the absolute value of x.
dvec4 trunc(dvec4 x);
// Returns a value equal to the nearest integer to x. The fraction 0.5 will round in a direction chosen by the implementation, presumably the direction that is fastest. This includes the possibility that round(x) returns the same value as roundEven(x) for all values of x.
dvec4 round(dvec4 x);
// Returns a value equal to the nearest integer to x. A fractional part of 0.5 will round toward the nearest even integer. (Both 3.5 and 4.5 for x will return 4.0.)
dvec4 roundEven(dvec4 x);
// Returns a value equal to the nearest integer that is greater than or equal to x.
dvec4 ceil(dvec4 x);
// Returns x - floor(x).
dvec4 fract(dvec4 x);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
dvec4 mod(dvec4 x, double y);
// Modulus. Returns x - y · floor(x / y).
// 
// Note that implementations may use a cheap approximation to the remainder, and the error can be large due to the discontinuity in floor. This can produce mathematically unexpected results in some cases, such as mod(x,x) computing x rather than 0, and can also cause the result to have a different sign than the infinitely precise result.
dvec4 mod(dvec4 x, dvec4 y);
// Returns the fractional part of x and sets i to the integer part (as a whole number floating-point value). Both the return value and the output parameter will have the same sign as x.
dvec4 modf(dvec4 x, out dvec4 i);
// Returns y if y < x; otherwise it returns x.
float min(float x, float y);
// Returns y if y < x; otherwise it returns x.
float min(float x, float y);
// Returns y if x < y; otherwise it returns x.
float max(float x, float y);
// Returns y if x < y; otherwise it returns x.
float max(float x, float y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
float clamp(float x, float minVal, float maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
float clamp(float x, float minVal, float maxVal);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
float16_t min(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
float16_t min(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
float16_t max(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
float16_t max(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
float16_t clamp(float16_t x, float16_t minVal, float16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
float16_t clamp(float16_t x, float16_t minVal, float16_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
double min(double x, double y);
// Returns y if y < x; otherwise it returns x.
double min(double x, double y);
// Returns y if x < y; otherwise it returns x.
double max(double x, double y);
// Returns y if x < y; otherwise it returns x.
double max(double x, double y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
double clamp(double x, double minVal, double maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
double clamp(double x, double minVal, double maxVal);
// Returns y if y < x; otherwise it returns x.
int min(int x, int y);
// Returns y if y < x; otherwise it returns x.
int min(int x, int y);
// Returns y if x < y; otherwise it returns x.
int max(int x, int y);
// Returns y if x < y; otherwise it returns x.
int max(int x, int y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int clamp(int x, int minVal, int maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int clamp(int x, int minVal, int maxVal);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
int8_t min(int8_t x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
int8_t min(int8_t x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
int8_t max(int8_t x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
int8_t max(int8_t x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int8_t clamp(int8_t x, int8_t minVal, int8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int8_t clamp(int8_t x, int8_t minVal, int8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
int16_t min(int16_t x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
int16_t min(int16_t x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
int16_t max(int16_t x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
int16_t max(int16_t x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int16_t clamp(int16_t x, int16_t minVal, int16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int16_t clamp(int16_t x, int16_t minVal, int16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
int64_t min(int64_t x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
int64_t min(int64_t x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
int64_t max(int64_t x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
int64_t max(int64_t x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int64_t clamp(int64_t x, int64_t minVal, int64_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
int64_t clamp(int64_t x, int64_t minVal, int64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
uint min(uint x, uint y);
// Returns y if y < x; otherwise it returns x.
uint min(uint x, uint y);
// Returns y if x < y; otherwise it returns x.
uint max(uint x, uint y);
// Returns y if x < y; otherwise it returns x.
uint max(uint x, uint y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint clamp(uint x, uint minVal, uint maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint clamp(uint x, uint minVal, uint maxVal);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
uint8_t min(uint8_t x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
uint8_t min(uint8_t x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
uint8_t max(uint8_t x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
uint8_t max(uint8_t x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint8_t clamp(uint8_t x, uint8_t minVal, uint8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint8_t clamp(uint8_t x, uint8_t minVal, uint8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
uint16_t min(uint16_t x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
uint16_t min(uint16_t x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
uint16_t max(uint16_t x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
uint16_t max(uint16_t x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint16_t clamp(uint16_t x, uint16_t minVal, uint16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint16_t clamp(uint16_t x, uint16_t minVal, uint16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
uint64_t min(uint64_t x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
uint64_t min(uint64_t x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
uint64_t max(uint64_t x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
uint64_t max(uint64_t x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint64_t clamp(uint64_t x, uint64_t minVal, uint64_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uint64_t clamp(uint64_t x, uint64_t minVal, uint64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
vec2 min(vec2 x, vec2 y);
// Returns y if y < x; otherwise it returns x.
vec2 min(vec2 x, float y);
// Returns y if x < y; otherwise it returns x.
vec2 max(vec2 x, vec2 y);
// Returns y if x < y; otherwise it returns x.
vec2 max(vec2 x, float y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
vec2 clamp(vec2 x, vec2 minVal, vec2 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
vec2 clamp(vec2 x, float minVal, float maxVal);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
f16vec2 min(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
f16vec2 min(f16vec2 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
f16vec2 max(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
f16vec2 max(f16vec2 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
f16vec2 clamp(f16vec2 x, f16vec2 minVal, f16vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
f16vec2 clamp(f16vec2 x, float16_t minVal, float16_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
dvec2 min(dvec2 x, dvec2 y);
// Returns y if y < x; otherwise it returns x.
dvec2 min(dvec2 x, double y);
// Returns y if x < y; otherwise it returns x.
dvec2 max(dvec2 x, dvec2 y);
// Returns y if x < y; otherwise it returns x.
dvec2 max(dvec2 x, double y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
dvec2 clamp(dvec2 x, dvec2 minVal, dvec2 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
dvec2 clamp(dvec2 x, double minVal, double maxVal);
// Returns y if y < x; otherwise it returns x.
ivec2 min(ivec2 x, ivec2 y);
// Returns y if y < x; otherwise it returns x.
ivec2 min(ivec2 x, int y);
// Returns y if x < y; otherwise it returns x.
ivec2 max(ivec2 x, ivec2 y);
// Returns y if x < y; otherwise it returns x.
ivec2 max(ivec2 x, int y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
ivec2 clamp(ivec2 x, ivec2 minVal, ivec2 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
ivec2 clamp(ivec2 x, int minVal, int maxVal);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
i8vec2 min(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
i8vec2 min(i8vec2 x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
i8vec2 max(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
i8vec2 max(i8vec2 x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i8vec2 clamp(i8vec2 x, i8vec2 minVal, i8vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i8vec2 clamp(i8vec2 x, int8_t minVal, int8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
i16vec2 min(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
i16vec2 min(i16vec2 x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
i16vec2 max(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
i16vec2 max(i16vec2 x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i16vec2 clamp(i16vec2 x, i16vec2 minVal, i16vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i16vec2 clamp(i16vec2 x, int16_t minVal, int16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
i64vec2 min(i64vec2 x, i64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
i64vec2 min(i64vec2 x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
i64vec2 max(i64vec2 x, i64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
i64vec2 max(i64vec2 x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i64vec2 clamp(i64vec2 x, i64vec2 minVal, i64vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i64vec2 clamp(i64vec2 x, int64_t minVal, int64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
uvec2 min(uvec2 x, uvec2 y);
// Returns y if y < x; otherwise it returns x.
uvec2 min(uvec2 x, uint y);
// Returns y if x < y; otherwise it returns x.
uvec2 max(uvec2 x, uvec2 y);
// Returns y if x < y; otherwise it returns x.
uvec2 max(uvec2 x, uint y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uvec2 clamp(uvec2 x, uvec2 minVal, uvec2 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uvec2 clamp(uvec2 x, uint minVal, uint maxVal);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
u8vec2 min(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
u8vec2 min(u8vec2 x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
u8vec2 max(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
u8vec2 max(u8vec2 x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u8vec2 clamp(u8vec2 x, u8vec2 minVal, u8vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u8vec2 clamp(u8vec2 x, uint8_t minVal, uint8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
u16vec2 min(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
u16vec2 min(u16vec2 x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
u16vec2 max(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
u16vec2 max(u16vec2 x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u16vec2 clamp(u16vec2 x, u16vec2 minVal, u16vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u16vec2 clamp(u16vec2 x, uint16_t minVal, uint16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
u64vec2 min(u64vec2 x, u64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
u64vec2 min(u64vec2 x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
u64vec2 max(u64vec2 x, u64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
u64vec2 max(u64vec2 x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u64vec2 clamp(u64vec2 x, u64vec2 minVal, u64vec2 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u64vec2 clamp(u64vec2 x, uint64_t minVal, uint64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
vec3 min(vec3 x, vec3 y);
// Returns y if y < x; otherwise it returns x.
vec3 min(vec3 x, float y);
// Returns y if x < y; otherwise it returns x.
vec3 max(vec3 x, vec3 y);
// Returns y if x < y; otherwise it returns x.
vec3 max(vec3 x, float y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
vec3 clamp(vec3 x, vec3 minVal, vec3 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
vec3 clamp(vec3 x, float minVal, float maxVal);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
f16vec3 min(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
f16vec3 min(f16vec3 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
f16vec3 max(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
f16vec3 max(f16vec3 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
f16vec3 clamp(f16vec3 x, f16vec3 minVal, f16vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
f16vec3 clamp(f16vec3 x, float16_t minVal, float16_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
dvec3 min(dvec3 x, dvec3 y);
// Returns y if y < x; otherwise it returns x.
dvec3 min(dvec3 x, double y);
// Returns y if x < y; otherwise it returns x.
dvec3 max(dvec3 x, dvec3 y);
// Returns y if x < y; otherwise it returns x.
dvec3 max(dvec3 x, double y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
dvec3 clamp(dvec3 x, dvec3 minVal, dvec3 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
dvec3 clamp(dvec3 x, double minVal, double maxVal);
// Returns y if y < x; otherwise it returns x.
ivec3 min(ivec3 x, ivec3 y);
// Returns y if y < x; otherwise it returns x.
ivec3 min(ivec3 x, int y);
// Returns y if x < y; otherwise it returns x.
ivec3 max(ivec3 x, ivec3 y);
// Returns y if x < y; otherwise it returns x.
ivec3 max(ivec3 x, int y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
ivec3 clamp(ivec3 x, ivec3 minVal, ivec3 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
ivec3 clamp(ivec3 x, int minVal, int maxVal);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
i8vec3 min(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
i8vec3 min(i8vec3 x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
i8vec3 max(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
i8vec3 max(i8vec3 x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i8vec3 clamp(i8vec3 x, i8vec3 minVal, i8vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i8vec3 clamp(i8vec3 x, int8_t minVal, int8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
i16vec3 min(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
i16vec3 min(i16vec3 x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
i16vec3 max(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
i16vec3 max(i16vec3 x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i16vec3 clamp(i16vec3 x, i16vec3 minVal, i16vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i16vec3 clamp(i16vec3 x, int16_t minVal, int16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
i64vec3 min(i64vec3 x, i64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
i64vec3 min(i64vec3 x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
i64vec3 max(i64vec3 x, i64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
i64vec3 max(i64vec3 x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i64vec3 clamp(i64vec3 x, i64vec3 minVal, i64vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i64vec3 clamp(i64vec3 x, int64_t minVal, int64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
uvec3 min(uvec3 x, uvec3 y);
// Returns y if y < x; otherwise it returns x.
uvec3 min(uvec3 x, uint y);
// Returns y if x < y; otherwise it returns x.
uvec3 max(uvec3 x, uvec3 y);
// Returns y if x < y; otherwise it returns x.
uvec3 max(uvec3 x, uint y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uvec3 clamp(uvec3 x, uvec3 minVal, uvec3 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uvec3 clamp(uvec3 x, uint minVal, uint maxVal);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
u8vec3 min(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
u8vec3 min(u8vec3 x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
u8vec3 max(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
u8vec3 max(u8vec3 x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u8vec3 clamp(u8vec3 x, u8vec3 minVal, u8vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u8vec3 clamp(u8vec3 x, uint8_t minVal, uint8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
u16vec3 min(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
u16vec3 min(u16vec3 x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
u16vec3 max(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
u16vec3 max(u16vec3 x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u16vec3 clamp(u16vec3 x, u16vec3 minVal, u16vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u16vec3 clamp(u16vec3 x, uint16_t minVal, uint16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
u64vec3 min(u64vec3 x, u64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
u64vec3 min(u64vec3 x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
u64vec3 max(u64vec3 x, u64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
u64vec3 max(u64vec3 x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u64vec3 clamp(u64vec3 x, u64vec3 minVal, u64vec3 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u64vec3 clamp(u64vec3 x, uint64_t minVal, uint64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
vec4 min(vec4 x, vec4 y);
// Returns y if y < x; otherwise it returns x.
vec4 min(vec4 x, float y);
// Returns y if x < y; otherwise it returns x.
vec4 max(vec4 x, vec4 y);
// Returns y if x < y; otherwise it returns x.
vec4 max(vec4 x, float y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
vec4 clamp(vec4 x, vec4 minVal, vec4 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
vec4 clamp(vec4 x, float minVal, float maxVal);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
f16vec4 min(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if y < x; otherwise it returns x.
f16vec4 min(f16vec4 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
f16vec4 max(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns y if x < y; otherwise it returns x.
f16vec4 max(f16vec4 x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
f16vec4 clamp(f16vec4 x, f16vec4 minVal, f16vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
f16vec4 clamp(f16vec4 x, float16_t minVal, float16_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
dvec4 min(dvec4 x, dvec4 y);
// Returns y if y < x; otherwise it returns x.
dvec4 min(dvec4 x, double y);
// Returns y if x < y; otherwise it returns x.
dvec4 max(dvec4 x, dvec4 y);
// Returns y if x < y; otherwise it returns x.
dvec4 max(dvec4 x, double y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
dvec4 clamp(dvec4 x, dvec4 minVal, dvec4 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
dvec4 clamp(dvec4 x, double minVal, double maxVal);
// Returns y if y < x; otherwise it returns x.
ivec4 min(ivec4 x, ivec4 y);
// Returns y if y < x; otherwise it returns x.
ivec4 min(ivec4 x, int y);
// Returns y if x < y; otherwise it returns x.
ivec4 max(ivec4 x, ivec4 y);
// Returns y if x < y; otherwise it returns x.
ivec4 max(ivec4 x, int y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
ivec4 clamp(ivec4 x, ivec4 minVal, ivec4 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
ivec4 clamp(ivec4 x, int minVal, int maxVal);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
i8vec4 min(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if y < x; otherwise it returns x.
i8vec4 min(i8vec4 x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
i8vec4 max(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns y if x < y; otherwise it returns x.
i8vec4 max(i8vec4 x, int8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i8vec4 clamp(i8vec4 x, i8vec4 minVal, i8vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i8vec4 clamp(i8vec4 x, int8_t minVal, int8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
i16vec4 min(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if y < x; otherwise it returns x.
i16vec4 min(i16vec4 x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
i16vec4 max(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns y if x < y; otherwise it returns x.
i16vec4 max(i16vec4 x, int16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i16vec4 clamp(i16vec4 x, i16vec4 minVal, i16vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i16vec4 clamp(i16vec4 x, int16_t minVal, int16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
i64vec4 min(i64vec4 x, i64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if y < x; otherwise it returns x.
i64vec4 min(i64vec4 x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
i64vec4 max(i64vec4 x, i64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns y if x < y; otherwise it returns x.
i64vec4 max(i64vec4 x, int64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i64vec4 clamp(i64vec4 x, i64vec4 minVal, i64vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
i64vec4 clamp(i64vec4 x, int64_t minVal, int64_t maxVal);
#endif
// Returns y if y < x; otherwise it returns x.
uvec4 min(uvec4 x, uvec4 y);
// Returns y if y < x; otherwise it returns x.
uvec4 min(uvec4 x, uint y);
// Returns y if x < y; otherwise it returns x.
uvec4 max(uvec4 x, uvec4 y);
// Returns y if x < y; otherwise it returns x.
uvec4 max(uvec4 x, uint y);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uvec4 clamp(uvec4 x, uvec4 minVal, uvec4 maxVal);
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
uvec4 clamp(uvec4 x, uint minVal, uint maxVal);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
u8vec4 min(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if y < x; otherwise it returns x.
u8vec4 min(u8vec4 x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
u8vec4 max(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns y if x < y; otherwise it returns x.
u8vec4 max(u8vec4 x, uint8_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u8vec4 clamp(u8vec4 x, u8vec4 minVal, u8vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u8vec4 clamp(u8vec4 x, uint8_t minVal, uint8_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
u16vec4 min(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if y < x; otherwise it returns x.
u16vec4 min(u16vec4 x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
u16vec4 max(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns y if x < y; otherwise it returns x.
u16vec4 max(u16vec4 x, uint16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u16vec4 clamp(u16vec4 x, u16vec4 minVal, u16vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u16vec4 clamp(u16vec4 x, uint16_t minVal, uint16_t maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
u64vec4 min(u64vec4 x, u64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if y < x; otherwise it returns x.
u64vec4 min(u64vec4 x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
u64vec4 max(u64vec4 x, u64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns y if x < y; otherwise it returns x.
u64vec4 max(u64vec4 x, uint64_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u64vec4 clamp(u64vec4 x, u64vec4 minVal, u64vec4 maxVal);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns min(max(x, minVal), maxVal). Results are undefined if minVal > maxVal.
u64vec4 clamp(u64vec4 x, uint64_t minVal, uint64_t maxVal);
#endif
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
float mix(float x, float y, float a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
float mix(float x, float y, float a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
float step(float edge, float x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
float step(float edge, float x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
float smoothstep(float edge0, float edge1, float x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
float smoothstep(float edge0, float edge1, float x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
float16_t mix(float16_t x, float16_t y, float16_t a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
float16_t mix(float16_t x, float16_t y, float16_t a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
float16_t step(float16_t edge, float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
float16_t step(float16_t edge, float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
float16_t smoothstep(float16_t edge0, float16_t edge1, float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
float16_t smoothstep(float16_t edge0, float16_t edge1, float16_t x);
#endif
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
double mix(double x, double y, double a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
double mix(double x, double y, double a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
double step(double edge, double x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
double step(double edge, double x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
double smoothstep(double edge0, double edge1, double x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
double smoothstep(double edge0, double edge1, double x);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
vec2 mix(vec2 x, vec2 y, vec2 a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
vec2 mix(vec2 x, vec2 y, float a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
vec2 step(vec2 edge, vec2 x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
vec2 step(float edge, vec2 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
vec2 smoothstep(vec2 edge0, vec2 edge1, vec2 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
vec2 smoothstep(float edge0, float edge1, vec2 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
f16vec2 mix(f16vec2 x, f16vec2 y, f16vec2 a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
f16vec2 mix(f16vec2 x, f16vec2 y, float16_t a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
f16vec2 step(f16vec2 edge, f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
f16vec2 step(float16_t edge, f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
f16vec2 smoothstep(f16vec2 edge0, f16vec2 edge1, f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
f16vec2 smoothstep(float16_t edge0, float16_t edge1, f16vec2 x);
#endif
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
dvec2 mix(dvec2 x, dvec2 y, dvec2 a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
dvec2 mix(dvec2 x, dvec2 y, double a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
dvec2 step(dvec2 edge, dvec2 x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
dvec2 step(double edge, dvec2 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
dvec2 smoothstep(dvec2 edge0, dvec2 edge1, dvec2 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
dvec2 smoothstep(double edge0, double edge1, dvec2 x);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
vec3 mix(vec3 x, vec3 y, vec3 a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
vec3 mix(vec3 x, vec3 y, float a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
vec3 step(vec3 edge, vec3 x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
vec3 step(float edge, vec3 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
vec3 smoothstep(vec3 edge0, vec3 edge1, vec3 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
vec3 smoothstep(float edge0, float edge1, vec3 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
f16vec3 mix(f16vec3 x, f16vec3 y, f16vec3 a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
f16vec3 mix(f16vec3 x, f16vec3 y, float16_t a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
f16vec3 step(f16vec3 edge, f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
f16vec3 step(float16_t edge, f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
f16vec3 smoothstep(f16vec3 edge0, f16vec3 edge1, f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
f16vec3 smoothstep(float16_t edge0, float16_t edge1, f16vec3 x);
#endif
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
dvec3 mix(dvec3 x, dvec3 y, dvec3 a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
dvec3 mix(dvec3 x, dvec3 y, double a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
dvec3 step(dvec3 edge, dvec3 x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
dvec3 step(double edge, dvec3 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
dvec3 smoothstep(dvec3 edge0, dvec3 edge1, dvec3 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
dvec3 smoothstep(double edge0, double edge1, dvec3 x);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
vec4 mix(vec4 x, vec4 y, vec4 a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
vec4 mix(vec4 x, vec4 y, float a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
vec4 step(vec4 edge, vec4 x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
vec4 step(float edge, vec4 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
vec4 smoothstep(vec4 edge0, vec4 edge1, vec4 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
vec4 smoothstep(float edge0, float edge1, vec4 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
f16vec4 mix(f16vec4 x, f16vec4 y, f16vec4 a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
f16vec4 mix(f16vec4 x, f16vec4 y, float16_t a);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
f16vec4 step(f16vec4 edge, f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x < edge; otherwise it returns 1.0.
f16vec4 step(float16_t edge, f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
f16vec4 smoothstep(f16vec4 edge0, f16vec4 edge1, f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
f16vec4 smoothstep(float16_t edge0, float16_t edge1, f16vec4 x);
#endif
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
dvec4 mix(dvec4 x, dvec4 y, dvec4 a);
// Returns the linear blend of x and y, i.e., x * (1 - a) + y * a.
dvec4 mix(dvec4 x, dvec4 y, double a);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
dvec4 step(dvec4 edge, dvec4 x);
// Returns 0.0 if x < edge; otherwise it returns 1.0.
dvec4 step(double edge, dvec4 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
dvec4 smoothstep(dvec4 edge0, dvec4 edge1, dvec4 x);
// Returns 0.0 if x ≤ edge0 and 1.0 if x ≥ edge1, and performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1. This is useful in cases where you would want a threshold function with a smooth transition. This is equivalent to:
// 
// ```
// genFType t;
// t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// ```
// 
// (And similarly for doubles.) Results are undefined if edge0 ≥ edge1.
dvec4 smoothstep(double edge0, double edge1, dvec4 x);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
float mix(float x, bool y, bool a);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
float16_t mix(float16_t x, bool y, bool a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
double mix(double x, bool y, bool a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
int mix(int x, bool y, bool a);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
int8_t mix(int8_t x, bool y, bool a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
int16_t mix(int16_t x, bool y, bool a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
int64_t mix(int64_t x, bool y, bool a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uint mix(uint x, bool y, bool a);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uint8_t mix(uint8_t x, bool y, bool a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uint16_t mix(uint16_t x, bool y, bool a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uint64_t mix(uint64_t x, bool y, bool a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
bool mix(bool x, bool y, bool a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
vec2 mix(vec2 x, bvec2 y, bvec2 a);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
f16vec2 mix(f16vec2 x, bvec2 y, bvec2 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
dvec2 mix(dvec2 x, bvec2 y, bvec2 a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
ivec2 mix(ivec2 x, bvec2 y, bvec2 a);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i8vec2 mix(i8vec2 x, bvec2 y, bvec2 a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i16vec2 mix(i16vec2 x, bvec2 y, bvec2 a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i64vec2 mix(i64vec2 x, bvec2 y, bvec2 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uvec2 mix(uvec2 x, bvec2 y, bvec2 a);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u8vec2 mix(u8vec2 x, bvec2 y, bvec2 a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u16vec2 mix(u16vec2 x, bvec2 y, bvec2 a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u64vec2 mix(u64vec2 x, bvec2 y, bvec2 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
bvec2 mix(bvec2 x, bvec2 y, bvec2 a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
vec3 mix(vec3 x, bvec3 y, bvec3 a);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
f16vec3 mix(f16vec3 x, bvec3 y, bvec3 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
dvec3 mix(dvec3 x, bvec3 y, bvec3 a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
ivec3 mix(ivec3 x, bvec3 y, bvec3 a);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i8vec3 mix(i8vec3 x, bvec3 y, bvec3 a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i16vec3 mix(i16vec3 x, bvec3 y, bvec3 a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i64vec3 mix(i64vec3 x, bvec3 y, bvec3 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uvec3 mix(uvec3 x, bvec3 y, bvec3 a);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u8vec3 mix(u8vec3 x, bvec3 y, bvec3 a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u16vec3 mix(u16vec3 x, bvec3 y, bvec3 a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u64vec3 mix(u64vec3 x, bvec3 y, bvec3 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
bvec3 mix(bvec3 x, bvec3 y, bvec3 a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
vec4 mix(vec4 x, bvec4 y, bvec4 a);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
f16vec4 mix(f16vec4 x, bvec4 y, bvec4 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
dvec4 mix(dvec4 x, bvec4 y, bvec4 a);
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
ivec4 mix(ivec4 x, bvec4 y, bvec4 a);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i8vec4 mix(i8vec4 x, bvec4 y, bvec4 a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i16vec4 mix(i16vec4 x, bvec4 y, bvec4 a);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
i64vec4 mix(i64vec4 x, bvec4 y, bvec4 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
uvec4 mix(uvec4 x, bvec4 y, bvec4 a);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u8vec4 mix(u8vec4 x, bvec4 y, bvec4 a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u16vec4 mix(u16vec4 x, bvec4 y, bvec4 a);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
u64vec4 mix(u64vec4 x, bvec4 y, bvec4 a);
#endif
// Selects which vector each returned component comes from. For a component of a that is false, the corresponding component of x is returned. For a component of a that is true, the corresponding component of y is returned. Components of x and y that are not selected are allowed to be invalid floating-point values and will have no effect on the results. Thus, this provides different functionality than, for example, `genFType mix(genFType x, genFType y, genFType(a))` where a is a Boolean vector.
bvec4 mix(bvec4 x, bvec4 y, bvec4 a);
// Returns true if x holds a NaN. Returns false otherwise.
bool isnan(float x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bool isinf(float x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a NaN. Returns false otherwise.
bool isnan(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bool isinf(float16_t x);
#endif
// Returns true if x holds a NaN. Returns false otherwise.
bool isnan(double x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bool isinf(double x);
// Returns true if x holds a NaN. Returns false otherwise.
bvec2 isnan(vec2 x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec2 isinf(vec2 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a NaN. Returns false otherwise.
bvec2 isnan(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec2 isinf(f16vec2 x);
#endif
// Returns true if x holds a NaN. Returns false otherwise.
bvec2 isnan(dvec2 x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec2 isinf(dvec2 x);
// Returns true if x holds a NaN. Returns false otherwise.
bvec3 isnan(vec3 x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec3 isinf(vec3 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a NaN. Returns false otherwise.
bvec3 isnan(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec3 isinf(f16vec3 x);
#endif
// Returns true if x holds a NaN. Returns false otherwise.
bvec3 isnan(dvec3 x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec3 isinf(dvec3 x);
// Returns true if x holds a NaN. Returns false otherwise.
bvec4 isnan(vec4 x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec4 isinf(vec4 x);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a NaN. Returns false otherwise.
bvec4 isnan(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec4 isinf(f16vec4 x);
#endif
// Returns true if x holds a NaN. Returns false otherwise.
bvec4 isnan(dvec4 x);
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
bvec4 isinf(dvec4 x);
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
int floatBitsToInt(highp float value);
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uint floatBitsToUint(highp float value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
float intBitsToFloat(highp int value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
float uintBitsToFloat(highp uint value);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
int floatBitsToInt(highp float16_t value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uint floatBitsToUint(highp float16_t value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
float16_t intBitsToFloat(highp int value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
float16_t uintBitsToFloat(highp uint value);
#endif
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
ivec2 floatBitsToInt(highp vec2 value);
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uvec2 floatBitsToUint(highp vec2 value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
vec2 intBitsToFloat(highp ivec2 value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
vec2 uintBitsToFloat(highp uvec2 value);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
ivec2 floatBitsToInt(highp f16vec2 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uvec2 floatBitsToUint(highp f16vec2 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
f16vec2 intBitsToFloat(highp ivec2 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
f16vec2 uintBitsToFloat(highp uvec2 value);
#endif
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
ivec3 floatBitsToInt(highp vec3 value);
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uvec3 floatBitsToUint(highp vec3 value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
vec3 intBitsToFloat(highp ivec3 value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
vec3 uintBitsToFloat(highp uvec3 value);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
ivec3 floatBitsToInt(highp f16vec3 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uvec3 floatBitsToUint(highp f16vec3 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
f16vec3 intBitsToFloat(highp ivec3 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
f16vec3 uintBitsToFloat(highp uvec3 value);
#endif
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
ivec4 floatBitsToInt(highp vec4 value);
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uvec4 floatBitsToUint(highp vec4 value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
vec4 intBitsToFloat(highp ivec4 value);
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
vec4 uintBitsToFloat(highp uvec4 value);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a signed integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
ivec4 floatBitsToInt(highp f16vec4 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns an unsigned integer value representing the encoding of a floating-point value. The float value's bit-level representation is preserved.
uvec4 floatBitsToUint(highp f16vec4 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
f16vec4 intBitsToFloat(highp ivec4 value);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a floating-point value. If a NaN is passed in, it will not signal, and the resulting value is unspecified. If an Inf is passed in, the resulting value is the corresponding Inf. If a subnormal number is passed in, the result might be flushed to 0. Otherwise, the bit-level representation is preserved.
f16vec4 uintBitsToFloat(highp uvec4 value);
#endif
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
float fma(float a, float b, float c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
float frexp(highp float x, out highp int exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
float ldexp(highp float x, highp int exp);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
float16_t fma(float16_t a, float16_t b, float16_t c);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
float16_t frexp(highp float16_t x, out highp int exp);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
float16_t ldexp(highp float16_t x, highp int exp);
#endif
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
double fma(double a, double b, double c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
double frexp(highp double x, out highp int exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
double ldexp(highp double x, highp int exp);
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
vec2 fma(vec2 a, vec2 b, vec2 c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
vec2 frexp(highp vec2 x, out highp ivec2 exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
vec2 ldexp(highp vec2 x, highp ivec2 exp);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
f16vec2 fma(f16vec2 a, f16vec2 b, f16vec2 c);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
f16vec2 frexp(highp f16vec2 x, out highp ivec2 exp);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
f16vec2 ldexp(highp f16vec2 x, highp ivec2 exp);
#endif
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
dvec2 fma(dvec2 a, dvec2 b, dvec2 c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
dvec2 frexp(highp dvec2 x, out highp ivec2 exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
dvec2 ldexp(highp dvec2 x, highp ivec2 exp);
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
vec3 fma(vec3 a, vec3 b, vec3 c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
vec3 frexp(highp vec3 x, out highp ivec3 exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
vec3 ldexp(highp vec3 x, highp ivec3 exp);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
f16vec3 fma(f16vec3 a, f16vec3 b, f16vec3 c);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
f16vec3 frexp(highp f16vec3 x, out highp ivec3 exp);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
f16vec3 ldexp(highp f16vec3 x, highp ivec3 exp);
#endif
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
dvec3 fma(dvec3 a, dvec3 b, dvec3 c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
dvec3 frexp(highp dvec3 x, out highp ivec3 exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
dvec3 ldexp(highp dvec3 x, highp ivec3 exp);
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
vec4 fma(vec4 a, vec4 b, vec4 c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
vec4 frexp(highp vec4 x, out highp ivec4 exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
vec4 ldexp(highp vec4 x, highp ivec4 exp);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
f16vec4 fma(f16vec4 a, f16vec4 b, f16vec4 c);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
f16vec4 frexp(highp f16vec4 x, out highp ivec4 exp);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
f16vec4 ldexp(highp f16vec4 x, highp ivec4 exp);
#endif
// Computes and returns a * b + c. In uses where the return value is eventually consumed by a variable declared as precise:
// 
// - fma() is considered a single operation, whereas the expression a * b + c consumed by a variable declared precise is considered two operations.
// - The precision of fma() can differ from the precision of the expression a * b + c.
// - fma() will be computed with the same precision as any other fma() consumed by a precise variable, giving invariant results for the same input values of a, b, and c.
// 
// Otherwise, in the absence of precise consumption, there are no special constraints on the number of operations or difference in precision between fma() and the expression a * b + c.
dvec4 fma(dvec4 a, dvec4 b, dvec4 c);
// Splits x into a floating-point significand in the range [0.5,1.0], and an integral exponent of two, such that
// 
// x = significant · 2exponent
// 
// The significand is returned by the function and the exponent is returned in the parameter exp. For a floating-point value of zero, the significand and exponent are both zero.
// 
// If an implementation supports signed zero, an input value of minus zero should return a significand of minus zero. For a floating-point value that is an infinity or is not a number, the results are undefined.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value returned by the function and the value written to exp are vectors with the same number of components as x.
dvec4 frexp(highp dvec4 x, out highp ivec4 exp);
// Builds a floating-point number from x and the corresponding integral exponent of two in exp, returning:
// 
// significand · 2exponent
// 
// If this product is too large to be represented in the floating-point type, the result is undefined.
// 
// If exp is greater than +128 (single-precision) or +1024 (double-precision), the value returned is undefined. If exp is less than -126 (single-precision) or -1022 (double-precision), the value returned may be flushed to zero. Additionally, splitting the value into a significand and exponent using frexp() and then reconstructing a floating-point value using ldexp() should yield the original input for zero and all finite non-subnormal values.
// 
// If the input x is a vector, this operation is performed in a component-wise manner; the value passed in exp and returned by the function are vectors with the same number of components as x.
dvec4 ldexp(highp dvec4 x, highp ivec4 exp);
// First, converts each component of the normalized floating-point value *v* into 16-bit (**2x16**) or 8-bit (**4x8**) integer values. Then, the results are packed into the returned 32-bit unsigned integer.
// 
// The conversion for component *c* of *v* to fixed point is done as follows:
// - **packUnorm2x16**: `round(clamp(c, 0, +1) * 65535.0)`
// - **packSnorm2x16**: `round(clamp(c, -1, +1) * 32767.0)`
// - **packUnorm4x8**: `round(clamp(c, 0, +1) * 255.0)`
// - **packSnorm4x8**: `round(clamp(c, -1, +1) * 127.0)`
// 
// The first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.
highp uint packUnorm2x16(vec2 v);
// First, converts each component of the normalized floating-point value *v* into 16-bit (**2x16**) or 8-bit (**4x8**) integer values. Then, the results are packed into the returned 32-bit unsigned integer.
// 
// The conversion for component *c* of *v* to fixed point is done as follows:
// - **packUnorm2x16**: `round(clamp(c, 0, +1) * 65535.0)`
// - **packSnorm2x16**: `round(clamp(c, -1, +1) * 32767.0)`
// - **packUnorm4x8**: `round(clamp(c, 0, +1) * 255.0)`
// - **packSnorm4x8**: `round(clamp(c, -1, +1) * 127.0)`
// 
// The first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.
highp uint packSnorm2x16(vec2 v);
// First, converts each component of the normalized floating-point value *v* into 16-bit (**2x16**) or 8-bit (**4x8**) integer values. Then, the results are packed into the returned 32-bit unsigned integer.
// 
// The conversion for component *c* of *v* to fixed point is done as follows:
// - **packUnorm2x16**: `round(clamp(c, 0, +1) * 65535.0)`
// - **packSnorm2x16**: `round(clamp(c, -1, +1) * 32767.0)`
// - **packUnorm4x8**: `round(clamp(c, 0, +1) * 255.0)`
// - **packSnorm4x8**: `round(clamp(c, -1, +1) * 127.0)`
// 
// The first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.
uint packUnorm4x8(vec4 v);
// First, converts each component of the normalized floating-point value *v* into 16-bit (**2x16**) or 8-bit (**4x8**) integer values. Then, the results are packed into the returned 32-bit unsigned integer.
// 
// The conversion for component *c* of *v* to fixed point is done as follows:
// - **packUnorm2x16**: `round(clamp(c, 0, +1) * 65535.0)`
// - **packSnorm2x16**: `round(clamp(c, -1, +1) * 32767.0)`
// - **packUnorm4x8**: `round(clamp(c, 0, +1) * 255.0)`
// - **packSnorm4x8**: `round(clamp(c, -1, +1) * 127.0)`
// 
// The first component of the vector will be written to the least significant bits of the output; the last component will be written to the most significant bits.
uint packSnorm4x8(vec4 v);
// First, unpacks a single 32-bit unsigned integer *p* into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
// 
// The conversion for unpacked fixed-point value *f* to floating-point is done as follows:
// - **unpackUnorm2x16**: `f / 65535.0`
// - **unpackSnorm2x16**: `clamp(f / 32767.0, -1, +1)`
// - **unpackUnorm4x8**: `f / 255.0`
// - **unpackSnorm4x8**: `clamp(f / 127.0, -1, +1)`
// 
// The first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.
vec2 unpackUnorm2x16(highp uint p);
// First, unpacks a single 32-bit unsigned integer *p* into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
// 
// The conversion for unpacked fixed-point value *f* to floating-point is done as follows:
// - **unpackUnorm2x16**: `f / 65535.0`
// - **unpackSnorm2x16**: `clamp(f / 32767.0, -1, +1)`
// - **unpackUnorm4x8**: `f / 255.0`
// - **unpackSnorm4x8**: `clamp(f / 127.0, -1, +1)`
// 
// The first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.
vec2 unpackSnorm2x16(highp uint p);
// First, unpacks a single 32-bit unsigned integer *p* into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
// 
// The conversion for unpacked fixed-point value *f* to floating-point is done as follows:
// - **unpackUnorm2x16**: `f / 65535.0`
// - **unpackSnorm2x16**: `clamp(f / 32767.0, -1, +1)`
// - **unpackUnorm4x8**: `f / 255.0`
// - **unpackSnorm4x8**: `clamp(f / 127.0, -1, +1)`
// 
// The first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.
vec4 unpackUnorm4x8(highp uint p);
// First, unpacks a single 32-bit unsigned integer *p* into a pair of 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit unsigned integers, or four 8-bit signed integers, respectively. Then, each component is converted to a normalized floating-point value to generate the returned two- or four-component vector.
// 
// The conversion for unpacked fixed-point value *f* to floating-point is done as follows:
// - **unpackUnorm2x16**: `f / 65535.0`
// - **unpackSnorm2x16**: `clamp(f / 32767.0, -1, +1)`
// - **unpackUnorm4x8**: `f / 255.0`
// - **unpackSnorm4x8**: `clamp(f / 127.0, -1, +1)`
// 
// The first component of the returned vector will be extracted from the least significant bits of the input; the last component will be extracted from the most significant bits.
vec4 unpackSnorm4x8(highp uint p);
// Returns an unsigned integer obtained by converting the components of a two-component floating-point vector to the 16-bit floating-point representation of the API, and then packing these two 16-bit integers into a 32-bit unsigned integer.
// 
// The first vector component specifies the 16 least-significant bits of the result; the second component specifies the 16 most-significant bits.
uint packHalf2x16(vec2 v);
// Returns a two-component floating-point vector with components obtained by unpacking a 32-bit unsigned integer into a pair of 16-bit values, interpreting those values as 16-bit floating-point numbers according to the API, and converting them to 32-bit floating-point values.
// 
// The first component of the vector is obtained from the 16 least-significant bits of *v*; the second component is obtained from the 16 most-significant bits of *v*.
vec2 unpackHalf2x16(uint v);
// Returns a double-precision value obtained by packing the components of *v* into a 64-bit value. If an IEEE 754 Inf or NaN is created, it will not signal, and the resulting floating-point value is unspecified. Otherwise, the bit-level representation of *v* is preserved. The first vector component specifies the 32 least significant bits; the second component specifies the 32 most significant bits.
double packDouble2x32(uvec2 v);
// Returns a two-component unsigned integer vector representation of *v*. The bit-level representation of *v* is preserved. The first component of the vector contains the 32 least significant bits of the double; the second component consists of the 32 most significant bits.
uvec2 unpackDouble2x32(double v);
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float length(float x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float distance(float p0, float p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float dot(float x, float y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
float normalize(float x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
float faceforward(float N, float I, float Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
float reflect(float I, float N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
float refract(float I, float N, float eta);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float16_t length(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float16_t distance(float16_t p0, float16_t p1);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float16_t dot(float16_t x, float16_t y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
float16_t normalize(float16_t x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// If dot(Nref, I) < 0 return N, otherwise return -N.
float16_t faceforward(float16_t N, float16_t I, float16_t Nref);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
float16_t reflect(float16_t I, float16_t N);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
float16_t refract(float16_t I, float16_t N, float16_t eta);
#endif
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
double length(double x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
double distance(double p0, double p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
double dot(double x, double y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
double normalize(double x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
double faceforward(double N, double I, double Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
double reflect(double I, double N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
double refract(double I, double N, double eta);
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float length(vec2 x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float distance(vec2 p0, vec2 p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float dot(vec2 x, vec2 y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
vec2 normalize(vec2 x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
vec2 faceforward(vec2 N, vec2 I, vec2 Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
vec2 reflect(vec2 I, vec2 N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
vec2 refract(vec2 I, vec2 N, float eta);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float16_t length(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float16_t distance(f16vec2 p0, f16vec2 p1);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float16_t dot(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
f16vec2 normalize(f16vec2 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// If dot(Nref, I) < 0 return N, otherwise return -N.
f16vec2 faceforward(f16vec2 N, f16vec2 I, f16vec2 Nref);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
f16vec2 reflect(f16vec2 I, f16vec2 N);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
f16vec2 refract(f16vec2 I, f16vec2 N, float16_t eta);
#endif
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
double length(dvec2 x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
double distance(dvec2 p0, dvec2 p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
double dot(dvec2 x, dvec2 y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
dvec2 normalize(dvec2 x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
dvec2 faceforward(dvec2 N, dvec2 I, dvec2 Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
dvec2 reflect(dvec2 I, dvec2 N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
dvec2 refract(dvec2 I, dvec2 N, double eta);
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float length(vec3 x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float distance(vec3 p0, vec3 p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float dot(vec3 x, vec3 y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
vec3 normalize(vec3 x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
vec3 faceforward(vec3 N, vec3 I, vec3 Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
vec3 reflect(vec3 I, vec3 N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
vec3 refract(vec3 I, vec3 N, float eta);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float16_t length(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float16_t distance(f16vec3 p0, f16vec3 p1);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float16_t dot(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
f16vec3 normalize(f16vec3 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// If dot(Nref, I) < 0 return N, otherwise return -N.
f16vec3 faceforward(f16vec3 N, f16vec3 I, f16vec3 Nref);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
f16vec3 reflect(f16vec3 I, f16vec3 N);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
f16vec3 refract(f16vec3 I, f16vec3 N, float16_t eta);
#endif
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
double length(dvec3 x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
double distance(dvec3 p0, dvec3 p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
double dot(dvec3 x, dvec3 y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
dvec3 normalize(dvec3 x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
dvec3 faceforward(dvec3 N, dvec3 I, dvec3 Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
dvec3 reflect(dvec3 I, dvec3 N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
dvec3 refract(dvec3 I, dvec3 N, double eta);
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float length(vec4 x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float distance(vec4 p0, vec4 p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float dot(vec4 x, vec4 y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
vec4 normalize(vec4 x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
vec4 faceforward(vec4 N, vec4 I, vec4 Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
vec4 reflect(vec4 I, vec4 N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
vec4 refract(vec4 I, vec4 N, float eta);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
float16_t length(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
float16_t distance(f16vec4 p0, f16vec4 p1);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
float16_t dot(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
f16vec4 normalize(f16vec4 x);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// If dot(Nref, I) < 0 return N, otherwise return -N.
f16vec4 faceforward(f16vec4 N, f16vec4 I, f16vec4 Nref);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
f16vec4 reflect(f16vec4 I, f16vec4 N);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
f16vec4 refract(f16vec4 I, f16vec4 N, float16_t eta);
#endif
// Returns the length of vector x, i.e., sqrt(x[0]^2 + x[1]^2 + …​).
double length(dvec4 x);
// Returns the distance between p0 and p1, i.e., length(p0 - p1).
double distance(dvec4 p0, dvec4 p1);
// Returns the dot product of x and y, i.e., x[0] · y[0] + x[1] · y[1] + …​
double dot(dvec4 x, dvec4 y);
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
dvec4 normalize(dvec4 x);
// If dot(Nref, I) < 0 return N, otherwise return -N.
dvec4 faceforward(dvec4 N, dvec4 I, dvec4 Nref);
// For the incident vector I and surface orientation N, returns the reflection direction: I - 2 · dot(N, I) · N. N must already be normalized in order to achieve the desired result.
dvec4 reflect(dvec4 I, dvec4 N);
// For the incident vector I and surface normal N, and the ratio of indices of refraction eta, return the refraction vector.
// 
// The result is computed by the refraction equation.
// 
// The input parameters for the incident vector I and the surface normal N must already be normalized to get the desired results.
dvec4 refract(dvec4 I, dvec4 N, double eta);
// Returns the cross product of x and y, i.e., (x[1] · y[2] - y[1] · x[2], x[2] · y[0] - y[2] · x[0], x[0] · y[1] - y[0] · x[1]).
vec3 cross(vec3 x, vec3 y);
// Returns the cross product of x and y, i.e., (x[1] · y[2] - y[1] · x[2], x[2] · y[0] - y[2] · x[0], x[0] · y[1] - y[0] · x[1]).
dvec3 cross(dvec3 x, dvec3 y);
// Available only when using the compatibility profile. For core OpenGL, use invariant.
// 
// For vertex shaders only. This function will ensure that the incoming vertex value will be transformed in a way that produces exactly the same result as would be produced by OpenGL's fixed functionality transform. It is intended to be used to compute gl_Position, e.g.
// 
// `gl_Position = ftransform()`
// 
// This function should be used, for example, when an application is rendering the same geometry in separate passes, and one pass uses the fixed functionality path to render and another pass uses programmable shaders.
vec4 ftransform();
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat2 matrixCompMult(mat2 x, mat2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat2 outerProduct(vec2 c, vec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat2 transpose(mat2 m);
// Returns the determinant of *m*.
float determinant(mat2 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
mat2 inverse(mat2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat2 matrixCompMult(f16mat2 x, f16mat2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat2 outerProduct(f16vec2 c, f16vec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat2 transpose(f16mat2 m);
// Returns the determinant of *m*.
float16_t determinant(f16mat2 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
f16mat2 inverse(f16mat2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat2 matrixCompMult(dmat2 x, dmat2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat2 outerProduct(dvec2 c, dvec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat2 transpose(dmat2 m);
// Returns the determinant of *m*.
double determinant(dmat2 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
dmat2 inverse(dmat2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat2x3 matrixCompMult(mat2x3 x, mat2x3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat2x3 outerProduct(vec2 c, vec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat3x2 transpose(mat2x3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat2x3 matrixCompMult(f16mat2x3 x, f16mat2x3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat2x3 outerProduct(f16vec2 c, f16vec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat3x2 transpose(f16mat2x3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat2x3 matrixCompMult(dmat2x3 x, dmat2x3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat2x3 outerProduct(dvec2 c, dvec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat3x2 transpose(dmat2x3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat2x4 matrixCompMult(mat2x4 x, mat2x4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat2x4 outerProduct(vec2 c, vec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat4x2 transpose(mat2x4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat2x4 matrixCompMult(f16mat2x4 x, f16mat2x4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat2x4 outerProduct(f16vec2 c, f16vec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat4x2 transpose(f16mat2x4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat2x4 matrixCompMult(dmat2x4 x, dmat2x4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat2x4 outerProduct(dvec2 c, dvec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat4x2 transpose(dmat2x4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat3x2 matrixCompMult(mat3x2 x, mat3x2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat3x2 outerProduct(vec3 c, vec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat2x3 transpose(mat3x2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat3x2 matrixCompMult(f16mat3x2 x, f16mat3x2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat3x2 outerProduct(f16vec3 c, f16vec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat2x3 transpose(f16mat3x2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat3x2 matrixCompMult(dmat3x2 x, dmat3x2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat3x2 outerProduct(dvec3 c, dvec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat2x3 transpose(dmat3x2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat3 matrixCompMult(mat3 x, mat3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat3 outerProduct(vec3 c, vec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat3 transpose(mat3 m);
// Returns the determinant of *m*.
float determinant(mat3 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
mat3 inverse(mat3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat3 matrixCompMult(f16mat3 x, f16mat3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat3 outerProduct(f16vec3 c, f16vec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat3 transpose(f16mat3 m);
// Returns the determinant of *m*.
float16_t determinant(f16mat3 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
f16mat3 inverse(f16mat3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat3 matrixCompMult(dmat3 x, dmat3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat3 outerProduct(dvec3 c, dvec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat3 transpose(dmat3 m);
// Returns the determinant of *m*.
double determinant(dmat3 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
dmat3 inverse(dmat3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat3x4 matrixCompMult(mat3x4 x, mat3x4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat3x4 outerProduct(vec3 c, vec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat4x3 transpose(mat3x4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat3x4 matrixCompMult(f16mat3x4 x, f16mat3x4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat3x4 outerProduct(f16vec3 c, f16vec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat4x3 transpose(f16mat3x4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat3x4 matrixCompMult(dmat3x4 x, dmat3x4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat3x4 outerProduct(dvec3 c, dvec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat4x3 transpose(dmat3x4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat4x2 matrixCompMult(mat4x2 x, mat4x2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat4x2 outerProduct(vec4 c, vec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat2x4 transpose(mat4x2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat4x2 matrixCompMult(f16mat4x2 x, f16mat4x2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat4x2 outerProduct(f16vec4 c, f16vec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat2x4 transpose(f16mat4x2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat4x2 matrixCompMult(dmat4x2 x, dmat4x2 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat4x2 outerProduct(dvec4 c, dvec2 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat2x4 transpose(dmat4x2 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat4x3 matrixCompMult(mat4x3 x, mat4x3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat4x3 outerProduct(vec4 c, vec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat3x4 transpose(mat4x3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat4x3 matrixCompMult(f16mat4x3 x, f16mat4x3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat4x3 outerProduct(f16vec4 c, f16vec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat3x4 transpose(f16mat4x3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat4x3 matrixCompMult(dmat4x3 x, dmat4x3 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat4x3 outerProduct(dvec4 c, dvec3 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat3x4 transpose(dmat4x3 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
mat4 matrixCompMult(mat4 x, mat4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
mat4 outerProduct(vec4 c, vec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
mat4 transpose(mat4 m);
// Returns the determinant of *m*.
float determinant(mat4 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
mat4 inverse(mat4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
f16mat4 matrixCompMult(f16mat4 x, f16mat4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
f16mat4 outerProduct(f16vec4 c, f16vec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
f16mat4 transpose(f16mat4 m);
// Returns the determinant of *m*.
float16_t determinant(f16mat4 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
f16mat4 inverse(f16mat4 m);
// Multiply matrix *x* by matrix *y* component-wise, i.e., result[i][j] is the scalar product of *x*[i][j] and *y*[i][j].
// 
// Note: to get linear algebraic matrix multiplication, use the multiply operator (*).
dmat4 matrixCompMult(dmat4 x, dmat4 y);
// Treats the first parameter *c* as a column vector (matrix with one column) and the second parameter *r* as a row vector (matrix with one row) and does a linear algebraic matrix multiply *c* * *r*, yielding a matrix whose number of rows is the number of components in *c* and whose number of columns is the number of components in *r*.
dmat4 outerProduct(dvec4 c, dvec4 r);
// Returns a matrix that is the transpose of *m*. The input matrix *m* is not modified.
dmat4 transpose(dmat4 m);
// Returns the determinant of *m*.
double determinant(dmat4 m);
// Returns a matrix that is the inverse of *m*. The input matrix *m* is not modified. The values in the returned matrix are undefined if *m* is singular or poorly-conditioned (nearly singular).
dmat4 inverse(dmat4 m);
// Returns the component-wise compare of x < y.
bvec2 lessThan(vec2 x, vec2 y);
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(vec2 x, vec2 y);
// Returns the component-wise compare of x > y.
bvec2 greaterThan(vec2 x, vec2 y);
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(vec2 x, vec2 y);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(f16vec2 x, f16vec2 y);
#endif
// Returns the component-wise compare of x < y.
bvec2 lessThan(dvec2 x, dvec2 y);
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(dvec2 x, dvec2 y);
// Returns the component-wise compare of x > y.
bvec2 greaterThan(dvec2 x, dvec2 y);
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(dvec2 x, dvec2 y);
// Returns the component-wise compare of x < y.
bvec2 lessThan(ivec2 x, ivec2 y);
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(ivec2 x, ivec2 y);
// Returns the component-wise compare of x > y.
bvec2 greaterThan(ivec2 x, ivec2 y);
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(ivec2 x, ivec2 y);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(i64vec2 x, i64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(i64vec2 x, i64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(i64vec2 x, i64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(i64vec2 x, i64vec2 y);
#endif
// Returns the component-wise compare of x < y.
bvec2 lessThan(uvec2 x, uvec2 y);
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(uvec2 x, uvec2 y);
// Returns the component-wise compare of x > y.
bvec2 greaterThan(uvec2 x, uvec2 y);
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(uvec2 x, uvec2 y);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x < y.
bvec2 lessThan(u64vec2 x, u64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x <= y.
bvec2 lessThanEqual(u64vec2 x, u64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x > y.
bvec2 greaterThan(u64vec2 x, u64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x >= y.
bvec2 greaterThanEqual(u64vec2 x, u64vec2 y);
#endif
// Returns the component-wise compare of x < y.
bvec3 lessThan(vec3 x, vec3 y);
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(vec3 x, vec3 y);
// Returns the component-wise compare of x > y.
bvec3 greaterThan(vec3 x, vec3 y);
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(vec3 x, vec3 y);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(f16vec3 x, f16vec3 y);
#endif
// Returns the component-wise compare of x < y.
bvec3 lessThan(dvec3 x, dvec3 y);
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(dvec3 x, dvec3 y);
// Returns the component-wise compare of x > y.
bvec3 greaterThan(dvec3 x, dvec3 y);
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(dvec3 x, dvec3 y);
// Returns the component-wise compare of x < y.
bvec3 lessThan(ivec3 x, ivec3 y);
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(ivec3 x, ivec3 y);
// Returns the component-wise compare of x > y.
bvec3 greaterThan(ivec3 x, ivec3 y);
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(ivec3 x, ivec3 y);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(i64vec3 x, i64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(i64vec3 x, i64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(i64vec3 x, i64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(i64vec3 x, i64vec3 y);
#endif
// Returns the component-wise compare of x < y.
bvec3 lessThan(uvec3 x, uvec3 y);
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(uvec3 x, uvec3 y);
// Returns the component-wise compare of x > y.
bvec3 greaterThan(uvec3 x, uvec3 y);
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(uvec3 x, uvec3 y);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x < y.
bvec3 lessThan(u64vec3 x, u64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x <= y.
bvec3 lessThanEqual(u64vec3 x, u64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x > y.
bvec3 greaterThan(u64vec3 x, u64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x >= y.
bvec3 greaterThanEqual(u64vec3 x, u64vec3 y);
#endif
// Returns the component-wise compare of x < y.
bvec4 lessThan(vec4 x, vec4 y);
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(vec4 x, vec4 y);
// Returns the component-wise compare of x > y.
bvec4 greaterThan(vec4 x, vec4 y);
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(vec4 x, vec4 y);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(f16vec4 x, f16vec4 y);
#endif
// Returns the component-wise compare of x < y.
bvec4 lessThan(dvec4 x, dvec4 y);
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(dvec4 x, dvec4 y);
// Returns the component-wise compare of x > y.
bvec4 greaterThan(dvec4 x, dvec4 y);
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(dvec4 x, dvec4 y);
// Returns the component-wise compare of x < y.
bvec4 lessThan(ivec4 x, ivec4 y);
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(ivec4 x, ivec4 y);
// Returns the component-wise compare of x > y.
bvec4 greaterThan(ivec4 x, ivec4 y);
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(ivec4 x, ivec4 y);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(i64vec4 x, i64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(i64vec4 x, i64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(i64vec4 x, i64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(i64vec4 x, i64vec4 y);
#endif
// Returns the component-wise compare of x < y.
bvec4 lessThan(uvec4 x, uvec4 y);
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(uvec4 x, uvec4 y);
// Returns the component-wise compare of x > y.
bvec4 greaterThan(uvec4 x, uvec4 y);
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(uvec4 x, uvec4 y);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x < y.
bvec4 lessThan(u64vec4 x, u64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x <= y.
bvec4 lessThanEqual(u64vec4 x, u64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x > y.
bvec4 greaterThan(u64vec4 x, u64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x >= y.
bvec4 greaterThanEqual(u64vec4 x, u64vec4 y);
#endif
// Returns the component-wise compare of x == y.
bvec2 equal(vec2 x, vec2 y);
// Returns the component-wise compare of x != y.
bvec2 notEqual(vec2 x, vec2 y);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(f16vec2 x, f16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(f16vec2 x, f16vec2 y);
#endif
// Returns the component-wise compare of x == y.
bvec2 equal(dvec2 x, dvec2 y);
// Returns the component-wise compare of x != y.
bvec2 notEqual(dvec2 x, dvec2 y);
// Returns the component-wise compare of x == y.
bvec2 equal(ivec2 x, ivec2 y);
// Returns the component-wise compare of x != y.
bvec2 notEqual(ivec2 x, ivec2 y);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(i8vec2 x, i8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(i16vec2 x, i16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(i64vec2 x, i64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(i64vec2 x, i64vec2 y);
#endif
// Returns the component-wise compare of x == y.
bvec2 equal(uvec2 x, uvec2 y);
// Returns the component-wise compare of x != y.
bvec2 notEqual(uvec2 x, uvec2 y);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(u8vec2 x, u8vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(u16vec2 x, u16vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x == y.
bvec2 equal(u64vec2 x, u64vec2 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x != y.
bvec2 notEqual(u64vec2 x, u64vec2 y);
#endif
// Returns the component-wise compare of x == y.
bvec2 equal(bvec2 x, bvec2 y);
// Returns the component-wise compare of x != y.
bvec2 notEqual(bvec2 x, bvec2 y);
// Returns the component-wise compare of x == y.
bvec3 equal(vec3 x, vec3 y);
// Returns the component-wise compare of x != y.
bvec3 notEqual(vec3 x, vec3 y);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(f16vec3 x, f16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(f16vec3 x, f16vec3 y);
#endif
// Returns the component-wise compare of x == y.
bvec3 equal(dvec3 x, dvec3 y);
// Returns the component-wise compare of x != y.
bvec3 notEqual(dvec3 x, dvec3 y);
// Returns the component-wise compare of x == y.
bvec3 equal(ivec3 x, ivec3 y);
// Returns the component-wise compare of x != y.
bvec3 notEqual(ivec3 x, ivec3 y);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(i8vec3 x, i8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(i16vec3 x, i16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(i64vec3 x, i64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(i64vec3 x, i64vec3 y);
#endif
// Returns the component-wise compare of x == y.
bvec3 equal(uvec3 x, uvec3 y);
// Returns the component-wise compare of x != y.
bvec3 notEqual(uvec3 x, uvec3 y);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(u8vec3 x, u8vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(u16vec3 x, u16vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x == y.
bvec3 equal(u64vec3 x, u64vec3 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x != y.
bvec3 notEqual(u64vec3 x, u64vec3 y);
#endif
// Returns the component-wise compare of x == y.
bvec3 equal(bvec3 x, bvec3 y);
// Returns the component-wise compare of x != y.
bvec3 notEqual(bvec3 x, bvec3 y);
// Returns the component-wise compare of x == y.
bvec4 equal(vec4 x, vec4 y);
// Returns the component-wise compare of x != y.
bvec4 notEqual(vec4 x, vec4 y);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(f16vec4 x, f16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(f16vec4 x, f16vec4 y);
#endif
// Returns the component-wise compare of x == y.
bvec4 equal(dvec4 x, dvec4 y);
// Returns the component-wise compare of x != y.
bvec4 notEqual(dvec4 x, dvec4 y);
// Returns the component-wise compare of x == y.
bvec4 equal(ivec4 x, ivec4 y);
// Returns the component-wise compare of x != y.
bvec4 notEqual(ivec4 x, ivec4 y);
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT8_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(i8vec4 x, i8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT16_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(i16vec4 x, i16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(i64vec4 x, i64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_INT64_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(i64vec4 x, i64vec4 y);
#endif
// Returns the component-wise compare of x == y.
bvec4 equal(uvec4 x, uvec4 y);
// Returns the component-wise compare of x != y.
bvec4 notEqual(uvec4 x, uvec4 y);
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT8_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(u8vec4 x, u8vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT16_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(u16vec4 x, u16vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x == y.
bvec4 equal(u64vec4 x, u64vec4 y);
#endif
#if __GLSLD_FEATURE_ENABLE_UINT64_TYPE
// Returns the component-wise compare of x != y.
bvec4 notEqual(u64vec4 x, u64vec4 y);
#endif
// Returns the component-wise compare of x == y.
bvec4 equal(bvec4 x, bvec4 y);
// Returns the component-wise compare of x != y.
bvec4 notEqual(bvec4 x, bvec4 y);
// Returns true if any component of x is true.
bool any(bvec2 x);
// Returns true if all components of x are true.
bool all(bvec2 x);
// Returns the component-wise logical complement of x.
bvec2 not(bvec2 x);
// Returns true if any component of x is true.
bool any(bvec3 x);
// Returns true if all components of x are true.
bool all(bvec3 x);
// Returns the component-wise logical complement of x.
bvec3 not(bvec3 x);
// Returns true if any component of x is true.
bool any(bvec4 x);
// Returns true if all components of x are true.
bool all(bvec4 x);
// Returns the component-wise logical complement of x.
bvec4 not(bvec4 x);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uint uaddCarry(highp uint x, highp uint y, out lowp uint carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uint usubBorrow(highp uint x, highp uint y, out lowp uint borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uint x, highp uint y, out highp uint msb, out highp uint lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uint8_t uaddCarry(highp uint8_t x, highp uint8_t y, out lowp uint8_t carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uint8_t usubBorrow(highp uint8_t x, highp uint8_t y, out lowp uint8_t borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uint8_t x, highp uint8_t y, out highp uint8_t msb, out highp uint8_t lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uint16_t uaddCarry(highp uint16_t x, highp uint16_t y, out lowp uint16_t carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uint16_t usubBorrow(highp uint16_t x, highp uint16_t y, out lowp uint16_t borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uint16_t x, highp uint16_t y, out highp uint16_t msb, out highp uint16_t lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uint64_t uaddCarry(highp uint64_t x, highp uint64_t y, out lowp uint64_t carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uint64_t usubBorrow(highp uint64_t x, highp uint64_t y, out lowp uint64_t borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uint64_t x, highp uint64_t y, out highp uint64_t msb, out highp uint64_t lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uvec2 uaddCarry(highp uvec2 x, highp uvec2 y, out lowp uvec2 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uvec2 usubBorrow(highp uvec2 x, highp uvec2 y, out lowp uvec2 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uvec2 x, highp uvec2 y, out highp uvec2 msb, out highp uvec2 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u8vec2 uaddCarry(highp u8vec2 x, highp u8vec2 y, out lowp u8vec2 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u8vec2 usubBorrow(highp u8vec2 x, highp u8vec2 y, out lowp u8vec2 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u8vec2 x, highp u8vec2 y, out highp u8vec2 msb, out highp u8vec2 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u16vec2 uaddCarry(highp u16vec2 x, highp u16vec2 y, out lowp u16vec2 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u16vec2 usubBorrow(highp u16vec2 x, highp u16vec2 y, out lowp u16vec2 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u16vec2 x, highp u16vec2 y, out highp u16vec2 msb, out highp u16vec2 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u64vec2 uaddCarry(highp u64vec2 x, highp u64vec2 y, out lowp u64vec2 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u64vec2 usubBorrow(highp u64vec2 x, highp u64vec2 y, out lowp u64vec2 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u64vec2 x, highp u64vec2 y, out highp u64vec2 msb, out highp u64vec2 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uvec3 uaddCarry(highp uvec3 x, highp uvec3 y, out lowp uvec3 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uvec3 usubBorrow(highp uvec3 x, highp uvec3 y, out lowp uvec3 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uvec3 x, highp uvec3 y, out highp uvec3 msb, out highp uvec3 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u8vec3 uaddCarry(highp u8vec3 x, highp u8vec3 y, out lowp u8vec3 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u8vec3 usubBorrow(highp u8vec3 x, highp u8vec3 y, out lowp u8vec3 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u8vec3 x, highp u8vec3 y, out highp u8vec3 msb, out highp u8vec3 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u16vec3 uaddCarry(highp u16vec3 x, highp u16vec3 y, out lowp u16vec3 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u16vec3 usubBorrow(highp u16vec3 x, highp u16vec3 y, out lowp u16vec3 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u16vec3 x, highp u16vec3 y, out highp u16vec3 msb, out highp u16vec3 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u64vec3 uaddCarry(highp u64vec3 x, highp u64vec3 y, out lowp u64vec3 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u64vec3 usubBorrow(highp u64vec3 x, highp u64vec3 y, out lowp u64vec3 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u64vec3 x, highp u64vec3 y, out highp u64vec3 msb, out highp u64vec3 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
uvec4 uaddCarry(highp uvec4 x, highp uvec4 y, out lowp uvec4 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
uvec4 usubBorrow(highp uvec4 x, highp uvec4 y, out lowp uvec4 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp uvec4 x, highp uvec4 y, out highp uvec4 msb, out highp uvec4 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u8vec4 uaddCarry(highp u8vec4 x, highp u8vec4 y, out lowp u8vec4 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u8vec4 usubBorrow(highp u8vec4 x, highp u8vec4 y, out lowp u8vec4 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u8vec4 x, highp u8vec4 y, out highp u8vec4 msb, out highp u8vec4 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u16vec4 uaddCarry(highp u16vec4 x, highp u16vec4 y, out lowp u16vec4 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u16vec4 usubBorrow(highp u16vec4 x, highp u16vec4 y, out lowp u16vec4 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u16vec4 x, highp u16vec4 y, out highp u16vec4 msb, out highp u16vec4 lsb);
// Adds 32-bit unsigned integers *x* and *y*, returning the sum modulo 2^32. The value *carry* is set to zero if the sum was less than 2^32, or one otherwise.
u64vec4 uaddCarry(highp u64vec4 x, highp u64vec4 y, out lowp u64vec4 carry);
// Subtracts the 32-bit unsigned integer *y* from *x*, returning the difference if non-negative, or 2^32 plus the difference otherwise. The value *borrow* is set to zero if *x* >= *y*, or one otherwise.
u64vec4 usubBorrow(highp u64vec4 x, highp u64vec4 y, out lowp u64vec4 borrow);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void umulExtended(highp u64vec4 x, highp u64vec4 y, out highp u64vec4 msb, out highp u64vec4 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp int x, highp int y, out highp int msb, out highp int lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp int8_t x, highp int8_t y, out highp int8_t msb, out highp int8_t lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp int16_t x, highp int16_t y, out highp int16_t msb, out highp int16_t lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp int64_t x, highp int64_t y, out highp int64_t msb, out highp int64_t lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp ivec2 x, highp ivec2 y, out highp ivec2 msb, out highp ivec2 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i8vec2 x, highp i8vec2 y, out highp i8vec2 msb, out highp i8vec2 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i16vec2 x, highp i16vec2 y, out highp i16vec2 msb, out highp i16vec2 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i64vec2 x, highp i64vec2 y, out highp i64vec2 msb, out highp i64vec2 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp ivec3 x, highp ivec3 y, out highp ivec3 msb, out highp ivec3 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i8vec3 x, highp i8vec3 y, out highp i8vec3 msb, out highp i8vec3 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i16vec3 x, highp i16vec3 y, out highp i16vec3 msb, out highp i16vec3 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i64vec3 x, highp i64vec3 y, out highp i64vec3 msb, out highp i64vec3 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp ivec4 x, highp ivec4 y, out highp ivec4 msb, out highp ivec4 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i8vec4 x, highp i8vec4 y, out highp i8vec4 msb, out highp i8vec4 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i16vec4 x, highp i16vec4 y, out highp i16vec4 msb, out highp i16vec4 lsb);
// Multiplies 32-bit unsigned or signed integers *x* and *y*, producing a 64-bit result. The 32 least-significant bits are returned in *lsb*. The 32 most-significant bits are returned in *msb*.
void imulExtended(highp i64vec4 x, highp i64vec4 y, out highp i64vec4 msb, out highp i64vec4 lsb);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
int bitfieldExtract(int value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
int bitfieldInsert(int base, int insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
int bitfieldReverse(highp int value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(int value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(int value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp int value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
int8_t bitfieldExtract(int8_t value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
int8_t bitfieldInsert(int8_t base, int8_t insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
int8_t bitfieldReverse(highp int8_t value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(int8_t value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(int8_t value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp int8_t value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
int16_t bitfieldExtract(int16_t value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
int16_t bitfieldInsert(int16_t base, int16_t insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
int16_t bitfieldReverse(highp int16_t value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(int16_t value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(int16_t value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp int16_t value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
int64_t bitfieldExtract(int64_t value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
int64_t bitfieldInsert(int64_t base, int64_t insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
int64_t bitfieldReverse(highp int64_t value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(int64_t value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(int64_t value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp int64_t value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uint bitfieldExtract(uint value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uint bitfieldInsert(uint base, uint insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uint bitfieldReverse(highp uint value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(uint value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(uint value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp uint value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uint8_t bitfieldExtract(uint8_t value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uint8_t bitfieldInsert(uint8_t base, uint8_t insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uint8_t bitfieldReverse(highp uint8_t value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(uint8_t value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(uint8_t value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp uint8_t value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uint16_t bitfieldExtract(uint16_t value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uint16_t bitfieldInsert(uint16_t base, uint16_t insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uint16_t bitfieldReverse(highp uint16_t value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(uint16_t value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(uint16_t value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp uint16_t value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uint64_t bitfieldExtract(uint64_t value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uint64_t bitfieldInsert(uint64_t base, uint64_t insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uint64_t bitfieldReverse(highp uint64_t value);
// Returns the number of one bits in the binary representation of *value*.
int bitCount(uint64_t value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
int findLSB(uint64_t value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
int findMSB(highp uint64_t value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
ivec2 bitfieldExtract(ivec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
ivec2 bitfieldInsert(ivec2 base, ivec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
ivec2 bitfieldReverse(highp ivec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(ivec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(ivec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp ivec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i8vec2 bitfieldExtract(i8vec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i8vec2 bitfieldInsert(i8vec2 base, i8vec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i8vec2 bitfieldReverse(highp i8vec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(i8vec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(i8vec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp i8vec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i16vec2 bitfieldExtract(i16vec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i16vec2 bitfieldInsert(i16vec2 base, i16vec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i16vec2 bitfieldReverse(highp i16vec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(i16vec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(i16vec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp i16vec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i64vec2 bitfieldExtract(i64vec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i64vec2 bitfieldInsert(i64vec2 base, i64vec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i64vec2 bitfieldReverse(highp i64vec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(i64vec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(i64vec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp i64vec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uvec2 bitfieldExtract(uvec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uvec2 bitfieldInsert(uvec2 base, uvec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uvec2 bitfieldReverse(highp uvec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(uvec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(uvec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp uvec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u8vec2 bitfieldExtract(u8vec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u8vec2 bitfieldInsert(u8vec2 base, u8vec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u8vec2 bitfieldReverse(highp u8vec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(u8vec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(u8vec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp u8vec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u16vec2 bitfieldExtract(u16vec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u16vec2 bitfieldInsert(u16vec2 base, u16vec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u16vec2 bitfieldReverse(highp u16vec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(u16vec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(u16vec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp u16vec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u64vec2 bitfieldExtract(u64vec2 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u64vec2 bitfieldInsert(u64vec2 base, u64vec2 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u64vec2 bitfieldReverse(highp u64vec2 value);
// Returns the number of one bits in the binary representation of *value*.
ivec2 bitCount(u64vec2 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec2 findLSB(u64vec2 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec2 findMSB(highp u64vec2 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
ivec3 bitfieldExtract(ivec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
ivec3 bitfieldInsert(ivec3 base, ivec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
ivec3 bitfieldReverse(highp ivec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(ivec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(ivec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp ivec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i8vec3 bitfieldExtract(i8vec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i8vec3 bitfieldInsert(i8vec3 base, i8vec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i8vec3 bitfieldReverse(highp i8vec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(i8vec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(i8vec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp i8vec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i16vec3 bitfieldExtract(i16vec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i16vec3 bitfieldInsert(i16vec3 base, i16vec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i16vec3 bitfieldReverse(highp i16vec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(i16vec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(i16vec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp i16vec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i64vec3 bitfieldExtract(i64vec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i64vec3 bitfieldInsert(i64vec3 base, i64vec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i64vec3 bitfieldReverse(highp i64vec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(i64vec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(i64vec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp i64vec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uvec3 bitfieldExtract(uvec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uvec3 bitfieldInsert(uvec3 base, uvec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uvec3 bitfieldReverse(highp uvec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(uvec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(uvec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp uvec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u8vec3 bitfieldExtract(u8vec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u8vec3 bitfieldInsert(u8vec3 base, u8vec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u8vec3 bitfieldReverse(highp u8vec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(u8vec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(u8vec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp u8vec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u16vec3 bitfieldExtract(u16vec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u16vec3 bitfieldInsert(u16vec3 base, u16vec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u16vec3 bitfieldReverse(highp u16vec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(u16vec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(u16vec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp u16vec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u64vec3 bitfieldExtract(u64vec3 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u64vec3 bitfieldInsert(u64vec3 base, u64vec3 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u64vec3 bitfieldReverse(highp u64vec3 value);
// Returns the number of one bits in the binary representation of *value*.
ivec3 bitCount(u64vec3 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec3 findLSB(u64vec3 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec3 findMSB(highp u64vec3 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
ivec4 bitfieldExtract(ivec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
ivec4 bitfieldInsert(ivec4 base, ivec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
ivec4 bitfieldReverse(highp ivec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(ivec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(ivec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp ivec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i8vec4 bitfieldExtract(i8vec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i8vec4 bitfieldInsert(i8vec4 base, i8vec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i8vec4 bitfieldReverse(highp i8vec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(i8vec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(i8vec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp i8vec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i16vec4 bitfieldExtract(i16vec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i16vec4 bitfieldInsert(i16vec4 base, i16vec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i16vec4 bitfieldReverse(highp i16vec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(i16vec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(i16vec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp i16vec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
i64vec4 bitfieldExtract(i64vec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
i64vec4 bitfieldInsert(i64vec4 base, i64vec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
i64vec4 bitfieldReverse(highp i64vec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(i64vec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(i64vec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp i64vec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
uvec4 bitfieldExtract(uvec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
uvec4 bitfieldInsert(uvec4 base, uvec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
uvec4 bitfieldReverse(highp uvec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(uvec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(uvec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp uvec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u8vec4 bitfieldExtract(u8vec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u8vec4 bitfieldInsert(u8vec4 base, u8vec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u8vec4 bitfieldReverse(highp u8vec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(u8vec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(u8vec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp u8vec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u16vec4 bitfieldExtract(u16vec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u16vec4 bitfieldInsert(u16vec4 base, u16vec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u16vec4 bitfieldReverse(highp u16vec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(u16vec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(u16vec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp u16vec4 value);
// Extracts bits [*offset*, *offset* + *bits* - 1] from *value*, returning them in the least significant bits of the result.
// 
// For unsigned data types, the most significant bits of the result will be set to zero. For signed data types, the most significant bits will be set to the value of bit *offset* + *bits* - 1.
// 
// If *bits* is zero, the result will be zero. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldExtract**(), a single pair of *offset* and *bits* values is shared for all components.
u64vec4 bitfieldExtract(u64vec4 value, int offset, int bits);
// Inserts the *bits* least significant bits of *insert* into *base*.
// 
// The result will have bits [*offset*, *offset* + *bits* - 1] taken from bits [0, *bits* - 1] of *insert*, and all other bits taken directly from the corresponding bits of *base*. If *bits* is zero, the result will simply be *base*. The result will be undefined if *offset* or *bits* is negative, or if the sum of *offset* and *bits* is greater than the number of bits used to store the operand. Note that for vector versions of **bitfieldInsert**(), a single pair of *offset* and *bits* values is shared for all components.
u64vec4 bitfieldInsert(u64vec4 base, u64vec4 insert, int offset, int bits);
// Reverses the bits of *value*. The bit numbered *n* of the result will be taken from bit (*bits* - 1) - *n* of *value*, where *bits* is the total number of bits used to represent *value*.
u64vec4 bitfieldReverse(highp u64vec4 value);
// Returns the number of one bits in the binary representation of *value*.
ivec4 bitCount(u64vec4 value);
// Returns the bit number of the least significant one bit in the binary representation of *value*. If *value* is zero, -1 will be returned.
ivec4 findLSB(u64vec4 value);
// Returns the bit number of the most significant bit in the binary representation of *value*.
// 
// For positive integers, the result will be the bit number of the most significant one bit. For negative integers, the result will be the bit number of the most significant zero bit. For a *value* of zero or negative one, -1 will be returned.
ivec4 findMSB(highp u64vec4 value);
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
float textureSize(sampler1D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler1D sampler_, float P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler1D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(sampler1D sampler_, float P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
vec4 textureProj(sampler1D sampler_, float P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(sampler1D sampler_, float P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
vec4 textureOffset(sampler1D sampler_, float P, int offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler1D sampler_, int P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
vec4 texelFetchOffset(sampler1D sampler_, int P, int lod, int offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
vec4 textureProjOffset(sampler1D sampler_, float P, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
vec4 textureLodOffset(sampler1D sampler_, float P, float lod, int offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
vec4 textureProjLod(sampler1D sampler_, vec2 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
vec4 textureProjLodOffset(sampler1D sampler_, vec2 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(sampler1D sampler_, float P, float dPdx, float dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
vec4 textureGradOffset(sampler1D sampler_, float P, float dPdx, float dPdy, int offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
vec4 textureProjGrad(sampler1D sampler_, float P, float lod, float dPdx, float dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
vec4 textureProjGradOffset(sampler1D sampler_, float P, float lod, float dPdx, float dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
float textureSize(sampler1DArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler1DArray sampler_, float P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler1DArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(sampler1DArray sampler_, vec2 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(sampler1DArray sampler_, vec2 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
vec4 textureOffset(sampler1DArray sampler_, vec2 P, int offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler1DArray sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
vec4 texelFetchOffset(sampler1DArray sampler_, ivec2 P, int lod, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
vec4 textureLodOffset(sampler1DArray sampler_, vec2 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(sampler1DArray sampler_, vec2 P, float dPdx, float dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
vec4 textureGradOffset(sampler1DArray sampler_, vec2 P, float dPdx, float dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
float textureSize(sampler1DShadow sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler1DShadow sampler_, float P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler1DShadow sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(sampler1DShadow sampler_, vec3 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
float textureProj(sampler1DShadow sampler_, vec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
float textureLod(sampler1DShadow sampler_, vec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
float textureOffset(sampler1DShadow sampler_, vec3 P, int offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
float textureProjOffset(sampler1DShadow sampler_, vec3 P, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
float textureLodOffset(sampler1DShadow sampler_, vec3 P, float lod, int offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
float textureProjLod(sampler1DShadow sampler_, vec4 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
float textureProjLodOffset(sampler1DShadow sampler_, vec4 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(sampler1DShadow sampler_, vec3 P, float dPdx, float dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
float textureGradOffset(sampler1DShadow sampler_, vec3 P, float dPdx, float dPdy, int offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
float textureProjGrad(sampler1DShadow sampler_, vec3 P, float lod, float dPdx, float dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
float textureProjGradOffset(sampler1DShadow sampler_, vec3 P, float lod, float dPdx, float dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
float textureSize(sampler1DArrayShadow sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler1DArrayShadow sampler_, float P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler1DArrayShadow sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(sampler1DArrayShadow sampler_, vec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
float textureLod(sampler1DArrayShadow sampler_, vec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
float textureOffset(sampler1DArrayShadow sampler_, vec3 P, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
float textureLodOffset(sampler1DArrayShadow sampler_, vec3 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(sampler1DArrayShadow sampler_, vec3 P, float dPdx, float dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
float textureGradOffset(sampler1DArrayShadow sampler_, vec3 P, float dPdx, float dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler2D sampler_, vec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler2D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(sampler2D sampler_, vec2 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
vec4 textureProj(sampler2D sampler_, vec2 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(sampler2D sampler_, vec2 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
vec4 textureOffset(sampler2D sampler_, vec2 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler2D sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
vec4 texelFetchOffset(sampler2D sampler_, ivec2 P, int lod, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
vec4 textureProjOffset(sampler2D sampler_, vec2 P, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
vec4 textureLodOffset(sampler2D sampler_, vec2 P, float lod, ivec2 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
vec4 textureProjLod(sampler2D sampler_, vec3 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
vec4 textureProjLodOffset(sampler2D sampler_, vec3 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(sampler2D sampler_, vec2 P, vec2 dPdx, vec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
vec4 textureGradOffset(sampler2D sampler_, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
vec4 textureProjGrad(sampler2D sampler_, vec2 P, float lod, vec2 dPdx, vec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
vec4 textureProjGradOffset(sampler2D sampler_, vec2 P, float lod, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2D sampler_, vec2 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2D sampler_, vec2 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
vec4 textureGatherOffset(sampler2D sampler_, vec2 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
vec4 textureGatherOffsets(sampler2D sampler_, vec2 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler2DArray sampler_, vec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler2DArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(sampler2DArray sampler_, vec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(sampler2DArray sampler_, vec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
vec4 textureOffset(sampler2DArray sampler_, vec3 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler2DArray sampler_, ivec3 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
vec4 texelFetchOffset(sampler2DArray sampler_, ivec3 P, int lod, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
vec4 textureLodOffset(sampler2DArray sampler_, vec3 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(sampler2DArray sampler_, vec3 P, vec2 dPdx, vec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
vec4 textureGradOffset(sampler2DArray sampler_, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DArray sampler_, vec3 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DArray sampler_, vec3 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
vec4 textureGatherOffset(sampler2DArray sampler_, vec3 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
vec4 textureGatherOffsets(sampler2DArray sampler_, vec3 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DShadow sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler2DShadow sampler_, vec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler2DShadow sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(sampler2DShadow sampler_, vec3 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
float textureProj(sampler2DShadow sampler_, vec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
float textureLod(sampler2DShadow sampler_, vec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
float textureOffset(sampler2DShadow sampler_, vec3 P, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
float textureProjOffset(sampler2DShadow sampler_, vec3 P, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
float textureLodOffset(sampler2DShadow sampler_, vec3 P, float lod, ivec2 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
float textureProjLod(sampler2DShadow sampler_, vec4 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
float textureProjLodOffset(sampler2DShadow sampler_, vec4 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(sampler2DShadow sampler_, vec3 P, vec2 dPdx, vec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
float textureGradOffset(sampler2DShadow sampler_, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
float textureProjGrad(sampler2DShadow sampler_, vec3 P, float lod, vec2 dPdx, vec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
float textureProjGradOffset(sampler2DShadow sampler_, vec3 P, float lod, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DShadow sampler_, vec2 P, float refZ)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
vec4 textureGatherOffset(sampler2DShadow sampler_, vec2 P, float refZ, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
vec4 textureGatherOffsets(sampler2DShadow sampler_, vec2 P, float refZ, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DArrayShadow sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler2DArrayShadow sampler_, vec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler2DArrayShadow sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(sampler2DArrayShadow sampler_, vec4 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
float textureLod(sampler2DArrayShadow sampler_, vec4 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
float textureOffset(sampler2DArrayShadow sampler_, vec4 P, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
float textureLodOffset(sampler2DArrayShadow sampler_, vec4 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(sampler2DArrayShadow sampler_, vec4 P, vec2 dPdx, vec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
float textureGradOffset(sampler2DArrayShadow sampler_, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DArrayShadow sampler_, vec3 P, float refZ)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
vec4 textureGatherOffset(sampler2DArrayShadow sampler_, vec3 P, float refZ, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
vec4 textureGatherOffsets(sampler2DArrayShadow sampler_, vec3 P, float refZ, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec3 textureSize(samplerCube sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(samplerCube sampler_, vec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(samplerCube sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(samplerCube sampler_, vec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(samplerCube sampler_, vec3 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(samplerCube sampler_, vec3 P, vec3 dPdx, vec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(samplerCube sampler_, vec3 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(samplerCube sampler_, vec3 P, int comp)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec3 textureSize(samplerCubeArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(samplerCubeArray sampler_, vec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(samplerCubeArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(samplerCubeArray sampler_, vec4 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(samplerCubeArray sampler_, vec4 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(samplerCubeArray sampler_, vec4 P, vec3 dPdx, vec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(samplerCubeArray sampler_, vec4 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(samplerCubeArray sampler_, vec4 P, int comp)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec3 textureSize(samplerCubeShadow sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(samplerCubeShadow sampler_, vec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(samplerCubeShadow sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(samplerCubeShadow sampler_, vec4 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
float textureLod(samplerCubeShadow sampler_, vec4 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(samplerCubeShadow sampler_, vec4 P, vec3 dPdx, vec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(samplerCubeShadow sampler_, vec3 P, float refZ)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec3 textureSize(samplerCubeArrayShadow sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(samplerCubeArrayShadow sampler_, vec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(samplerCubeArrayShadow sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(samplerCubeArrayShadow sampler_, vec4 P, float compare)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
float textureLod(samplerCubeArrayShadow sampler_, vec4 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(samplerCubeArrayShadow sampler_, vec4 P, vec3 dPdx, vec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(samplerCubeArrayShadow sampler_, vec4 P, float refZ)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec3 textureSize(sampler3D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(sampler3D sampler_, vec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(sampler3D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(sampler3D sampler_, vec3 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
vec4 textureProj(sampler3D sampler_, vec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
vec4 textureLod(sampler3D sampler_, vec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
vec4 textureOffset(sampler3D sampler_, vec3 P, ivec3 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler3D sampler_, ivec3 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
vec4 texelFetchOffset(sampler3D sampler_, ivec3 P, int lod, ivec3 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
vec4 textureProjOffset(sampler3D sampler_, vec3 P, ivec3 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
vec4 textureLodOffset(sampler3D sampler_, vec3 P, float lod, ivec3 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
vec4 textureProjLod(sampler3D sampler_, vec4 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
vec4 textureProjLodOffset(sampler3D sampler_, vec4 P, float lod, ivec3 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(sampler3D sampler_, vec3 P, vec3 dPdx, vec3 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
vec4 textureGradOffset(sampler3D sampler_, vec3 P, vec3 dPdx, vec3 dPdy, ivec3 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
vec4 textureProjGrad(sampler3D sampler_, vec3 P, float lod, vec3 dPdx, vec3 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
vec4 textureProjGradOffset(sampler3D sampler_, vec3 P, float lod, vec3 dPdx, vec3 dPdy, ivec3 offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DRect sampler_, int lod);
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
vec4 texture(sampler2DRect sampler_, vec2 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
vec4 textureProj(sampler2DRect sampler_, vec2 P)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
vec4 textureOffset(sampler2DRect sampler_, vec2 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler2DRect sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
vec4 texelFetchOffset(sampler2DRect sampler_, ivec2 P, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
vec4 textureProjOffset(sampler2DRect sampler_, vec2 P, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
vec4 textureGrad(sampler2DRect sampler_, vec2 P, vec2 dPdx, vec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
vec4 textureGradOffset(sampler2DRect sampler_, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
vec4 textureProjGrad(sampler2DRect sampler_, vec2 P, float lod, vec2 dPdx, vec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
vec4 textureProjGradOffset(sampler2DRect sampler_, vec2 P, float lod, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DRect sampler_, vec2 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DRect sampler_, vec2 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
vec4 textureGatherOffset(sampler2DRect sampler_, vec2 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
vec4 textureGatherOffsets(sampler2DRect sampler_, vec2 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DRectShadow sampler_, int lod);
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
float texture(sampler2DRectShadow sampler_, vec3 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
float textureProj(sampler2DRectShadow sampler_, vec3 P)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
float textureOffset(sampler2DRectShadow sampler_, vec3 P, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
float textureProjOffset(sampler2DRectShadow sampler_, vec3 P, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
float textureGrad(sampler2DRectShadow sampler_, vec3 P, vec2 dPdx, vec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
float textureGradOffset(sampler2DRectShadow sampler_, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
float textureProjGrad(sampler2DRectShadow sampler_, vec3 P, float lod, vec2 dPdx, vec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
float textureProjGradOffset(sampler2DRectShadow sampler_, vec3 P, float lod, vec2 dPdx, vec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
vec4 textureGather(sampler2DRectShadow sampler_, vec2 P, float refZ)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
vec4 textureGatherOffset(sampler2DRectShadow sampler_, vec2 P, float refZ, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
vec4 textureGatherOffsets(sampler2DRectShadow sampler_, vec2 P, float refZ, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
float textureSize(samplerBuffer sampler_, int lod);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(samplerBuffer sampler_, int P, int lod)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DMS sampler_, int lod);
// Returns the number of samples of the texture or textures bound to sampler.
int textureSamples(sampler2DMS sampler_);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler2DMS sampler_, ivec2 P, int sample)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
vec2 textureSize(sampler2DMSArray sampler_, int lod);
// Returns the number of samples of the texture or textures bound to sampler.
int textureSamples(sampler2DMSArray sampler_);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
vec4 texelFetch(sampler2DMSArray sampler_, ivec3 P, int sample)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
int textureSize(isampler1D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isampler1D sampler_, int P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isampler1D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isampler1D sampler_, int P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
ivec4 textureProj(isampler1D sampler_, int P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isampler1D sampler_, int P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
ivec4 textureOffset(isampler1D sampler_, int P, int offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler1D sampler_, int P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
ivec4 texelFetchOffset(isampler1D sampler_, int P, int lod, int offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
ivec4 textureProjOffset(isampler1D sampler_, int P, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
ivec4 textureLodOffset(isampler1D sampler_, int P, float lod, int offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
ivec4 textureProjLod(isampler1D sampler_, ivec2 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
ivec4 textureProjLodOffset(isampler1D sampler_, ivec2 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isampler1D sampler_, int P, int dPdx, int dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
ivec4 textureGradOffset(isampler1D sampler_, int P, int dPdx, int dPdy, int offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
ivec4 textureProjGrad(isampler1D sampler_, int P, float lod, int dPdx, int dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
ivec4 textureProjGradOffset(isampler1D sampler_, int P, float lod, int dPdx, int dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
int textureSize(isampler1DArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isampler1DArray sampler_, int P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isampler1DArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isampler1DArray sampler_, ivec2 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isampler1DArray sampler_, ivec2 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
ivec4 textureOffset(isampler1DArray sampler_, ivec2 P, int offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler1DArray sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
ivec4 texelFetchOffset(isampler1DArray sampler_, ivec2 P, int lod, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
ivec4 textureLodOffset(isampler1DArray sampler_, ivec2 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isampler1DArray sampler_, ivec2 P, int dPdx, int dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
ivec4 textureGradOffset(isampler1DArray sampler_, ivec2 P, int dPdx, int dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec2 textureSize(isampler2D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isampler2D sampler_, ivec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isampler2D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isampler2D sampler_, ivec2 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
ivec4 textureProj(isampler2D sampler_, ivec2 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isampler2D sampler_, ivec2 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
ivec4 textureOffset(isampler2D sampler_, ivec2 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler2D sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
ivec4 texelFetchOffset(isampler2D sampler_, ivec2 P, int lod, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
ivec4 textureProjOffset(isampler2D sampler_, ivec2 P, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
ivec4 textureLodOffset(isampler2D sampler_, ivec2 P, float lod, ivec2 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
ivec4 textureProjLod(isampler2D sampler_, ivec3 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
ivec4 textureProjLodOffset(isampler2D sampler_, ivec3 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isampler2D sampler_, ivec2 P, ivec2 dPdx, ivec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
ivec4 textureGradOffset(isampler2D sampler_, ivec2 P, ivec2 dPdx, ivec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
ivec4 textureProjGrad(isampler2D sampler_, ivec2 P, float lod, ivec2 dPdx, ivec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
ivec4 textureProjGradOffset(isampler2D sampler_, ivec2 P, float lod, ivec2 dPdx, ivec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isampler2D sampler_, ivec2 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isampler2D sampler_, ivec2 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
ivec4 textureGatherOffset(isampler2D sampler_, ivec2 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
ivec4 textureGatherOffsets(isampler2D sampler_, ivec2 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec2 textureSize(isampler2DArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isampler2DArray sampler_, ivec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isampler2DArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isampler2DArray sampler_, ivec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isampler2DArray sampler_, ivec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
ivec4 textureOffset(isampler2DArray sampler_, ivec3 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler2DArray sampler_, ivec3 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
ivec4 texelFetchOffset(isampler2DArray sampler_, ivec3 P, int lod, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
ivec4 textureLodOffset(isampler2DArray sampler_, ivec3 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isampler2DArray sampler_, ivec3 P, ivec2 dPdx, ivec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
ivec4 textureGradOffset(isampler2DArray sampler_, ivec3 P, ivec2 dPdx, ivec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isampler2DArray sampler_, ivec3 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isampler2DArray sampler_, ivec3 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
ivec4 textureGatherOffset(isampler2DArray sampler_, ivec3 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
ivec4 textureGatherOffsets(isampler2DArray sampler_, ivec3 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec3 textureSize(isamplerCube sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isamplerCube sampler_, ivec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isamplerCube sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isamplerCube sampler_, ivec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isamplerCube sampler_, ivec3 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isamplerCube sampler_, ivec3 P, ivec3 dPdx, ivec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isamplerCube sampler_, ivec3 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isamplerCube sampler_, ivec3 P, int comp)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec3 textureSize(isamplerCubeArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isamplerCubeArray sampler_, ivec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isamplerCubeArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isamplerCubeArray sampler_, ivec4 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isamplerCubeArray sampler_, ivec4 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isamplerCubeArray sampler_, ivec4 P, ivec3 dPdx, ivec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isamplerCubeArray sampler_, ivec4 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isamplerCubeArray sampler_, ivec4 P, int comp)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec3 textureSize(isampler3D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(isampler3D sampler_, ivec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(isampler3D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isampler3D sampler_, ivec3 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
ivec4 textureProj(isampler3D sampler_, ivec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
ivec4 textureLod(isampler3D sampler_, ivec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
ivec4 textureOffset(isampler3D sampler_, ivec3 P, ivec3 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler3D sampler_, ivec3 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
ivec4 texelFetchOffset(isampler3D sampler_, ivec3 P, int lod, ivec3 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
ivec4 textureProjOffset(isampler3D sampler_, ivec3 P, ivec3 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
ivec4 textureLodOffset(isampler3D sampler_, ivec3 P, float lod, ivec3 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
ivec4 textureProjLod(isampler3D sampler_, ivec4 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
ivec4 textureProjLodOffset(isampler3D sampler_, ivec4 P, float lod, ivec3 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isampler3D sampler_, ivec3 P, ivec3 dPdx, ivec3 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
ivec4 textureGradOffset(isampler3D sampler_, ivec3 P, ivec3 dPdx, ivec3 dPdy, ivec3 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
ivec4 textureProjGrad(isampler3D sampler_, ivec3 P, float lod, ivec3 dPdx, ivec3 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
ivec4 textureProjGradOffset(isampler3D sampler_, ivec3 P, float lod, ivec3 dPdx, ivec3 dPdy, ivec3 offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec2 textureSize(isampler2DRect sampler_, int lod);
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
ivec4 texture(isampler2DRect sampler_, ivec2 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
ivec4 textureProj(isampler2DRect sampler_, ivec2 P)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
ivec4 textureOffset(isampler2DRect sampler_, ivec2 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler2DRect sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
ivec4 texelFetchOffset(isampler2DRect sampler_, ivec2 P, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
ivec4 textureProjOffset(isampler2DRect sampler_, ivec2 P, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
ivec4 textureGrad(isampler2DRect sampler_, ivec2 P, ivec2 dPdx, ivec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
ivec4 textureGradOffset(isampler2DRect sampler_, ivec2 P, ivec2 dPdx, ivec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
ivec4 textureProjGrad(isampler2DRect sampler_, ivec2 P, float lod, ivec2 dPdx, ivec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
ivec4 textureProjGradOffset(isampler2DRect sampler_, ivec2 P, float lod, ivec2 dPdx, ivec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isampler2DRect sampler_, ivec2 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
ivec4 textureGather(isampler2DRect sampler_, ivec2 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
ivec4 textureGatherOffset(isampler2DRect sampler_, ivec2 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
ivec4 textureGatherOffsets(isampler2DRect sampler_, ivec2 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
int textureSize(isamplerBuffer sampler_, int lod);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isamplerBuffer sampler_, int P, int lod)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec2 textureSize(isampler2DMS sampler_, int lod);
// Returns the number of samples of the texture or textures bound to sampler.
int textureSamples(isampler2DMS sampler_);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler2DMS sampler_, ivec2 P, int sample)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
ivec2 textureSize(isampler2DMSArray sampler_, int lod);
// Returns the number of samples of the texture or textures bound to sampler.
int textureSamples(isampler2DMSArray sampler_);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
ivec4 texelFetch(isampler2DMSArray sampler_, ivec3 P, int sample)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uint textureSize(usampler1D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usampler1D sampler_, uint P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usampler1D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usampler1D sampler_, uint P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
uvec4 textureProj(usampler1D sampler_, uint P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usampler1D sampler_, uint P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
uvec4 textureOffset(usampler1D sampler_, uint P, int offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler1D sampler_, int P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
uvec4 texelFetchOffset(usampler1D sampler_, int P, int lod, int offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
uvec4 textureProjOffset(usampler1D sampler_, uint P, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
uvec4 textureLodOffset(usampler1D sampler_, uint P, float lod, int offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
uvec4 textureProjLod(usampler1D sampler_, uvec2 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
uvec4 textureProjLodOffset(usampler1D sampler_, uvec2 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usampler1D sampler_, uint P, uint dPdx, uint dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
uvec4 textureGradOffset(usampler1D sampler_, uint P, uint dPdx, uint dPdy, int offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
uvec4 textureProjGrad(usampler1D sampler_, uint P, float lod, uint dPdx, uint dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
uvec4 textureProjGradOffset(usampler1D sampler_, uint P, float lod, uint dPdx, uint dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uint textureSize(usampler1DArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usampler1DArray sampler_, uint P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usampler1DArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usampler1DArray sampler_, uvec2 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usampler1DArray sampler_, uvec2 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
uvec4 textureOffset(usampler1DArray sampler_, uvec2 P, int offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler1DArray sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
uvec4 texelFetchOffset(usampler1DArray sampler_, ivec2 P, int lod, int offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
uvec4 textureLodOffset(usampler1DArray sampler_, uvec2 P, float lod, int offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usampler1DArray sampler_, uvec2 P, uint dPdx, uint dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
uvec4 textureGradOffset(usampler1DArray sampler_, uvec2 P, uint dPdx, uint dPdy, int offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec2 textureSize(usampler2D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usampler2D sampler_, uvec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usampler2D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usampler2D sampler_, uvec2 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
uvec4 textureProj(usampler2D sampler_, uvec2 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usampler2D sampler_, uvec2 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
uvec4 textureOffset(usampler2D sampler_, uvec2 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler2D sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
uvec4 texelFetchOffset(usampler2D sampler_, ivec2 P, int lod, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
uvec4 textureProjOffset(usampler2D sampler_, uvec2 P, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
uvec4 textureLodOffset(usampler2D sampler_, uvec2 P, float lod, ivec2 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
uvec4 textureProjLod(usampler2D sampler_, uvec3 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
uvec4 textureProjLodOffset(usampler2D sampler_, uvec3 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usampler2D sampler_, uvec2 P, uvec2 dPdx, uvec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
uvec4 textureGradOffset(usampler2D sampler_, uvec2 P, uvec2 dPdx, uvec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
uvec4 textureProjGrad(usampler2D sampler_, uvec2 P, float lod, uvec2 dPdx, uvec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
uvec4 textureProjGradOffset(usampler2D sampler_, uvec2 P, float lod, uvec2 dPdx, uvec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usampler2D sampler_, uvec2 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usampler2D sampler_, uvec2 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
uvec4 textureGatherOffset(usampler2D sampler_, uvec2 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
uvec4 textureGatherOffsets(usampler2D sampler_, uvec2 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec2 textureSize(usampler2DArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usampler2DArray sampler_, uvec2 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usampler2DArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usampler2DArray sampler_, uvec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usampler2DArray sampler_, uvec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
uvec4 textureOffset(usampler2DArray sampler_, uvec3 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler2DArray sampler_, ivec3 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
uvec4 texelFetchOffset(usampler2DArray sampler_, ivec3 P, int lod, ivec2 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
uvec4 textureLodOffset(usampler2DArray sampler_, uvec3 P, float lod, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usampler2DArray sampler_, uvec3 P, uvec2 dPdx, uvec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
uvec4 textureGradOffset(usampler2DArray sampler_, uvec3 P, uvec2 dPdx, uvec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usampler2DArray sampler_, uvec3 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usampler2DArray sampler_, uvec3 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
uvec4 textureGatherOffset(usampler2DArray sampler_, uvec3 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
uvec4 textureGatherOffsets(usampler2DArray sampler_, uvec3 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec3 textureSize(usamplerCube sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usamplerCube sampler_, uvec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usamplerCube sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usamplerCube sampler_, uvec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usamplerCube sampler_, uvec3 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usamplerCube sampler_, uvec3 P, uvec3 dPdx, uvec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usamplerCube sampler_, uvec3 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usamplerCube sampler_, uvec3 P, int comp)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec3 textureSize(usamplerCubeArray sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usamplerCubeArray sampler_, uvec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usamplerCubeArray sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usamplerCubeArray sampler_, uvec4 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usamplerCubeArray sampler_, uvec4 P, float lod)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usamplerCubeArray sampler_, uvec4 P, uvec3 dPdx, uvec3 dPdy)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usamplerCubeArray sampler_, uvec4 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usamplerCubeArray sampler_, uvec4 P, int comp)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec3 textureSize(usampler3D sampler_, int lod);
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// 
// Returns the computed level-of-detail relative to the base level in the y component of the return value.
// 
// If called on an incomplete texture, the results are undefined.
vec2 textureQueryLod(usampler3D sampler_, uvec3 P);
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined in the OpenGL Specification.
// 
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// 
// Available in all shader stages.
int textureQueryLevels(usampler3D sampler_)
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usampler3D sampler_, uvec3 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
uvec4 textureProj(usampler3D sampler_, uvec3 P)
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies λ_base and sets the partial derivatives as follows:
// (See section 8.14 “Texture Minification” and equations 8.4-8.6 of the OpenGL Specification.)
// 
// ∂u / ∂x = ∂v / ∂x = ∂w / ∂x = 0
// ∂u / ∂y = ∂v / ∂y = ∂w / ∂y = 0
uvec4 textureLod(usampler3D sampler_, uvec3 P, float lod)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
uvec4 textureOffset(usampler3D sampler_, uvec3 P, ivec3 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler3D sampler_, ivec3 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
uvec4 texelFetchOffset(usampler3D sampler_, ivec3 P, int lod, ivec3 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
uvec4 textureProjOffset(usampler3D sampler_, uvec3 P, ivec3 offset)
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
uvec4 textureLodOffset(usampler3D sampler_, uvec3 P, float lod, ivec3 offset)
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
uvec4 textureProjLod(usampler3D sampler_, uvec4 P, float lod)
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj, textureLod, and textureOffset.
uvec4 textureProjLodOffset(usampler3D sampler_, uvec4 P, float lod, ivec3 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usampler3D sampler_, uvec3 P, uvec3 dPdx, uvec3 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
uvec4 textureGradOffset(usampler3D sampler_, uvec3 P, uvec3 dPdx, uvec3 dPdy, ivec3 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
uvec4 textureProjGrad(usampler3D sampler_, uvec3 P, float lod, uvec3 dPdx, uvec3 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
uvec4 textureProjGradOffset(usampler3D sampler_, uvec3 P, float lod, uvec3 dPdx, uvec3 dPdy, ivec3 offset)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec2 textureSize(usampler2DRect sampler_, int lod);
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// 
// For shadow forms: When compare is present, it is used as Dref and the array layer comes from the last component of P. When compare is not present, the last component of P is used as Dref and the array layer comes from the second to last component of P. (The second component of P is unused for 1D shadow lookups.)
// 
// For non-shadow forms: the array layer comes from the last component of P.
uvec4 texture(usampler2DRect sampler_, uvec2 P)
// Do a texture lookup with projection. The texture coordinates consumed from P, not including the last component of P, are divided by the last component of P to form projected coordinates P'. The resulting third component of P in the shadow forms is used as Dref. The third component of P is ignored when sampler has type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds as in texture.
uvec4 textureProj(usampler2DRect sampler_, uvec2 P)
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates before looking up each texel. The offset value must be a constant expression. A limited range of offset values are supported; the minimum and maximum offset values are implementation-dependent and given by gl_MinProgramTexelOffset and gl_MaxProgramTexelOffset, respectively.
// 
// Note that offset does not apply to the layer coordinate for texture arrays. This is explained in detail in section 8.14.2 “Coordinate Wrapping and Texel Selection” of the OpenGL Specification, where offset is (δu, δv, δw).
// Note that texel offsets are also not supported for cube maps.
uvec4 textureOffset(usampler2DRect sampler_, uvec2 P, ivec2 offset)
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler2DRect sampler_, ivec2 P, int lod)
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
uvec4 texelFetchOffset(usampler2DRect sampler_, ivec2 P, ivec2 offset)
// Do a projective texture lookup as described in textureProj, offset by offset as described in textureOffset.
uvec4 textureProjOffset(usampler2DRect sampler_, uvec2 P, ivec2 offset)
// Do a texture lookup as in texture but with explicit gradients as shown below. The partial derivatives of P are with respect to window x and window y. For the cube version, the partial derivatives of P are assumed to be in the coordinate system used before texture coordinates are projected onto the appropriate cube face.
uvec4 textureGrad(usampler2DRect sampler_, uvec2 P, uvec2 dPdx, uvec2 dPdy)
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and textureOffset.
uvec4 textureGradOffset(usampler2DRect sampler_, uvec2 P, uvec2 dPdx, uvec2 dPdy, ivec2 offset)
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already projected.
uvec4 textureProjGrad(usampler2DRect sampler_, uvec2 P, float lod, uvec2 dPdx, uvec2 dPdy)
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as well as with offset, as described in textureOffset.
uvec4 textureProjGradOffset(usampler2DRect sampler_, uvec2 P, float lod, uvec2 dPdx, uvec2 dPdy, ivec2 offset)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usampler2DRect sampler_, uvec2 P)
// Returns the value
// 
// ```
// vec4(Sample_i0_j1(P, base).comp,
// Sample_i1_j1(P, base).comp,
// Sample_i1_j0(P, base).comp,
// Sample_i0_j0(P, base).comp)
// ```
// 
// If specified, the value of comp must be a constant integer expression with a value of 0, 1, 2, or 3, identifying the x, y, z, or w post-swizzled component of the four-component vector lookup result for each texel, respectively. If comp is not specified, it is treated as 0, selecting the x component of each texel to generate the result.
uvec4 textureGather(usampler2DRect sampler_, uvec2 P, int comp)
// Perform a texture gather operation as in textureGather by offset as described in textureOffset except that the offset can be variable (non constant) and the implementation-dependent minimum and maximum offset values are given by MIN_PROGRAM_TEXTURE_GATHER_OFFSET and MAX_PROGRAM_TEXTURE_GATHER_OFFSET, respectively.
uvec4 textureGatherOffset(usampler2DRect sampler_, uvec2 P, ivec2 offset)
// Operate identically to textureGatherOffset except that offsets is used to determine the location of the four texels to sample. Each of the four texels is obtained by applying the corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the four-texel LINEAR footprint, and then selecting the texel i0 j0 of that footprint. The specified values in offsets must be constant integral expressions.
uvec4 textureGatherOffsets(usampler2DRect sampler_, uvec2 P, ivec2 offset[4])
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uint textureSize(usamplerBuffer sampler_, int lod);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usamplerBuffer sampler_, int P, int lod)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec2 textureSize(usampler2DMS sampler_, int lod);
// Returns the number of samples of the texture or textures bound to sampler.
int textureSamples(usampler2DMS sampler_);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler2DMS sampler_, ivec2 P, int sample)
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described in section 11.1.3.4 “Texture Queries” of the OpenGL Specification.
// The components in the return value are filled in, in order, with the width, height, and depth of the texture.
// 
// For the array forms, the last component of the return value is the number of layers in the texture array, or the number of cubes in the texture cube map array.
uvec2 textureSize(usampler2DMSArray sampler_, int lod);
// Returns the number of samples of the texture or textures bound to sampler.
int textureSamples(usampler2DMSArray sampler_);
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes from the last component of P for the array forms. The level-of-detail lod (if present) is as described in sections 11.1.3.2 “Texel Fetches” and 8.14.1 “Scale Factor and Level of Detail” of the OpenGL Specification.
uvec4 texelFetch(usampler2DMSArray sampler_, ivec3 P, int sample)
// Atomically
// 
// 1. increments the counter for *c*, and
// 2. returns its value prior to the increment operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterIncrement(atomic_uint c);
// Atomically
// 
// 1. decrements the counter for *c*, and
// 2. returns the value resulting from the decrement operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterDecrement(atomic_uint c);
// Returns the counter value for *c*.
uint atomicCounter(atomic_uint c);
// Atomically
// 
// 1. adds the value of *data* to the counter for *c*, and
// 2. returns its value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterAdd(atomic_uint c, uint data);
// Atomically
// 
// 1. subtracts the value of *data* from the counter for *c*, and
// 2. returns its value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterSubtract(atomic_uint c, uint data);
// Atomically
// 
// 1. sets the counter for *c* to the minimum of the value of the counter and the value of *data*, and
// 2. returns the value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterMin(atomic_uint c, uint data);
// Atomically
// 
// 1. sets the counter for *c* to the maximum of the value of the counter and the value of *data*, and
// 2. returns the value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterMax(atomic_uint c, uint data);
// Atomically
// 
// 1. sets the counter for *c* to the bitwise AND of the value of the counter and the value of *data*, and
// 2. returns the value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterAnd(atomic_uint c, uint data);
// Atomically
// 
// 1. sets the counter for *c* to the bitwise OR of the value of the counter and the value of *data*, and
// 2. returns the value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterOr(atomic_uint c, uint data);
// Atomically
// 
// 1. sets the counter for *c* to the bitwise XOR of the value of the counter and the value of *data*, and
// 2. returns the value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterXor(atomic_uint c, uint data);
// Atomically
// 
// 1. sets the counter value for *c* to the value of *data*, and
// 2. returns its value prior to the operation.
// 
// These two steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterExchange(atomic_uint c, uint data);
// Atomically
// 
// 1. compares the value of *compare* and the counter value for *c*
// 2. if the values are equal, sets the counter value for *c* to the value of *data*, and
// 3. returns its value prior to the operation.
// 
// These three steps are done atomically with respect to the atomic counter functions in this table.
uint atomicCounterCompSwap(atomic_uint c, uint compare, uint data);
// Computes a new value by adding the value of *data* to the contents of *mem*.
uint atomicAdd(inout uint mem, uint data);
// Computes a new value by adding the value of *data* to the contents of *mem*.
int atomicAdd(inout int mem, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of *mem*.
uint atomicMin(inout uint mem, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of *mem*.
int atomicMin(inout int mem, int data);
// Computes a new value by taking the maximum of the value of *data* and the contents of *mem*.
uint atomicMax(inout uint mem, uint data);
// Computes a new value by taking the maximum of the value of *data* and the contents of *mem*.
int atomicMax(inout int mem, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of *mem*.
uint atomicAnd(inout uint mem, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of *mem*.
int atomicAnd(inout int mem, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of *mem*.
uint atomicOr(inout uint mem, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of *mem*.
int atomicOr(inout int mem, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of *mem*.
uint atomicXor(inout uint mem, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of *mem*.
int atomicXor(inout int mem, int data);
// Computes a new value by simply copying the value of *data*.
uint atomicExchange(inout uint mem, uint data);
// Computes a new value by simply copying the value of *data*.
int atomicExchange(inout int mem, int data);
// Compares the value of *compare* and the contents of *mem*. If the values are equal, the new value is given by *data*; otherwise, it is taken from the original contents of *mem*.
uint atomicCompSwap(inout uint mem, uint compare, uint data);
// Compares the value of *compare* and the contents of *mem*. If the values are equal, the new value is given by *data*; otherwise, it is taken from the original contents of *mem*.
int atomicCompSwap(inout int mem, int compare, int data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly image1D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image1D image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image1D image_, int P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image1D image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image1D image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image1D image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image1D image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image1D image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image1D image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image1D image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image1D image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image1D image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image1D image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image1D image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image1D image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image1D image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image1D image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image1D image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image1D image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image1D image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly image1DArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image1DArray image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image1DArray image_, int P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image1DArray image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image1DArray image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image1DArray image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image1DArray image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image1DArray image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image1DArray image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image1DArray image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image1DArray image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image1DArray image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image1DArray image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image1DArray image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly image2D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image2D image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image2D image_, ivec2 P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image2D image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image2D image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image2D image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image2D image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image2D image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image2D image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image2D image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image2D image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image2D image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image2D image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image2D image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly image2DArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image2DArray image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image2DArray image_, ivec2 P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image2DArray image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image2DArray image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image2DArray image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image2DArray image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image2DArray image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image2DArray image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image2DArray image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image2DArray image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image2DArray image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image2DArray image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image2DArray image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly imageCube image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly imageCube image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly imageCube image_, ivec3 P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(imageCube image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(imageCube image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(imageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(imageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(imageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(imageCube image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(imageCube image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(imageCube image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(imageCube image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(imageCube image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(imageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(imageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(imageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(imageCube image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(imageCube image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(imageCube image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(imageCube image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly imageCubeArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly imageCubeArray image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly imageCubeArray image_, ivec3 P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(imageCubeArray image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(imageCubeArray image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(imageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(imageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(imageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(imageCubeArray image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(imageCubeArray image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(imageCubeArray image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(imageCubeArray image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(imageCubeArray image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(imageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(imageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(imageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(imageCubeArray image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(imageCubeArray image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(imageCubeArray image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(imageCubeArray image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly image3D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image3D image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image3D image_, ivec3 P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image3D image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image3D image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image3D image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image3D image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image3D image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image3D image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image3D image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image3D image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image3D image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image3D image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image3D image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly image2DRect image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image2DRect image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image2DRect image_, ivec2 P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image2DRect image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image2DRect image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image2DRect image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image2DRect image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image2DRect image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image2DRect image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image2DRect image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image2DRect image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image2DRect image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image2DRect image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image2DRect image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly imageBuffer image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly imageBuffer image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly imageBuffer image_, int P, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(imageBuffer image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(imageBuffer image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(imageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(imageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(imageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(imageBuffer image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(imageBuffer image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(imageBuffer image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(imageBuffer image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(imageBuffer image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(imageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(imageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(imageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(imageBuffer image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(imageBuffer image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(imageBuffer image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(imageBuffer image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly image2DMS image_);
// Returns the number of samples in the image bound to *image*.
int imageSamples(readonly writeonly image2DMS image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image2DMS image_, ivec2 P, int sample);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image2DMS image_, ivec2 P, int sample, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image2DMS image_, ivec2 P, int sample, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image2DMS image_, ivec2 P, int sample, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image2DMS image_, ivec2 P, int sample, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image2DMS image_, ivec2 P, int sample, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image2DMS image_, ivec2 P, int sample, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly image2DMSArray image_);
// Returns the number of samples in the image bound to *image*.
int imageSamples(readonly writeonly image2DMSArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
vec4 imageLoad(readonly image2DMSArray image_, ivec2 P, int sample);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly image2DMSArray image_, ivec2 P, int sample, vec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(image2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(image2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(image2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(image2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(image2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(image2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(image2DMSArray image_, ivec2 P, int sample, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(image2DMSArray image_, ivec2 P, int sample, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(image2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(image2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(image2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(image2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(image2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(image2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(image2DMSArray image_, ivec2 P, int sample, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(image2DMSArray image_, ivec2 P, int sample, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(image2DMSArray image_, ivec2 P, int sample, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly iimage1D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage1D image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage1D image_, int P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage1D image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage1D image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage1D image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage1D image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage1D image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage1D image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage1D image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage1D image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage1D image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage1D image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage1D image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage1D image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage1D image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage1D image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage1D image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage1D image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage1D image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly iimage1DArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage1DArray image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage1DArray image_, int P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage1DArray image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage1DArray image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage1DArray image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage1DArray image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage1DArray image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage1DArray image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage1DArray image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage1DArray image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage1DArray image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage1DArray image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage1DArray image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly iimage2D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage2D image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage2D image_, ivec2 P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage2D image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage2D image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage2D image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage2D image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage2D image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage2D image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage2D image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage2D image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage2D image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage2D image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage2D image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly iimage2DArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage2DArray image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage2DArray image_, ivec2 P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage2DArray image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage2DArray image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage2DArray image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage2DArray image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage2DArray image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage2DArray image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage2DArray image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage2DArray image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage2DArray image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage2DArray image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage2DArray image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly iimageCube image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimageCube image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimageCube image_, ivec3 P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimageCube image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimageCube image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimageCube image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimageCube image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimageCube image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimageCube image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimageCube image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimageCube image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimageCube image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimageCube image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimageCube image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly iimageCubeArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimageCubeArray image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimageCubeArray image_, ivec3 P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimageCubeArray image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimageCubeArray image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimageCubeArray image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimageCubeArray image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimageCubeArray image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimageCubeArray image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimageCubeArray image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimageCubeArray image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly iimage3D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage3D image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage3D image_, ivec3 P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage3D image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage3D image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage3D image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage3D image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage3D image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage3D image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage3D image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage3D image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage3D image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage3D image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage3D image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly iimage2DRect image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage2DRect image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage2DRect image_, ivec2 P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage2DRect image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage2DRect image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage2DRect image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage2DRect image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage2DRect image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage2DRect image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage2DRect image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage2DRect image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage2DRect image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage2DRect image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage2DRect image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly iimageBuffer image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimageBuffer image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimageBuffer image_, int P, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimageBuffer image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimageBuffer image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimageBuffer image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimageBuffer image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimageBuffer image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimageBuffer image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimageBuffer image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimageBuffer image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimageBuffer image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimageBuffer image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimageBuffer image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly iimage2DMS image_);
// Returns the number of samples in the image bound to *image*.
int imageSamples(readonly writeonly iimage2DMS image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage2DMS image_, ivec2 P, int sample);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage2DMS image_, ivec2 P, int sample, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage2DMS image_, ivec2 P, int sample, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage2DMS image_, ivec2 P, int sample, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage2DMS image_, ivec2 P, int sample, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage2DMS image_, ivec2 P, int sample, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage2DMS image_, ivec2 P, int sample, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly iimage2DMSArray image_);
// Returns the number of samples in the image bound to *image*.
int imageSamples(readonly writeonly iimage2DMSArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
ivec4 imageLoad(readonly iimage2DMSArray image_, ivec2 P, int sample);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly iimage2DMSArray image_, ivec2 P, int sample, ivec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(iimage2DMSArray image_, ivec2 P, int sample, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(iimage2DMSArray image_, ivec2 P, int sample, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(iimage2DMSArray image_, ivec2 P, int sample, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(iimage2DMSArray image_, ivec2 P, int sample, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(iimage2DMSArray image_, ivec2 P, int sample, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly uimage1D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage1D image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage1D image_, int P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage1D image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage1D image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage1D image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage1D image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage1D image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage1D image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage1D image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage1D image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage1D image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage1D image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage1D image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage1D image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage1D image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage1D image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage1D image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage1D image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage1D image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly uimage1DArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage1DArray image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage1DArray image_, int P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage1DArray image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage1DArray image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage1DArray image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage1DArray image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage1DArray image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage1DArray image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage1DArray image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage1DArray image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage1DArray image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage1DArray image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage1DArray image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage1DArray image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage1DArray image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly uimage2D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage2D image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage2D image_, ivec2 P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage2D image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage2D image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage2D image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage2D image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage2D image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage2D image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage2D image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage2D image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage2D image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage2D image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage2D image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage2D image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage2D image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly uimage2DArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage2DArray image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage2DArray image_, ivec2 P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage2DArray image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage2DArray image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage2DArray image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage2DArray image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage2DArray image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage2DArray image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage2DArray image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage2DArray image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage2DArray image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage2DArray image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage2DArray image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage2DArray image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage2DArray image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly uimageCube image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimageCube image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimageCube image_, ivec3 P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimageCube image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimageCube image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimageCube image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimageCube image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimageCube image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimageCube image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimageCube image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimageCube image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimageCube image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimageCube image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimageCube image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimageCube image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimageCube image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly uimageCubeArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimageCubeArray image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimageCubeArray image_, ivec3 P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimageCubeArray image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimageCubeArray image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimageCubeArray image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimageCubeArray image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimageCubeArray image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimageCubeArray image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimageCubeArray image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimageCubeArray image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimageCubeArray image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimageCubeArray image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec3 imageSize(readonly writeonly uimage3D image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage3D image_, ivec3 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage3D image_, ivec3 P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage3D image_, ivec3 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage3D image_, ivec3 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage3D image_, ivec3 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage3D image_, ivec3 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage3D image_, ivec3 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage3D image_, ivec3 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage3D image_, ivec3 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage3D image_, ivec3 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage3D image_, ivec3 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage3D image_, ivec3 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage3D image_, ivec3 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage3D image_, ivec3 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage3D image_, ivec3 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly uimage2DRect image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage2DRect image_, ivec2 P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage2DRect image_, ivec2 P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage2DRect image_, ivec2 P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage2DRect image_, ivec2 P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage2DRect image_, ivec2 P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage2DRect image_, ivec2 P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage2DRect image_, ivec2 P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage2DRect image_, ivec2 P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage2DRect image_, ivec2 P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage2DRect image_, ivec2 P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage2DRect image_, ivec2 P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage2DRect image_, ivec2 P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage2DRect image_, ivec2 P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage2DRect image_, ivec2 P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage2DRect image_, ivec2 P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
int imageSize(readonly writeonly uimageBuffer image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimageBuffer image_, int P);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimageBuffer image_, int P, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimageBuffer image_, int P, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimageBuffer image_, int P, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimageBuffer image_, int P, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimageBuffer image_, int P, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimageBuffer image_, int P, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimageBuffer image_, int P, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimageBuffer image_, int P, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimageBuffer image_, int P, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimageBuffer image_, int P, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimageBuffer image_, int P, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimageBuffer image_, int P, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimageBuffer image_, int P, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimageBuffer image_, int P, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly uimage2DMS image_);
// Returns the number of samples in the image bound to *image*.
int imageSamples(readonly writeonly uimage2DMS image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage2DMS image_, ivec2 P, int sample);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage2DMS image_, ivec2 P, int sample, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage2DMS image_, ivec2 P, int sample, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage2DMS image_, ivec2 P, int sample, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage2DMS image_, ivec2 P, int sample, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage2DMS image_, ivec2 P, int sample, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage2DMS image_, ivec2 P, int sample, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage2DMS image_, ivec2 P, int sample, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage2DMS image_, ivec2 P, int sample, float data);
// Returns the dimensions of the image or images bound to *image*.
// For arrayed images, the last component of the return value will hold the size of the array.
// Cube images only return the dimensions of one face, and the number of cubes in the cube map array, if arrayed.
// 
// Note: The qualification **readonly writeonly** accepts a variable qualified with **readonly**, **writeonly**, both, or neither. It means the formal argument will be used for neither reading nor writing to the underlying memory.
ivec2 imageSize(readonly writeonly uimage2DMSArray image_);
// Returns the number of samples in the image bound to *image*.
int imageSamples(readonly writeonly uimage2DMSArray image_);
// Loads the texel at the coordinate *P* from the image unit *image*.
// For multisample loads, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent the selected texel in memory are converted to a **vec4**, **ivec4**, or **uvec4** in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and returned.
uvec4 imageLoad(readonly uimage2DMSArray image_, ivec2 P, int sample);
// Stores *data* into the texel at the coordinate *P* from the image specified by *image*.
// For multisample stores, the sample number is given by *sample*.
// When *image*, *P*, and *sample* identify a valid texel, the bits used to represent *data* are converted to the format of the image unit in the manner described in section 8.26 "Texture Image Loads and Stores" of the OpenGL Specification and stored to the specified texel.
void imageStore(writeonly uimage2DMSArray image_, ivec2 P, int sample, uvec4 data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
int imageAtomicAdd(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
int imageAtomicMin(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
int imageAtomicMax(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
int imageAtomicAnd(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
int imageAtomicOr(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
int imageAtomicXor(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Computes a new value by simply copying the value of *data* into the selected texel.
int imageAtomicExchange(uimage2DMSArray image_, ivec2 P, int sample, int data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
int imageAtomicCompSwap(uimage2DMSArray image_, ivec2 P, int sample, int compare, int data);
// Computes a new value by adding the value of *data* to the contents of the selected texel.
uint imageAtomicAdd(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the minimum of the value of *data* and the contents of the selected texel.
uint imageAtomicMin(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by taking the maximum of the value *data* and the contents of the selected texel.
uint imageAtomicMax(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise AND of the value of *data* and the contents of the selected texel.
uint imageAtomicAnd(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise OR of the value of *data* and the contents of the selected texel.
uint imageAtomicOr(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of *data* and the contents of the selected texel.
uint imageAtomicXor(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
uint imageAtomicExchange(uimage2DMSArray image_, ivec2 P, int sample, uint data);
// Compares the value of *compare* and the contents of the selected texel.
// If the values are equal, the new value is given by *data*; otherwise, it is taken from the original value loaded from the texel.
uint imageAtomicCompSwap(uimage2DMSArray image_, ivec2 P, int sample, uint compare, uint data);
// Computes a new value by simply copying the value of *data* into the selected texel.
float imageAtomicExchange(uimage2DMSArray image_, ivec2 P, int sample, float data);
// Emits the current values of output variables to the current output primitive on stream *stream*.
// The argument to *stream* must be a constant integral expression.
// On return from this call, the values of all output variables are undefined.
// Can only be used if multiple output streams are supported.
void EmitStreamVertex(int stream);
// Completes the current output primitive on stream *stream* and starts a new one.
// The argument to *stream* must be a constant integral expression.
// No vertex is emitted.
// Can only be used if multiple output streams are supported.
void EndStreamPrimitive(int stream);
// Emits the current values of output variables to the current output primitive.
// When multiple output streams are supported, this is equivalent to calling **EmitStreamVertex**(0).
// On return from this call, the values of output variables are undefined.
void EmitVertex();
// Completes the current output primitive and starts a new one.
// When multiple output streams are supported, this is equivalent to calling **EndStreamPrimitive**(0).
// No vertex is emitted.
void EndPrimitive();
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
float dFdx(float p);
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
float dFdy(float p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
float dFdxFine(float p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
float dFdyFine(float p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
float dFdxCoarse(float p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
float dFdyCoarse(float p);
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
float fwidth(float p);
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
float fwidthFine(float p);
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
float fwidthCoarse(float p);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
float16_t dFdx(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
float16_t dFdy(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
float16_t dFdxFine(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
float16_t dFdyFine(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
float16_t dFdxCoarse(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
float16_t dFdyCoarse(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
float16_t fwidth(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
float16_t fwidthFine(float16_t p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
float16_t fwidthCoarse(float16_t p);
#endif
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
vec2 dFdx(vec2 p);
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
vec2 dFdy(vec2 p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
vec2 dFdxFine(vec2 p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
vec2 dFdyFine(vec2 p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
vec2 dFdxCoarse(vec2 p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
vec2 dFdyCoarse(vec2 p);
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
vec2 fwidth(vec2 p);
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
vec2 fwidthFine(vec2 p);
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
vec2 fwidthCoarse(vec2 p);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
f16vec2 dFdx(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
f16vec2 dFdy(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
f16vec2 dFdxFine(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
f16vec2 dFdyFine(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
f16vec2 dFdxCoarse(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
f16vec2 dFdyCoarse(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
f16vec2 fwidth(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
f16vec2 fwidthFine(f16vec2 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
f16vec2 fwidthCoarse(f16vec2 p);
#endif
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
vec3 dFdx(vec3 p);
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
vec3 dFdy(vec3 p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
vec3 dFdxFine(vec3 p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
vec3 dFdyFine(vec3 p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
vec3 dFdxCoarse(vec3 p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
vec3 dFdyCoarse(vec3 p);
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
vec3 fwidth(vec3 p);
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
vec3 fwidthFine(vec3 p);
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
vec3 fwidthCoarse(vec3 p);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
f16vec3 dFdx(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
f16vec3 dFdy(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
f16vec3 dFdxFine(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
f16vec3 dFdyFine(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
f16vec3 dFdxCoarse(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
f16vec3 dFdyCoarse(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
f16vec3 fwidth(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
f16vec3 fwidthFine(f16vec3 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
f16vec3 fwidthCoarse(f16vec3 p);
#endif
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
vec4 dFdx(vec4 p);
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
vec4 dFdy(vec4 p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
vec4 dFdxFine(vec4 p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
vec4 dFdyFine(vec4 p);
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
vec4 dFdxCoarse(vec4 p);
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
vec4 dFdyCoarse(vec4 p);
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
vec4 fwidth(vec4 p);
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
vec4 fwidthFine(vec4 p);
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
vec4 fwidthCoarse(vec4 p);
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdxFine**(*p*) or **dFdxCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
f16vec4 dFdx(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns either **dFdyFine**(*p*) or **dFdyCoarse**(*p*), based on implementation choice, presumably whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
f16vec4 dFdy(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
f16vec4 dFdxFine(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment and its immediate neighbor(s).
f16vec4 dFdyFine(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window x coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can x compute derivatives in fewer unique locations than would be allowed for **dFdxFine**(*p*).
f16vec4 dFdxCoarse(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns the partial derivative of *p* with respect to the window y coordinate. Will use local differencing based on the value of *p* for the current fragment's neighbors, and will possibly, but not necessarily, include the value of *p* for the current fragment. That is, over a given area, the implementation can compute y derivatives in fewer unique locations than would be allowed for **dFdyFine**(*p*).
f16vec4 dFdyCoarse(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdx**(*p*)) + **abs**(**dFdy**(*p*)).
f16vec4 fwidth(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxFine**(*p*)) + **abs**(**dFdyFine**(*p*)).
f16vec4 fwidthFine(f16vec4 p);
#endif
#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE
// Returns **abs**(**dFdxCoarse**(*p*)) + **abs**(**dFdyCoarse**(*p*)).
f16vec4 fwidthCoarse(f16vec4 p);
#endif
// Returns the value of the input *interpolant* sampled at a location inside both the pixel and the primitive being processed. The value obtained would be the same value assigned to the input variable if declared with the **centroid** qualifier.
float interpolateAtCentroid(float interpolant);
// Returns the value of the input *interpolant* variable at the location of sample number *sample*. If multisample buffers are not available, the input variable will be evaluated at the center of the pixel. If sample *sample* does not exist, the position used to interpolate the input variable is undefined.
float interpolateAtSample(float interpolant, int sample);
// Returns the value of the input *interpolant* variable sampled at an offset from the center of the pixel specified by *offset*. The two floating-point components of *offset*, give the offset in pixels in the *x* and *y* directions, respectively. An offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets supported by this function is implementation-dependent.
float interpolateAtOffset(float interpolant, vec2 offset);
// Returns the value of the input *interpolant* sampled at a location inside both the pixel and the primitive being processed. The value obtained would be the same value assigned to the input variable if declared with the **centroid** qualifier.
vec2 interpolateAtCentroid(vec2 interpolant);
// Returns the value of the input *interpolant* variable at the location of sample number *sample*. If multisample buffers are not available, the input variable will be evaluated at the center of the pixel. If sample *sample* does not exist, the position used to interpolate the input variable is undefined.
vec2 interpolateAtSample(vec2 interpolant, int sample);
// Returns the value of the input *interpolant* variable sampled at an offset from the center of the pixel specified by *offset*. The two floating-point components of *offset*, give the offset in pixels in the *x* and *y* directions, respectively. An offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets supported by this function is implementation-dependent.
vec2 interpolateAtOffset(vec2 interpolant, vec2 offset);
// Returns the value of the input *interpolant* sampled at a location inside both the pixel and the primitive being processed. The value obtained would be the same value assigned to the input variable if declared with the **centroid** qualifier.
vec3 interpolateAtCentroid(vec3 interpolant);
// Returns the value of the input *interpolant* variable at the location of sample number *sample*. If multisample buffers are not available, the input variable will be evaluated at the center of the pixel. If sample *sample* does not exist, the position used to interpolate the input variable is undefined.
vec3 interpolateAtSample(vec3 interpolant, int sample);
// Returns the value of the input *interpolant* variable sampled at an offset from the center of the pixel specified by *offset*. The two floating-point components of *offset*, give the offset in pixels in the *x* and *y* directions, respectively. An offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets supported by this function is implementation-dependent.
vec3 interpolateAtOffset(vec3 interpolant, vec2 offset);
// Returns the value of the input *interpolant* sampled at a location inside both the pixel and the primitive being processed. The value obtained would be the same value assigned to the input variable if declared with the **centroid** qualifier.
vec4 interpolateAtCentroid(vec4 interpolant);
// Returns the value of the input *interpolant* variable at the location of sample number *sample*. If multisample buffers are not available, the input variable will be evaluated at the center of the pixel. If sample *sample* does not exist, the position used to interpolate the input variable is undefined.
vec4 interpolateAtSample(vec4 interpolant, int sample);
// Returns the value of the input *interpolant* variable sampled at an offset from the center of the pixel specified by *offset*. The two floating-point components of *offset*, give the offset in pixels in the *x* and *y* directions, respectively. An offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets supported by this function is implementation-dependent.
vec4 interpolateAtOffset(vec4 interpolant, vec2 offset);
// For any given static instance of **barrier**(), all tessellation control shader invocations for a single input patch must enter it before any will be allowed to continue beyond it, or all compute shader invocations for a single workgroup must enter it before any will continue beyond it.
void barrier();
// Control the ordering of memory transactions issued by a single shader invocation.
// 
// When called, this function will wait for the completion of all reads and writes to atomic counter, buffer, image, and shared variables previously performed by the caller, and then return with no other effect.
void memoryBarrier();
// Control the ordering of accesses to atomic-counter variables issued by a single shader invocation.
// 
// When called, this function will wait for the completion of all reads and writes to atomic counter variables previously performed by the caller, and then return with no other effect.
void memoryBarrierAtomicCounter();
// Control the ordering of memory transactions to buffer variables issued within a single shader invocation.
// 
// When called, this function will wait for the completion of all reads and writes to buffer variables previously performed by the caller, and then return with no other effect.
void memoryBarrierBuffer();
// Control the ordering of memory transactions to shared variables issued within a single shader invocation, as viewed by other invocations in the same workgroup.
// 
// When called, this function will wait for the completion of all reads and writes to shared variables previously performed by the caller, and then return with no other effect.
// 
// Only available in compute shaders.
void memoryBarrierShared();
// Control the ordering of memory transactions to images issued within a single shader invocation.
// 
// When called, this function will wait for the completion of all reads and writes to image variables previously performed by the caller, and then return with no other effect.
void memoryBarrierImage();
// Control the ordering of all memory transactions issued within a single shader invocation, as viewed by other invocations in the same workgroup.
// 
// When called, this function will wait for the completion of all reads and writes to atomic counter, buffer, image, and shared variables previously performed by the caller, and then return with no other effect.
// 
// Only available in compute shaders.
void groupMemoryBarrier();
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
vec4 subpassLoad(subpassInput subpass);
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
vec4 subpassLoad(subpassInputMS subpass, int sample);
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
ivec4 subpassLoad(isubpassInput subpass);
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
ivec4 subpassLoad(isubpassInputMS subpass, int sample);
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
uvec4 subpassLoad(usubpassInput subpass);
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
uvec4 subpassLoad(usubpassInputMS subpass, int sample);
// Returns true if and only if value is true for at least one active invocation in the group.
bool anyInvocation(bool value);
// Returns true if and only if value is true for all active invocations in the group.
bool allInvocations(bool value);
// Returns true if value is the same for all active invocations in the group.
bool allInvocationsEqual(bool value);
)glsl";
} // namespace glsld
// clang-format on
