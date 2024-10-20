__glsld_syscmd_begin_context__;
__glsld_syscmd_require_stage__ tessControl;

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

__glsld_syscmd_end_context__;
