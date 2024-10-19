
__glsld_syscmd_begin_context__;
__glsld_syscmd_require_stage__ compute;

in    highp uvec3 gl_NumWorkGroups;
const highp uvec3 gl_WorkGroupSize = uvec3(1,1,1);
in highp uvec3 gl_WorkGroupID;
in highp uvec3 gl_LocalInvocationID;
in highp uvec3 gl_GlobalInvocationID;
in highp uint gl_LocalInvocationIndex;

__glsld_syscmd_end_context__;
