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