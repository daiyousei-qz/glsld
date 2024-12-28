
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