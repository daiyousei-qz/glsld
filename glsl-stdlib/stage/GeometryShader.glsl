#if __GLSLD_SHADER_STAGE_GEOMETRY

void EmitStreamVertex(int stream);
void EndStreamPrimitive(int stream);
void EmitVertex();
void EndPrimitive();

in gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
    // vec4 gl_SecondaryPositionNV;
    // vec4 gl_PositionPerViewNV[];
} gl_in[];
in int gl_PrimitiveIDIn;
out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};
out int gl_PrimitiveID;
out int gl_Layer;
out int gl_ViewportIndex;

#endif