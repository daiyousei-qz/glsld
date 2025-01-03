// FIXME: support SPV_NV_compute_shader_derivatives, aka. derivative functions in compute shader

#if defined(__GLSLD_SHADER_STAGE_FRAGMENT)

float dFdx(float);
vec2 dFdx(vec2);
vec3 dFdx(vec3);
vec4 dFdx(vec4);
float dFdy(float);
vec2 dFdy(vec2);
vec3 dFdy(vec3);
vec4 dFdy(vec4);
float dFdxFine(float p);
vec2  dFdxFine(vec2  p);
vec3  dFdxFine(vec3  p);
vec4  dFdxFine(vec4  p);
float dFdyFine(float p);
vec2  dFdyFine(vec2  p);
vec3  dFdyFine(vec3  p);
vec4  dFdyFine(vec4  p);
float dFdxCoarse(float p);
vec2  dFdxCoarse(vec2  p);
vec3  dFdxCoarse(vec3  p);
vec4  dFdxCoarse(vec4  p);
float dFdyCoarse(float p);
vec2  dFdyCoarse(vec2  p);
vec3  dFdyCoarse(vec3  p);
vec4  dFdyCoarse(vec4  p);
float fwidth(float);
vec2 fwidth(vec2);
vec3 fwidth(vec3);
vec4 fwidth(vec4);
float fwidthFine(float p);
vec2  fwidthFine(vec2  p);
vec3  fwidthFine(vec3  p);
vec4  fwidthFine(vec4  p);
float fwidthCoarse(float p);
vec2  fwidthCoarse(vec2  p);
vec3  fwidthCoarse(vec3  p);
vec4  fwidthCoarse(vec4  p);

#endif

// FIXME: support SPV_NV_compute_shader_derivatives, aka. derivative functions in compute shader???
#if defined(__GLSLD_SHADER_STAGE_FRAGMENT)

// float64_t dFdx(float64_t);
// f64vec2   dFdx(f64vec2);
// f64vec3   dFdx(f64vec3);
// f64vec4   dFdx(f64vec4);
// float64_t dFdy(float64_t);
// f64vec2   dFdy(f64vec2);
// f64vec3   dFdy(f64vec3);
// f64vec4   dFdy(f64vec4);
// float64_t dFdxFine(float64_t);
// f64vec2   dFdxFine(f64vec2);
// f64vec3   dFdxFine(f64vec3);
// f64vec4   dFdxFine(f64vec4);
// float64_t dFdyFine(float64_t);
// f64vec2   dFdyFine(f64vec2);
// f64vec3   dFdyFine(f64vec3);
// f64vec4   dFdyFine(f64vec4);
// float64_t dFdxCoarse(float64_t);
// f64vec2   dFdxCoarse(f64vec2);
// f64vec3   dFdxCoarse(f64vec3);
// f64vec4   dFdxCoarse(f64vec4);
// float64_t dFdyCoarse(float64_t);
// f64vec2   dFdyCoarse(f64vec2);
// f64vec3   dFdyCoarse(f64vec3);
// f64vec4   dFdyCoarse(f64vec4);
// float64_t fwidth(float64_t);
// f64vec2   fwidth(f64vec2);
// f64vec3   fwidth(f64vec3);
// f64vec4   fwidth(f64vec4);
// float64_t fwidthFine(float64_t);
// f64vec2   fwidthFine(f64vec2);
// f64vec3   fwidthFine(f64vec3);
// f64vec4   fwidthFine(f64vec4);
// float64_t fwidthCoarse(float64_t);
// f64vec2   fwidthCoarse(f64vec2);
// f64vec3   fwidthCoarse(f64vec3);
// f64vec4   fwidthCoarse(f64vec4);
// float16_t dFdx(float16_t);
// f16vec2   dFdx(f16vec2);
// f16vec3   dFdx(f16vec3);
// f16vec4   dFdx(f16vec4);
// float16_t dFdy(float16_t);
// f16vec2   dFdy(f16vec2);
// f16vec3   dFdy(f16vec3);
// f16vec4   dFdy(f16vec4);
// float16_t dFdxFine(float16_t);
// f16vec2   dFdxFine(f16vec2);
// f16vec3   dFdxFine(f16vec3);
// f16vec4   dFdxFine(f16vec4);
// float16_t dFdyFine(float16_t);
// f16vec2   dFdyFine(f16vec2);
// f16vec3   dFdyFine(f16vec3);
// f16vec4   dFdyFine(f16vec4);
// float16_t dFdxCoarse(float16_t);
// f16vec2   dFdxCoarse(f16vec2);
// f16vec3   dFdxCoarse(f16vec3);
// f16vec4   dFdxCoarse(f16vec4);
// float16_t dFdyCoarse(float16_t);
// f16vec2   dFdyCoarse(f16vec2);
// f16vec3   dFdyCoarse(f16vec3);
// f16vec4   dFdyCoarse(f16vec4);
// float16_t fwidth(float16_t);
// f16vec2   fwidth(f16vec2);
// f16vec3   fwidth(f16vec3);
// f16vec4   fwidth(f16vec4);
// float16_t fwidthFine(float16_t);
// f16vec2   fwidthFine(f16vec2);
// f16vec3   fwidthFine(f16vec3);
// f16vec4   fwidthFine(f16vec4);
// float16_t fwidthCoarse(float16_t);
// f16vec2   fwidthCoarse(f16vec2);
// f16vec3   fwidthCoarse(f16vec3);
// f16vec4   fwidthCoarse(f16vec4);

#endif