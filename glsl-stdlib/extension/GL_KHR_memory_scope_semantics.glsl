
__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_KHR_memory_scope_semantics;

const int gl_ScopeDevice      = 1;
const int gl_ScopeWorkgroup   = 2;
const int gl_ScopeSubgroup    = 3;
const int gl_ScopeInvocation  = 4;
const int gl_ScopeQueueFamily = 5;
const int gl_ScopeShaderCallEXT = 6;

const int gl_SemanticsRelaxed         = 0x0;
const int gl_SemanticsAcquire         = 0x2;
const int gl_SemanticsRelease         = 0x4;
const int gl_SemanticsAcquireRelease  = 0x8;
const int gl_SemanticsMakeAvailable   = 0x2000;
const int gl_SemanticsMakeVisible     = 0x4000;
const int gl_SemanticsVolatile        = 0x8000;

const int gl_StorageSemanticsNone     = 0x0;
const int gl_StorageSemanticsBuffer   = 0x40;
const int gl_StorageSemanticsShared   = 0x100;
const int gl_StorageSemanticsImage    = 0x800;
const int gl_StorageSemanticsOutput   = 0x1000;

__glsld_syscmd_end_context__;