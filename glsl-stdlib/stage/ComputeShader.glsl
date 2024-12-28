#if __GLSLD_SHADER_STAGE_COMPUTE

in    highp uvec3 gl_NumWorkGroups;
const highp uvec3 gl_WorkGroupSize = uvec3(1,1,1);
in highp uvec3 gl_WorkGroupID;
in highp uvec3 gl_LocalInvocationID;
in highp uvec3 gl_GlobalInvocationID;
in highp uint gl_LocalInvocationIndex;

#endif