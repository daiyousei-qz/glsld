__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_EXT_ray_tracing;

__glsld_syscmd_require_stage__ rayGen rayAnyHit rayClosestHit rayIntersect rayMiss rayCallable;
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

__glsld_syscmd_require_stage__ rayGen rayClosestHit rayMiss rayCallable;
void executeCallableEXT(uint sbtRecordIndex, int callable);

__glsld_syscmd_require_stage__ rayGen rayClosestHit rayMiss;
void traceRayEXT(accelerationStructureEXT topLevel,uint rayFlags,uint cullMask,uint sbtRecordOffset,uint sbtRecordStride,uint missIndex,vec3 origin,float tMin,vec3 direction,float tMax,int payload);

__glsld_syscmd_require_stage__ rayAnyHit rayClosestHit rayIntersect rayMiss;
in    vec3   gl_WorldRayOriginEXT;
in    vec3   gl_WorldRayDirectionEXT;
in    float  gl_RayTminEXT;
in    float  gl_RayTmaxEXT;
in    uint   gl_IncomingRayFlagsEXT;
in    uint   gl_CullMaskEXT;

__glsld_syscmd_require_stage__ rayAnyHit rayClosestHit rayIntersect;
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

__glsld_syscmd_require_stage__ rayIntersect;
bool reportIntersectionEXT(float hitT, uint hitKind);

__glsld_syscmd_end_context__;
