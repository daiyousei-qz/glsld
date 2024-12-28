
#if __GLSLD_FEATURE_ENABLE_MEMORY_SCOPE_SEMANTICS

int atomicAdd(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicAdd(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicMin(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicMin(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicMax(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicMax(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicAnd(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicAnd(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicOr(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicOr(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicXor(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicXor(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicExchange(coherent volatile inout int mem,int data, int scope, int storage, int semantic);
uint atomicExchange(coherent volatile inout uint mem,uint data, int scope, int storage, int semantic);
int atomicCompSwap(coherent volatile inout int mem,int compare,int data, int scope, int storage, int semantic);
uint atomicCompSwap(coherent volatile inout uint mem,uint compare,uint data, int scope, int storage, int semantic);

#endif

#if __GLSLD_FEATURE_ENABLE_MEMORY_SCOPE_SEMANTICS && __GLSLD_FEATURE_ENABLE_INT64_TYPE

uint64_t atomicMin(coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicMin(coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicMax(coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicMax(coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicAnd(coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicAnd(coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicOr (coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicOr (coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicXor(coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicXor(coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicAdd(coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicAdd(coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicExchange(coherent volatile inout uint64_t mem, uint64_t data, int scope, int storage, int semantic);
int64_t atomicExchange(coherent volatile inout  int64_t mem,  int64_t data, int scope, int storage, int semantic);
uint64_t atomicCompSwap(coherent volatile inout uint64_t mem, uint64_t compare, uint64_t data, int scope, int storage, int semantic);
int64_t atomicCompSwap(coherent volatile inout  int64_t mem,  int64_t compare,  int64_t data, int scope, int storage, int semantic);

#endif


#if __GLSLD_FEATURE_ENABLE_MEMORY_SCOPE_SEMANTICS && __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE

float16_t atomicMin(coherent volatile inout float16_t mem, float16_t data, int scope, int storage, int semantic);
float16_t atomicMax(coherent volatile inout float16_t mem, float16_t data, int scope, int storage, int semantic);
float16_t atomicAdd(coherent volatile inout float16_t mem, float16_t data, int scope, int storage, int semantic);
float16_t atomicExchange(coherent volatile inout float16_t mem, float16_t data, int scope, int storage, int semantic);

#endif