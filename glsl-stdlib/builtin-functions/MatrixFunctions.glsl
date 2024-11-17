mat2 matrixCompMult(mat2 x, mat2 y);
mat3 matrixCompMult(mat3 x, mat3 y);
mat4 matrixCompMult(mat4 x, mat4 y);
mat2x3 matrixCompMult(mat2x3 x, mat2x3 y);
mat2x4 matrixCompMult(mat2x4 x, mat2x4 y);
mat3x2 matrixCompMult(mat3x2 x, mat3x2 y);
mat3x4 matrixCompMult(mat3x4 x, mat3x4 y);
mat4x2 matrixCompMult(mat4x2 x, mat4x2 y);
mat4x3 matrixCompMult(mat4x3 x, mat4x3 y);
dmat2 matrixCompMult(dmat2 x, dmat2 y);
dmat3 matrixCompMult(dmat3 x, dmat3 y);
dmat4 matrixCompMult(dmat4 x, dmat4 y);
dmat2x3 matrixCompMult(dmat2x3 x, dmat2x3 y);
dmat2x4 matrixCompMult(dmat2x4 x, dmat2x4 y);
dmat3x2 matrixCompMult(dmat3x2 x, dmat3x2 y);
dmat3x4 matrixCompMult(dmat3x4 x, dmat3x4 y);
dmat4x2 matrixCompMult(dmat4x2 x, dmat4x2 y);
dmat4x3 matrixCompMult(dmat4x3 x, dmat4x3 y);
mat2   outerProduct(vec2 c, vec2 r);
mat3   outerProduct(vec3 c, vec3 r);
mat4   outerProduct(vec4 c, vec4 r);
mat2x3 outerProduct(vec3 c, vec2 r);
mat3x2 outerProduct(vec2 c, vec3 r);
mat2x4 outerProduct(vec4 c, vec2 r);
mat4x2 outerProduct(vec2 c, vec4 r);
mat3x4 outerProduct(vec4 c, vec3 r);
mat4x3 outerProduct(vec3 c, vec4 r);
dmat2   outerProduct(dvec2 c, dvec2 r);
dmat3   outerProduct(dvec3 c, dvec3 r);
dmat4   outerProduct(dvec4 c, dvec4 r);
dmat2x3 outerProduct(dvec3 c, dvec2 r);
dmat3x2 outerProduct(dvec2 c, dvec3 r);
dmat2x4 outerProduct(dvec4 c, dvec2 r);
dmat4x2 outerProduct(dvec2 c, dvec4 r);
dmat3x4 outerProduct(dvec4 c, dvec3 r);
dmat4x3 outerProduct(dvec3 c, dvec4 r);
mat2   transpose(mat2   m);
mat3   transpose(mat3   m);
mat4   transpose(mat4   m);
mat2x3 transpose(mat3x2 m);
mat3x2 transpose(mat2x3 m);
mat2x4 transpose(mat4x2 m);
mat4x2 transpose(mat2x4 m);
mat3x4 transpose(mat4x3 m);
mat4x3 transpose(mat3x4 m);
dmat2   transpose(dmat2 m);
dmat3   transpose(dmat3 m);
dmat4   transpose(dmat4 m);
dmat2x3 transpose(dmat3x2 m);
dmat3x2 transpose(dmat2x3 m);
dmat2x4 transpose(dmat4x2 m);
dmat4x2 transpose(dmat2x4 m);
dmat3x4 transpose(dmat4x3 m);
dmat4x3 transpose(dmat3x4 m);
float determinant(mat2 m);
float determinant(mat3 m);
float determinant(mat4 m);
double determinant(dmat2 m);
double determinant(dmat3 m);
double determinant(dmat4 m);
mat2 inverse(mat2 m);
mat3 inverse(mat3 m);
mat4 inverse(mat4 m);
dmat2 inverse(dmat2 m);
dmat3 inverse(dmat3 m);
dmat4 inverse(dmat4 m);


__glsld_syscmd_begin_context__;
__glsld_syscmd_require_extension__ GL_EXT_shader_explicit_arithmetic_types_float16;

f16mat2   matrixCompMult(f16mat2 x,   f16mat2 y);
f16mat3   matrixCompMult(f16mat3 x,   f16mat3 y);
f16mat4   matrixCompMult(f16mat4 x,   f16mat4 y);
f16mat2x3 matrixCompMult(f16mat2x3 x, f16mat2x3 y);
f16mat2x4 matrixCompMult(f16mat2x4 x, f16mat2x4 y);
f16mat3x2 matrixCompMult(f16mat3x2 x, f16mat3x2 y);
f16mat3x4 matrixCompMult(f16mat3x4 x, f16mat3x4 y);
f16mat4x2 matrixCompMult(f16mat4x2 x, f16mat4x2 y);
f16mat4x3 matrixCompMult(f16mat4x3 x, f16mat4x3 y);
f16mat2   outerProduct(f16vec2 c, f16vec2 r);
f16mat3   outerProduct(f16vec3 c, f16vec3 r);
f16mat4   outerProduct(f16vec4 c, f16vec4 r);
f16mat2x3 outerProduct(f16vec3 c, f16vec2 r);
f16mat3x2 outerProduct(f16vec2 c, f16vec3 r);
f16mat2x4 outerProduct(f16vec4 c, f16vec2 r);
f16mat4x2 outerProduct(f16vec2 c, f16vec4 r);
f16mat3x4 outerProduct(f16vec4 c, f16vec3 r);
f16mat4x3 outerProduct(f16vec3 c, f16vec4 r);
f16mat2   transpose(f16mat2 m);
f16mat3   transpose(f16mat3 m);
f16mat4   transpose(f16mat4 m);
f16mat2x3 transpose(f16mat3x2 m);
f16mat3x2 transpose(f16mat2x3 m);
f16mat2x4 transpose(f16mat4x2 m);
f16mat4x2 transpose(f16mat2x4 m);
f16mat3x4 transpose(f16mat4x3 m);
f16mat4x3 transpose(f16mat3x4 m);
float16_t determinant(f16mat2 m);
float16_t determinant(f16mat3 m);
float16_t determinant(f16mat4 m);
f16mat2 inverse(f16mat2 m);
f16mat3 inverse(f16mat3 m);
f16mat4 inverse(f16mat4 m);

__glsld_syscmd_end_context__;