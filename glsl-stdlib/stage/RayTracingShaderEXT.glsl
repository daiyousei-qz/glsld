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