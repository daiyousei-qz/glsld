
__glsld_syscmd_begin_context__;
__glsld_syscmd_require_target__ opengl;
__glsld_syscmd_require_stage__ vertex;

in int gl_VertexID;
in int gl_InstanceID;

__glsld_syscmd_end_context__;


__glsld_syscmd_begin_context__;
__glsld_syscmd_require_target__ vulkan;
__glsld_syscmd_require_stage__ vertex;

in int gl_VertexIndex;
in int gl_InstanceIndex;

__glsld_syscmd_end_context__;

__glsld_syscmd_begin_context__;
__glsld_syscmd_require_stage__ vertex;

in int gl_DrawID;
in int gl_BaseVertex;
in int gl_BaseInstance;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};

__glsld_syscmd_end_context__;
