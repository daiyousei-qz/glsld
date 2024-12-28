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