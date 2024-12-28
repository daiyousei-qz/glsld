#if __GLSLD_SHADER_STAGE_VERTEX

// FIXME: support these
#if OPENGL
in int gl_VertexID;
in int gl_InstanceID;
#endif


// FIXME: support these
#if VULKAN
in int gl_VertexIndex;
in int gl_InstanceIndex;
#endif

in int gl_DrawID;
in int gl_BaseVertex;
in int gl_BaseInstance;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};

#endif