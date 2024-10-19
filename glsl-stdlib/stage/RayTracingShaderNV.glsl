
__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_NV_ray_tracing;

__glsld_syscmd_require_stage__ rayGen rayAnyHit rayClosestHit rayIntersect rayMiss rayCallable;
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

__glsld_syscmd_require_stage__ rayGen rayClosestHit rayMiss rayCallable;
void executeCallableNV(uint sbtRecordIndex, int callable);

__glsld_syscmd_require_stage__ rayGen rayClosestHit rayMiss;
void traceNV(accelerationStructureNV topLevel,uint rayFlags,uint cullMask,uint sbtRecordOffset,uint sbtRecordStride,uint missIndex,vec3 origin,float tMin,vec3 direction,float tMax,int payload);
void traceRayMotionNV(accelerationStructureNV topLevel,uint rayFlags,uint cullMask,uint sbtRecordOffset,uint sbtRecordStride,uint missIndex,vec3 origin,float tMin,vec3 direction,float tMax,float currentTime,int payload);

__glsld_syscmd_require_stage__ rayAnyHit rayClosestHit rayIntersect rayMiss;
in    vec3   gl_WorldRayOriginNV;
in    vec3   gl_WorldRayDirectionNV;
in    vec3   gl_ObjectRayOriginNV;
in    vec3   gl_ObjectRayDirectionNV;
in    float  gl_RayTminNV;
in    float  gl_RayTmaxNV;
in    uint   gl_IncomingRayFlagsNV;
in    float  gl_CurrentRayTimeNV;

__glsld_syscmd_require_stage__ rayAnyHit rayClosestHit rayIntersect;
in     int   gl_PrimitiveID;
in     int   gl_InstanceID;
in     int   gl_InstanceCustomIndexNV;
in    float  gl_HitTNV;
in    uint   gl_HitKindNV;
in    mat4x3 gl_ObjectToWorldNV;
in    mat4x3 gl_WorldToObjectNV;

__glsld_syscmd_require_stage__ rayAnyHit;
void ignoreIntersectionNV();
void terminateRayNV();

__glsld_syscmd_require_stage__ rayIntersect;
bool reportIntersectionNV(float hitT, uint hitKind);

__glsld_syscmd_end_context__;
