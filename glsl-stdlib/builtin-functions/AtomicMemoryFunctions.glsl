int atomicAdd(coherent volatile inout int mem,int data);
uint atomicAdd(coherent volatile inout uint mem,uint data);
int atomicMin(coherent volatile inout int mem,int data);
uint atomicMin(coherent volatile inout uint mem,uint data);
int atomicMax(coherent volatile inout int mem,int data);
uint atomicMax(coherent volatile inout uint mem,uint data);
int atomicAnd(coherent volatile inout int mem,int data);
uint atomicAnd(coherent volatile inout uint mem,uint data);
int atomicOr(coherent volatile inout int mem,int data);
uint atomicOr(coherent volatile inout uint mem,uint data);
int atomicXor(coherent volatile inout int mem,int data);
uint atomicXor(coherent volatile inout uint mem,uint data);
int atomicExchange(coherent volatile inout int mem,int data);
uint atomicExchange(coherent volatile inout uint mem,uint data);
int atomicCompSwap(coherent volatile inout int mem,int compare,int data);
uint atomicCompSwap(coherent volatile inout uint mem,uint compare,uint data);

__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_EXT_shader_explicit_arithmetic_types_int64;

uint64_t atomicMin(coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicMin(coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicMax(coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicMax(coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicAnd(coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicAnd(coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicOr (coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicOr (coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicXor(coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicXor(coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicAdd(coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicAdd(coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicExchange(coherent volatile inout uint64_t mem, uint64_t data);
int64_t atomicExchange(coherent volatile inout  int64_t mem,  int64_t data);
uint64_t atomicCompSwap(coherent volatile inout uint64_t mem, uint64_t compare, uint64_t data);
int64_t atomicCompSwap(coherent volatile inout  int64_t mem,  int64_t compare,  int64_t data);

__glsld_syscmd_end_context__;

__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_EXT_shader_explicit_arithmetic_types_float16;

float16_t atomicMin(coherent volatile inout float16_t mem, float16_t data);
float16_t atomicMax(coherent volatile inout float16_t mem, float16_t data);
float16_t atomicAdd(coherent volatile inout float16_t mem, float16_t data);
float16_t atomicExchange(coherent volatile inout float16_t mem, float16_t data);

__glsld_syscmd_end_context__;
