// FIXME: correct qualifiers

__glsld_syscmd_begin_context__;
__glsld_syscmd_require_stage__ fragment;

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

__glsld_syscmd_end_context__;
