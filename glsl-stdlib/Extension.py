from StdlibBuilder import StdlibBuilder

def addBuiltinFor_GL_EXT_ray_query(builder: StdlibBuilder):
    builder.addSnippet("""
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
""")

def addBuiltinFor_GL_KHR_memory_scope_semantics(builder: StdlibBuilder):
    builder.addSnippet("""
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
""")

def addBuiltinFor_GL_NV_shader_sm_builtins(builder: StdlibBuilder):
    builder.addSnippet("""
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
""")
