int   floatBitsToInt(highp float value);
ivec2 floatBitsToInt(highp vec2  value);
ivec3 floatBitsToInt(highp vec3  value);
ivec4 floatBitsToInt(highp vec4  value);
uint  floatBitsToUint(highp float value);
uvec2 floatBitsToUint(highp vec2  value);
uvec3 floatBitsToUint(highp vec3  value);
uvec4 floatBitsToUint(highp vec4  value);
float intBitsToFloat(highp int   value);
vec2  intBitsToFloat(highp ivec2 value);
vec3  intBitsToFloat(highp ivec3 value);
vec4  intBitsToFloat(highp ivec4 value);
float uintBitsToFloat(highp uint  value);
vec2  uintBitsToFloat(highp uvec2 value);
vec3  uintBitsToFloat(highp uvec3 value);
vec4  uintBitsToFloat(highp uvec4 value);
float frexp(highp float x, out highp int exp);
vec2  frexp(highp vec2 x,  out highp ivec2 exp);
vec3  frexp(highp vec3 x,  out highp ivec3 exp);
vec4  frexp(highp vec4 x,  out highp ivec4 exp);
double  frexp(double x, out int exp);
dvec2  frexp( dvec2 x, out ivec2 exp);
dvec3  frexp( dvec3 x, out ivec3 exp);
dvec4  frexp( dvec4 x, out ivec4 exp);
float ldexp(highp float x, highp int exp);
vec2  ldexp(highp vec2 x,  highp ivec2 exp);
vec3  ldexp(highp vec3 x,  highp ivec3 exp);
vec4  ldexp(highp vec4 x,  highp ivec4 exp);
double ldexp(double x, int exp);
dvec2  ldexp(dvec2 x, ivec2 exp);
dvec3  ldexp(dvec3 x, ivec3 exp);
dvec4  ldexp(dvec4 x, ivec4 exp);

#if __GLSLD_FEATURE_ENABLE_INT64_TYPE

int64_t doubleBitsToInt64(float64_t value);
uint64_t doubleBitsToUint64(float64_t value);
float64_t int64BitsToDouble(int64_t value);
float64_t uint64BitsToDouble(uint64_t value);
i64vec2 doubleBitsToInt64(f64vec2 value);
i64vec3 doubleBitsToInt64(f64vec3 value);
i64vec4 doubleBitsToInt64(f64vec4 value);
f64vec2  int64BitsToDouble(i64vec2 value);
f64vec3  int64BitsToDouble(i64vec3 value);
f64vec4  int64BitsToDouble(i64vec4 value);
u64vec2  doubleBitsToUint64(f64vec2 value);
u64vec3  doubleBitsToUint64(f64vec3 value);
u64vec4  doubleBitsToUint64(f64vec4 value);
f64vec2  uint64BitsToDouble(u64vec2 value);
f64vec3  uint64BitsToDouble(u64vec3 value);
f64vec4  uint64BitsToDouble(u64vec4 value);

#endif

#if __GLSLD_FEATURE_ENABLE_INT16_TYPE && __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE

int16_t halfBitsToInt16(float16_t value);
uint16_t halfBitsToUint16(float16_t value);
int16_t float16BitsToInt16(float16_t value);
uint16_t float16BitsToUint16(float16_t value);
float16_t int16BitsToFloat16(int16_t value);
float16_t uint16BitsToFloat16(uint16_t value);
float16_t int16BitsToHalf(int16_t value);
float16_t uint16BitsToHalf(uint16_t value);
i16vec2 halfBitsToInt16(f16vec2 value);
i16vec3 halhBitsToInt16(f16vec3 value);
i16vec4 halfBitsToInt16(f16vec4 value);
i16vec2 float16BitsToInt16(f16vec2 value);
i16vec3 float16BitsToInt16(f16vec3 value);
i16vec4 float16BitsToInt16(f16vec4 value);
f16vec2   int16BitsToFloat16(i16vec2 value);
f16vec3   int16BitsToFloat16(i16vec3 value);
f16vec4   int16BitsToFloat16(i16vec4 value);
f16vec2   int16BitsToHalf(i16vec2 value);
f16vec3   int16BitsToHalf(i16vec3 value);
f16vec4   int16BitsToHalf(i16vec4 value);
u16vec2  halfBitsToUint16(f16vec2 value);
u16vec3  halfBitsToUint16(f16vec3 value);
u16vec4  halfBitsToUint16(f16vec4 value);
u16vec2  float16BitsToUint16(f16vec2 value);
u16vec3  float16BitsToUint16(f16vec3 value);
u16vec4  float16BitsToUint16(f16vec4 value);
f16vec2   uint16BitsToFloat16(u16vec2 value);
f16vec3   uint16BitsToFloat16(u16vec3 value);
f16vec4   uint16BitsToFloat16(u16vec4 value);
f16vec2   uint16BitsToHalf(u16vec2 value);
f16vec3   uint16BitsToHalf(u16vec3 value);
f16vec4   uint16BitsToHalf(u16vec4 value);
float16_t frexp(float16_t x, out int16_t exp);
f16vec2   frexp(f16vec2 x,   out i16vec2 exp);
f16vec3   frexp(f16vec3 x,   out i16vec3 exp);
f16vec4   frexp(f16vec4 x,   out i16vec4 exp);
float16_t frexp(float16_t x, out int exp);
f16vec2   frexp(f16vec2 x,   out ivec2 exp);
f16vec3   frexp(f16vec3 x,   out ivec3 exp);
f16vec4   frexp(f16vec4 x,   out ivec4 exp);
float16_t ldexp(float16_t x, int16_t exp);
f16vec2   ldexp(f16vec2 x,   i16vec2 exp);
f16vec3   ldexp(f16vec3 x,   i16vec3 exp);
f16vec4   ldexp(f16vec4 x,   i16vec4 exp);
float16_t ldexp(float16_t x, int exp);
f16vec2   ldexp(f16vec2 x,   ivec2 exp);
f16vec3   ldexp(f16vec3 x,   ivec3 exp);
f16vec4   ldexp(f16vec4 x,   ivec4 exp);

#endif