
__glsld_syscmd_begin_context__;
__glsld_syscmd_require_stage__ fragment;

float interpolateAtCentroid(float interpolant);
vec2  interpolateAtCentroid(vec2 interpolant);
vec3  interpolateAtCentroid(vec3 interpolant);
vec4  interpolateAtCentroid(vec4 interpolant);
float interpolateAtSample(float interpolant, int sample_);
vec2  interpolateAtSample(vec2 interpolant,  int sample_);
vec3  interpolateAtSample(vec3 interpolant,  int sample_);
vec4  interpolateAtSample(vec4 interpolant,  int sample_);
float interpolateAtOffset(float interpolant, vec2 offset);
vec2  interpolateAtOffset(vec2 interpolant,  vec2 offset);
vec3  interpolateAtOffset(vec3 interpolant,  vec2 offset);
vec4  interpolateAtOffset(vec4 interpolant,  vec2 offset);

__glsld_syscmd_end_context__;

// float64_t interpolateAtCentroid(float64_t interpolant);
// f64vec2   interpolateAtCentroid(f64vec2 interpolant);
// f64vec3   interpolateAtCentroid(f64vec3 interpolant);
// f64vec4   interpolateAtCentroid(f64vec4 interpolant);
// float64_t interpolateAtSample(float64_t interpolant, int sample_);
// f64vec2   interpolateAtSample(f64vec2 interpolant,   int sample_);
// f64vec3   interpolateAtSample(f64vec3 interpolant,   int sample_);
// f64vec4   interpolateAtSample(f64vec4 interpolant,   int sample_);
// float64_t interpolateAtOffset(float64_t interpolant, f64vec2 offset);
// f64vec2   interpolateAtOffset(f64vec2 interpolant,   f64vec2 offset);
// f64vec3   interpolateAtOffset(f64vec3 interpolant,   f64vec2 offset);
// f64vec4   interpolateAtOffset(f64vec4 interpolant,   f64vec2 offset);

__glsld_syscmd_begin_context__;
__glsld_syscmd_require_stage__ fragment;
__glsld_syscmd_require_extension__ GL_EXT_shader_explicit_arithmetic_types_float16;

float16_t interpolateAtCentroid(float16_t interpolant);
f16vec2   interpolateAtCentroid(f16vec2 interpolant);
f16vec3   interpolateAtCentroid(f16vec3 interpolant);
f16vec4   interpolateAtCentroid(f16vec4 interpolant);
float16_t interpolateAtSample(float16_t interpolant, int sample_);
f16vec2   interpolateAtSample(f16vec2 interpolant,   int sample_);
f16vec3   interpolateAtSample(f16vec3 interpolant,   int sample_);
f16vec4   interpolateAtSample(f16vec4 interpolant,   int sample_);
float16_t interpolateAtOffset(float16_t interpolant, f16vec2 offset);
f16vec2   interpolateAtOffset(f16vec2 interpolant,   f16vec2 offset);
f16vec3   interpolateAtOffset(f16vec3 interpolant,   f16vec2 offset);
f16vec4   interpolateAtOffset(f16vec4 interpolant,   f16vec2 offset);

__glsld_syscmd_end_context__;