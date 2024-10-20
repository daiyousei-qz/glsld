__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_NV_shader_sm_builtins;

__glsld_syscmd_require_stage__ vertex geometry tessControl tessEval compute task mesh;
in highp   uint  gl_WarpsPerSMNV;
in highp   uint  gl_SMCountNV;
in highp   uint  gl_WarpIDNV;
in highp   uint  gl_SMIDNV;

__glsld_syscmd_require_stage__ fragment;
flat in highp   uint  gl_WarpsPerSMNV;
flat in highp   uint  gl_SMCountNV;
flat in highp   uint  gl_WarpIDNV;
flat in highp   uint  gl_SMIDNV;

__glsld_syscmd_require_stage__ rayGen rayAnyHit rayClosestHit rayIntersect rayMiss rayCallable;
in highp    uint  gl_WarpsPerSMNV;
in highp    uint  gl_SMCountNV;
in highp volatile uint  gl_WarpIDNV;
in highp volatile uint  gl_SMIDNV;

__glsld_syscmd_end_context__;
