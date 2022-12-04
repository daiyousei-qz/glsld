LibraryFunctions = [
    # Angle and Trigonometry Functions
    "genFType radians(genFType degrees)",
    "genFType degrees(genFType radians)",
    "genFType sin(genFType angle)",
    "genFType cos(genFType angle)",
    "genFType tan(genFType angle)",
    "genFType asin(genFType x)",
    "genFType acos(genFType x)",
    "genFType atan(genFType y, genFType x)",
    "genFType atan(genFType y_over_x)",
    "genFType sinh(genFType x)",
    "genFType cosh(genFType x)",
    "genFType tanh(genFType x)",
    "genFType asinh(genFType x)",
    "genFType acosh(genFType x)",
    "genFType atanh(genFType x)",

    # Exponential Functions
    "genFType pow(genFType x, genFType y)",
    "genFType exp(genFType x)",
    "genFType log(genFType x)",
    "genFType exp2(genFType x)",
    "genFType log2(genFType x)",
    "genFType sqrt(genFType x)",
    "genDType sqrt(genDType x)",
    "genFType inversesqrt(genFType x)",
    "genDType inversesqrt(genDType x)",

    # Common Functions
    "genFType abs(genFType x)",
    "genIType abs(genIType x)",
    "genDType abs(genDType x)",
    "genFType sign(genFType x)",
    "genIType sign(genIType x)",
    "genDType sign(genDType x)",
    "genFType floor(genFType x)",
    "genDType floor(genDType x)",
    "genFType trunc(genFType x)",
    "genDType trunc(genDType x)",
    "genFType round(genFType x)",
    "genDType round(genDType x)",
    "genFType roundEven(genFType x)",
    "genDType roundEven(genDType x)",
    "genFType ceil(genFType x)",
    "genDType ceil(genDType x)",
    "genFType fract(genFType x)",
    "genDType fract(genDType x)",
    "genFType mod(genFType x, float y)",
    "genFType mod(genFType x, genFType y)",
    "genDType mod(genDType x, double y)",
    "genDType mod(genDType x, genDType y)",
    "genFType modf(genFType x, out genFType i)",
    "genDType modf(genDType x, out genDType i)",
    "genFType min(genFType x, genFType y)",
    "genFType min(genFType x, float y)",
    "genDType min(genDType x, genDType y)",
    "genDType min(genDType x, double y)",
    "genIType min(genIType x, genIType y)",
    "genIType min(genIType x, int y)",
    "genUType min(genUType x, genUType y)",
    "genUType min(genUType x, uint y)",
    "genFType max(genFType x, genFType y)",
    "genFType max(genFType x, float y)",
    "genDType max(genDType x, genDType y)",
    "genDType max(genDType x, double y)",
    "genIType max(genIType x, genIType y)",
    "genIType max(genIType x, int y)",
    "genUType max(genUType x, genUType y)",
    "genUType max(genUType x, uint y)",
    "genFType clamp(genFType x, genFType minVal, genFType maxVal)",
    "genFType clamp(genFType x, float minVal, float maxVal)",
    "genDType clamp(genDType x, genDType minVal, genDType maxVal)",
    "genDType clamp(genDType x, double minVal, double maxVal)",
    "genIType clamp(genIType x, genIType minVal, genIType maxVal)",
    "genIType clamp(genIType x, int minVal, int maxVal)",
    "genUType clamp(genUType x, genUType minVal, genUType maxVal)",
    "genUType clamp(genUType x, uint minVal, uint maxVal)",
    "genFType mix(genFType x, genFType y, genFType a)",
    "genFType mix(genFType x, genFType y, float a)",
    "genDType mix(genDType x, genDType y, genDType a)",
    "genDType mix(genDType x, genDType y, double a)",
    "genFType mix(genFType x, genFType y, genBType a)",
    "genDType mix(genDType x, genDType y, genBType a)",
    "genIType mix(genIType x, genIType y, genBType a)",
    "genUType mix(genUType x, genUType y, genBType a)",
    "genBType mix(genBType x, genBType y, genBType a)",
    "genFType step(genFType edge, genFType x)",
    "genFType step(float edge, genFType x)",
    "genDType step(genDType edge, genDType x)",
    "genDType step(double edge, genDType x)",
    "genFType smoothstep(genFType edge0, genFType edge1, genFType x)",
    "genFType smoothstep(float edge0, float edge1, genFType x)",
    "genDType smoothstep(genDType edge0, genDType edge1, genDType x)",
    "genDType smoothstep(double edge0, double edge1, genDType x)",
    "genBType isnan(genFType x)",
    "genBType isnan(genDType x)",
    "genBType isinf(genFType x)",
    "genBType isinf(genDType x)",
    "genIType floatBitsToInt(highp genFType value)",
    "genUType floatBitsToUint(highp genFType value)",
    "genFType intBitsToFloat(highp genIType value)",
    "genFType uintBitsToFloat(highp genUType value)",
    "genFType fma(genFType a, genFType b, genFType c)",
    "genDType fma(genDType a, genDType b, genDType c)",
    "genFType frexp(highp genFType x, out highp genIType exp)",
    "genDType frexp(genDType x, out genIType exp)",
    "genFType ldexp(highp genFType x, highp genIType exp)",
    "genDType ldexp(genDType x, genIType exp)",

    # Floating-Point Pack and Unpack Functions
    "highp uint packUnorm2x16(vec2 v)",
    "highp uint packSnorm2x16(vec2 v)",
    "uint packUnorm4x8(vec4 v)",
    "uint packSnorm4x8(vec4 v)",
    "vec2 unpackUnorm2x16(highp uint p)",
    "vec2 unpackSnorm2x16(highp uint p)",
    "vec4 unpackUnorm4x8(highp uint p)",
    "vec4 unpackSnorm4x8(highp uint p)",
    "uint packHalf2x16(vec2 v)",
    "vec2 unpackHalf2x16( uint v)",
    "double packDouble2x32(uvec2 v)",
    "uvec2 unpackDouble2x32(double v)",

    # Geometric Functions
    "float length(genFType x)",
    "double length(genDType x)",
    "float distance(genFType p0, genFType p1)",
    "double distance(genDType p0, genDType p1)",
    "float dot(genFType x, genFType y)",
    "double dot(genDType x, genDType y)",
    "vec3 cross(vec3 x, vec3 y)",
    "dvec3 cross(dvec3 x, dvec3 y)",
    "genFType normalize(genFType x)",
    "genDType normalize(genDType x)",
    "vec4 ftransform()",  # compatibility only
    "genFType faceforward(genFType N, genFType I, genFType Nref)",
    "genDType faceforward(genDType N, genDType I, genDType Nref)",
    "genFType reflect(genFType I, genFType N)",
    "genDType reflect(genDType I, genDType N)",
    "genFType refract(genFType I, genFType N, float eta)",
    "genDType refract(genDType I, genDType N, double eta)",

    # Matrix Functions
    "mat matrixCompMult(mat x, mat y)",
    "mat2 outerProduct(vec2 c, vec2 r)",
    "mat3 outerProduct(vec3 c, vec3 r)",
    "mat4 outerProduct(vec4 c, vec4 r)",
    "mat2x3 outerProduct(vec3 c, vec2 r)",
    "mat3x2 outerProduct(vec2 c, vec3 r)",
    "mat2x4 outerProduct(vec4 c, vec2 r)",
    "mat4x2 outerProduct(vec2 c, vec4 r)",
    "mat3x4 outerProduct(vec4 c, vec3 r)",
    "mat4x3 outerProduct(vec3 c, vec4 r)",
    "mat2 transpose(mat2 m)",
    "mat3 transpose(mat3 m)",
    "mat4 transpose(mat4 m)",
    "mat2x3 transpose(mat3x2 m)",
    "mat3x2 transpose(mat2x3 m)",
    "mat2x4 transpose(mat4x2 m)",
    "mat4x2 transpose(mat2x4 m)",
    "mat3x4 transpose(mat4x3 m)",
    "mat4x3 transpose(mat3x4 m)",
    "float determinant(mat2 m)",
    "float determinant(mat3 m)",
    "float determinant(mat4 m)",
    "mat2 inverse(mat2 m)",
    "mat3 inverse(mat3 m)",
    "mat4 inverse(mat4 m)",

    # Matrix Functions (double)
    "dmat matrixCompMult(dmat x, dmat y)",
    "dmat2 outerProduct(dvec2 c, dvec2 r)",
    "dmat3 outerProduct(dvec3 c, dvec3 r)",
    "dmat4 outerProduct(dvec4 c, dvec4 r)",
    "dmat2x3 outerProduct(dvec3 c, dvec2 r)",
    "dmat3x2 outerProduct(dvec2 c, dvec3 r)",
    "dmat2x4 outerProduct(dvec4 c, dvec2 r)",
    "dmat4x2 outerProduct(dvec2 c, dvec4 r)",
    "dmat3x4 outerProduct(dvec4 c, dvec3 r)",
    "dmat4x3 outerProduct(dvec3 c, dvec4 r)",
    "dmat2 transpose(dmat2 m)",
    "dmat3 transpose(dmat3 m)",
    "dmat4 transpose(dmat4 m)",
    "dmat2x3 transpose(dmat3x2 m)",
    "dmat3x2 transpose(dmat2x3 m)",
    "dmat2x4 transpose(dmat4x2 m)",
    "dmat4x2 transpose(dmat2x4 m)",
    "dmat3x4 transpose(dmat4x3 m)",
    "dmat4x3 transpose(dmat3x4 m)",
    "double determinant(dmat2 m)",
    "double determinant(dmat3 m)",
    "double determinant(dmat4 m)",
    "dmat2 inverse(dmat2 m)",
    "dmat3 inverse(dmat3 m)",
    "dmat4 inverse(dmat4 m)",

    # Vector Relational Functions
    "bvec lessThan(vec x, vec y)",
    "bvec lessThan(ivec x, ivec y)",
    "bvec lessThan(uvec x, uvec y)",
    "bvec lessThanEqual(vec x, vec y)",
    "bvec lessThanEqual(ivec x, ivec y)",
    "bvec lessThanEqual(uvec x, uvec y)",
    "bvec greaterThan(vec x, vec y)",
    "bvec greaterThan(ivec x, ivec y)",
    "bvec greaterThan(uvec x, uvec y)",
    "bvec greaterThanEqual(vec x, vec y)",
    "bvec greaterThanEqual(ivec x, ivec y)",
    "bvec greaterThanEqual(uvec x, uvec y)",
    "bvec equal(vec x, vec y)",
    "bvec equal(ivec x, ivec y)",
    "bvec equal(uvec x, uvec y)",
    "bvec equal(bvec x, bvec y)",
    "bvec notEqual(vec x, vec y)",
    "bvec notEqual(ivec x, ivec y)",
    "bvec notEqual(uvec x, uvec y)",
    "bvec notEqual(bvec x, bvec y)",
    "bool any(bvec x)",
    "bool all(bvec x)",
    "bvec not(bvec x)",

    # Integer Functions
    "genUType uaddCarry(highp genUType x, highp genUType y, out lowp genUType carry)",
    "genUType usubBorrow(highp genUType x, highp genUType y, out lowp genUType borrow)",
    "void umulExtended(highp genUType x, highp genUType y, out highp genUType msb, out highp genUType lsb)",
    "void imulExtended(highp genIType x, highp genIType y, out highp genIType msb, out highp genIType lsb)",
    "genIType bitfieldExtract(genIType value, int offset, int bits)",
    "genUType bitfieldExtract(genUType value, int offset, int bits)",
    "genIType bitfieldInsert(genIType base, genIType insert, int offset, int bits)",
    "genUType bitfieldInsert(genUType base, genUType insert, int offset, int bits)",
    "genIType bitfieldReverse(highp genIType value)",
    "genUType bitfieldReverse(highp genUType value)",
    "genIType bitCount(genIType value)",
    "genIType bitCount(genUType value)",
    "genIType findLSB(genIType value)",
    "genIType findLSB(genUType value)",
    "genIType findMSB(highp genIType value)",
    "genIType findMSB(highp genUType value)",

    # Texture Query Functions
    "int textureSize(gsampler1D sampler, int lod)",
    "ivec2 textureSize(gsampler2D sampler, int lod)",
    "ivec3 textureSize(gsampler3D sampler, int lod)",
    "ivec2 textureSize(gsamplerCube sampler, int lod)",
    "int textureSize(sampler1DShadow sampler, int lod)",
    "ivec2 textureSize(sampler2DShadow sampler, int lod)",
    "ivec2 textureSize(samplerCubeShadow sampler, int lod) ivec3 textureSize(gsamplerCubeArray sampler, int lod)",
    "ivec3 textureSize(samplerCubeArrayShadow sampler, int lod)",
    "ivec2 textureSize(gsampler2DRect sampler)",
    "ivec2 textureSize(sampler2DRectShadow sampler)",
    "ivec2 textureSize(gsampler1DArray sampler, int lod)",
    "ivec2 textureSize(sampler1DArrayShadow sampler, int lod)",
    "ivec3 textureSize(gsampler2DArray sampler, int lod)",
    "ivec3 textureSize(sampler2DArrayShadow sampler, int lod)",
    "int textureSize(gsamplerBuffer sampler)",
    "ivec2 textureSize(gsampler2DMS sampler)",
    "ivec3 textureSize(gsampler2DMSArray sampler)",
    "vec2 textureQueryLod(gsampler1D sampler, float P)",
    "vec2 textureQueryLod(gsampler2D sampler, vec2 P)",
    "vec2 textureQueryLod(gsampler3D sampler, vec3 P)",
    "vec2 textureQueryLod(gsamplerCube sampler, vec3 P)",
    "vec2 textureQueryLod(gsampler1DArray sampler, float P)",
    "vec2 textureQueryLod(gsampler2DArray sampler, vec2 P)",
    "vec2 textureQueryLod(gsamplerCubeArray sampler, vec3 P)",
    "vec2 textureQueryLod(sampler1DShadow sampler, float P)",
    "vec2 textureQueryLod(sampler2DShadow sampler, vec2 P)",
    "vec2 textureQueryLod(samplerCubeShadow sampler, vec3 P)",
    "vec2 textureQueryLod(sampler1DArrayShadow sampler, float P)",
    "vec2 textureQueryLod(sampler2DArrayShadow sampler, vec2 P)",
    "vec2 textureQueryLod(samplerCubeArrayShadow sampler, vec3 P)",
    "int textureQueryLevels(gsampler1D sampler)",
    "int textureQueryLevels(gsampler2D sampler)",
    "int textureQueryLevels(gsampler3D sampler)",
    "int textureQueryLevels(gsamplerCube sampler)",
    "int textureQueryLevels(gsampler1DArray sampler)",
    "int textureQueryLevels(gsampler2DArray sampler)",
    "int textureQueryLevels(gsamplerCubeArray sampler)",
    "int textureQueryLevels(sampler1DShadow sampler)",
    "int textureQueryLevels(sampler2DShadow sampler)",
    "int textureQueryLevels(samplerCubeShadow sampler)",
    "int textureQueryLevels(sampler1DArrayShadow sampler)",
    "int textureQueryLevels(sampler2DArrayShadow sampler)",
    "int textureQueryLevels(samplerCubeArrayShadow sampler)",
    "int textureSamples(gsampler2DMS sampler)",
    "int textureSamples(gsampler2DMSArray sampler)",

    # Texel Lookup Functions
    "gvec4 texture(gsampler1D sampler, float P [, float bias] )",
    "gvec4 texture(gsampler2D sampler, vec2 P [, float bias] )",
    "gvec4 texture(gsampler3D sampler, vec3 P [, float bias] )",
    "gvec4 texture(gsamplerCube sampler, vec3 P[, float bias] )",
    "float texture(sampler1DShadow sampler, vec3 P [, float bias])",
    "float texture(sampler2DShadow sampler, vec3 P [, float bias])",
    "float texture(samplerCubeShadow sampler, vec4 P [, float bias] )",
    "gvec4 texture(gsampler2DArray sampler, vec3 P [, float bias] )",
    "gvec4 texture(gsamplerCubeArray sampler, vec4 P [, float bias] )",
    "gvec4 texture(gsampler1DArray sampler, vec2 P [, float bias] )",
    "float texture(sampler1DArrayShadow sampler, vec3 P [, float bias] )",
    "float texture(sampler2DArrayShadow sampler, vec4 P)",
    "gvec4 texture(gsampler2DRect sampler, vec2 P)",
    "float texture(sampler2DRectShadow sampler, vec3 P)",
    "float texture(samplerCubeArrayShadow sampler, vec4 P, float compare)",
    "gvec4 textureProj(gsampler1D sampler, vec2 P [, float bias] )",
    "gvec4 textureProj(gsampler1D sampler, vec4 P [, float bias] )",
    "gvec4 textureProj(gsampler2D sampler, vec3 P [, float bias] )",
    "gvec4 textureProj(gsampler2D sampler, vec4 P [, float bias] )",
    "gvec4 textureProj(gsampler3D sampler, vec4 P [, float bias] )",
    "float textureProj(sampler1DShadow sampler, vec4 P [, float bias] )",
    "float textureProj(sampler2DShadow sampler, vec4 P [, float bias] )",
    "gvec4 textureProj(gsampler2DRect sampler, vec3 P)",
    "gvec4 textureProj(gsampler2DRect sampler, vec4 P)",
    "float textureProj(sampler2DRectShadow sampler, vec4 P)",
    "gvec4 textureLod(gsampler1D sampler, float P, float lod)",
    "gvec4 textureLod(gsampler2D sampler, vec2 P, float lod)",
    "gvec4 textureLod(gsampler3D sampler, vec3 P, float lod)",
    "gvec4 textureLod(gsamplerCube sampler, vec3 P, float lod)",
    "float textureLod(sampler2DShadow sampler, vec3 P, float lod)",
    "float textureLod(sampler1DShadow sampler, vec3 P, float lod)",
    "gvec4 textureLod(gsampler1DArray sampler, vec2 P, float lod)",
    "float textureLod(sampler1DArrayShadow sampler, vec3 P, float lod)",
    "gvec4 textureLod(gsampler2DArray sampler, vec3 P, float lod)",
    "gvec4 textureLod(gsamplerCubeArray sampler, vec4 P, float lod)",
    "gvec4 textureOffset(gsampler1D sampler, float P, int offset [, float bias] )",
    "gvec4 textureOffset(gsampler2D sampler, vec2 P, ivec2 offset [, float bias] )",
    "gvec4 textureOffset(gsampler3D sampler, vec3 P, ivec3 offset [, float bias] )",
    "float textureOffset(sampler2DShadow sampler, vec3 P, ivec2 offset [, float bias] )",
    "gvec4 textureOffset(gsampler2DRect sampler, vec2 P, ivec2 offset)",
    "float textureOffset(sampler2DRectShadow sampler, vec3 P, ivec2 offset)",
    "float textureOffset(sampler1DShadow sampler, vec3 P, int offset [, float bias] )",
    "gvec4 textureOffset(gsampler1DArray sampler, vec2 P, int offset [, float bias] )",
    "gvec4 textureOffset(gsampler2DArray sampler, vec3 P, ivec2 offset [, float bias] )",
    "float textureOffset(sampler1DArrayShadow sampler, vec3 P, int offset [, float bias] )",
    "float textureOffset(sampler2DArrayShadow sampler, vec4 P, ivec2 offset)",
    "gvec4 texelFetch(gsampler1D sampler, int P, int lod)",
    "gvec4 texelFetch(gsampler2D sampler, ivec2 P, int lod)",
    "gvec4 texelFetch(gsampler3D sampler, ivec3 P, int lod)",
    "gvec4 texelFetch(gsampler2DRect sampler, ivec2 P)",
    "gvec4 texelFetch(gsampler1DArray sampler, ivec2 P, int lod)",
    "gvec4 texelFetch(gsampler2DArray sampler, ivec3 P, int lod)",
    "gvec4 texelFetch(gsamplerBuffer sampler, int P)",
    "gvec4 texelFetch(gsampler2DMS sampler, ivec2 P, int sample)",
    "gvec4 texelFetch(gsampler2DMSArray sampler, ivec3 P, int sample)",
    "gvec4 texelFetchOffset(gsampler1D sampler, int P, int lod, int offset)",
    "gvec4 texelFetchOffset(gsampler2D sampler, ivec2 P, int lod, ivec2 offset)",
    "gvec4 texelFetchOffset(gsampler3D sampler, ivec3 P, int lod, ivec3 offset)",
    "gvec4 texelFetchOffset(gsampler2DRect sampler, ivec2 P, ivec2 offset)",
    "gvec4 texelFetchOffset(gsampler1DArray sampler, ivec2 P, int lod, int offset)",
    "gvec4 texelFetchOffset(gsampler2DArray sampler, ivec3 P, int lod, ivec2 offset)",
    "gvec4 textureProjOffset(gsampler1D sampler, vec2 P, int offset [, float bias] )",
    "gvec4 textureProjOffset(gsampler1D sampler, vec4 P, int offset [, float bias] )",
    "gvec4 textureProjOffset(gsampler2D sampler, vec3 P, ivec2 offset [, float bias] )",
    "gvec4 textureProjOffset(gsampler2D sampler, vec4 P, ivec2 offset [, float bias] )",
    "gvec4 textureProjOffset(gsampler3D sampler, vec4 P, ivec3 offset [, float bias] )",
    "gvec4 textureProjOffset(gsampler2DRect sampler, vec3 P, ivec2 offset)",
    "gvec4 textureProjOffset(gsampler2DRect sampler, vec4 P, ivec2 offset)",
    "float textureProjOffset(sampler2DRectShadow sampler, vec4 P, ivec2 offset)",
    "float textureProjOffset(sampler1DShadow sampler, vec4 P, int offset [, float bias] )",
    "float textureProjOffset(sampler2DShadow sampler, vec4 P, ivec2 offset [, float bias] )",
    "gvec4 textureLodOffset(gsampler1D sampler, float P, float lod, int offset)",
    "gvec4 textureLodOffset(gsampler2D sampler, vec2 P, float lod, ivec2 offset)",
    "gvec4 textureLodOffset(gsampler3D sampler, vec3 P, float lod, ivec3 offset)",
    "float textureLodOffset(sampler1DShadow sampler, vec3 P, float lod, int offset)",
    "float textureLodOffset(sampler2DShadow sampler, vec3 P, float lod, ivec2 offset)",
    "gvec4 textureLodOffset(gsampler1DArray sampler, vec2 P, float lod, int offset)",
    "gvec4 textureLodOffset(gsampler2DArray sampler, vec3 P, float lod, ivec2 offset)",
    "float textureLodOffset(sampler1DArrayShadow sampler, vec3 P, float lod, int offset)",
    "gvec4 textureProjLod(gsampler1D sampler, vec2 P, float lod)",
    "gvec4 textureProjLod(gsampler1D sampler, vec4 P, float lod)",
    "gvec4 textureProjLod(gsampler2D sampler, vec3 P, float lod)",
    "gvec4 textureProjLod(gsampler2D sampler, vec4 P, float lod)",
    "gvec4 textureProjLod(gsampler3D sampler, vec4 P, float lod)",
    "float textureProjLod(sampler1DShadow sampler, vec4 P, float lod)",
    "float textureProjLod(sampler2DShadow sampler, vec4 P, float lod)",
    "gvec4 textureProjLodOffset(gsampler1D sampler, vec2 P, float lod, int offset)",
    "gvec4 textureProjLodOffset(gsampler1D sampler, vec4 P, float lod, int offset)",
    "gvec4 textureProjLodOffset(gsampler2D sampler, vec3 P, float lod, ivec2 offset)",
    "gvec4 textureProjLodOffset(gsampler2D sampler, vec4 P, float lod, ivec2 offset)",
    "gvec4 textureProjLodOffset(gsampler3D sampler, vec4 P, float lod, ivec3 offset)",
    "float textureProjLodOffset(sampler1DShadow sampler, vec4 P, float lod, int offset)",
    "float textureProjLodOffset(sampler2DShadow sampler, vec4 P, float lod, ivec2 offset)",
    "gvec4 textureGrad(gsampler1D sampler, float _P, float dPdx, float dPdy)",
    "gvec4 textureGrad(gsampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy)",
    "gvec4 textureGrad(gsampler3D sampler, P, vec3 dPdx, vec3 dPdy)",
    "gvec4 textureGrad(gsamplerCube sampler, vec3 P, vec3 dPdx, vec3 dPdy)",
    "gvec4 textureGrad(gsampler2DRect sampler, vec2 P, vec2 dPdx, vec2 dPdy)",
    "float textureGrad(sampler2DRectShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy)",
    "float textureGrad(sampler1DShadow sampler, vec3 P, float dPdx, float dPdy)",
    "gvec4 textureGrad(gsampler1DArray sampler, vec2 P, float dPdx, float dPdy)",
    "gvec4 textureGrad(gsampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy)",
    "float textureGrad(sampler1DArrayShadow sampler, vec3 P, float dPdx, float dPdy)",
    "float textureGrad(sampler2DShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy)",
    "float textureGrad(samplerCubeShadow sampler, vec4 P, vec3 dPdx, vec3 dPdy)",
    "float textureGrad(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy)",
    "gvec4 textureGrad(gsamplerCubeArray sampler, vec4 P, vec3 dPdx, vec3 dPdy)",
    "gvec4 textureGradOffset(gsampler1D sampler, float P, float dPdx, float dPdy, int offset)",
    "gvec4 textureGradOffset(gsampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureGradOffset(gsampler3D sampler, vec3 P, vec3 dPdx, vec3 dPdy, ivec3 offset)",
    "gvec4 textureGradOffset(gsampler2DRect sampler, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "float textureGradOffset(sampler2DRectShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "float textureGradOffset(sampler1DShadow sampler, vec3 P, float dPdx, float dPdy, int offset)",
    "float textureGradOffset(sampler2DShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureGradOffset(gsampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureGradOffset(gsampler1DArray sampler, vec2 P, float dPdx, float dPdy, int offset)",
    "float textureGradOffset(sampler1DArrayShadow sampler, vec3 P, float dPdx, float dPdy, int offset)",
    "float textureGradOffset(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureProjGrad(gsampler1D sampler, vec2 P, float dPdx, float dPdy)",
    "gvec4 textureProjGrad(gsampler1D sampler, vec4 P, float dPdx, float dPdy)",
    "gvec4 textureProjGrad(gsampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy)",
    "gvec4 textureProjGrad(gsampler2D sampler, vec4 P, vec2 dPdx, vec2 dPdy)",
    "gvec4 textureProjGrad(gsampler3D sampler, vec4 P, vec3 dPdx, vec3 dPdy)",
    "gvec4 textureProjGrad(gsampler2DRect sampler, vec3 P, vec2 dPdx, vec2 dPdy)",
    "gvec4 textureProjGrad(gsampler2DRect sampler, vec4 P, vec2 dPdx, vec2 dPdy)",
    "float textureProjGrad(sampler2DRectShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy)",
    "float textureProjGrad(sampler1DShadow sampler, vec4 P, float dPdx, float dPdy)",
    "float textureProjGrad(sampler2DShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy)",
    "gvec4 textureProjGradOffset(gsampler1D sampler, vec2 P, float dPdx, float dPdy, int offset)",
    "gvec4 textureProjGradOffset(gsampler1D sampler, vec4 P, float dPdx, float dPdy, int offset)",
    "gvec4 textureProjGradOffset(gsampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureProjGradOffset(gsampler2D sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureProjGradOffset(gsampler3D sampler, vec4 P, vec3 dPdx, vec3 dPdy, ivec3 offset)",
    "gvec4 textureProjGradOffset(gsampler2DRect sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "gvec4 textureProjGradOffset(gsampler2DRect sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "float textureProjGradOffset(sampler2DRectShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",
    "float textureProjGradOffset(sampler1DShadow sampler, vec4 P, float dPdx, float dPdy, int offset)",
    "float textureProjGradOffset(sampler2DShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset)",

    # Texture Gather Functions
    "gvec4 textureGather(gsampler2D sampler, vec2 P [, int comp])",
    "gvec4 textureGather(gsampler2DArray sampler, vec3 P [, int comp])",
    "gvec4 textureGather(gsamplerCube sampler, vec3 P [, int comp])",
    "gvec4 textureGather(gsamplerCubeArray sampler, vec4 P[, int comp])",
    "gvec4 textureGather(gsampler2DRect sampler, vec2 P[, int comp])",
    "vec4 textureGather(sampler2DShadow sampler, vec2 P, float refZ)",
    "vec4 textureGather(sampler2DArrayShadow sampler, vec3 P, float refZ)",
    "vec4 textureGather(samplerCubeShadow sampler, vec3 P, float refZ)",
    "vec4 textureGather(samplerCubeArrayShadow sampler, vec4 P, float refZ)",
    "vec4 textureGather(sampler2DRectShadow sampler, vec2 P, float refZ)",
    "gvec4 textureGatherOffset(gsampler2D sampler, vec2 P, ivec2 offset [, int comp])",
    "gvec4 textureGatherOffset(gsampler2DArray sampler, vec3 P, ivec2 offset [, int comp])",
    "vec4 textureGatherOffset(sampler2DShadow sampler, vec2 P, float refZ, ivec2 offset)",
    "vec4 textureGatherOffset(sampler2DArrayShadow sampler, vec3 P, float refZ, ivec2 offset)",
    "gvec4 textureGatherOffset(gsampler2DRect sampler, vec2 P, ivec2 offset [, int comp])",
    "vec4 textureGatherOffset(sampler2DRectShadow sampler, vec2 P, float refZ, ivec2 offset)",
    "gvec4 textureGatherOffsets(gsampler2D sampler, vec2 P, ivec2 offsets[4] [, int comp])",
    "gvec4 textureGatherOffsets(gsampler2DArray sampler, vec3 P, ivec2 offsets[4] [, int comp])",
    "vec4 textureGatherOffsets(sampler2DShadow sampler, vec2 P, float refZ, ivec2 offsets[4])",
    "vec4 textureGatherOffsets(sampler2DArrayShadow sampler, vec3 P, float refZ, ivec2 offsets[4])",
    "gvec4 textureGatherOffsets(gsampler2DRect sampler, vec2 P, ivec2 offsets[4] [, int comp])",
    "vec4 textureGatherOffsets(sampler2DRectShadow sampler, vec2 P, float refZ, ivec2 offsets[4])",

    # Compatibility Profile Texture Functions
    # skipped

    # Atomic Counter Functions
    "uint atomicCounterIncrement(atomic_uint c)",
    "uint atomicCounterDecrement(atomic_uint c)",
    "uint atomicCounter(atomic_uint c)",
    "uint atomicCounterAdd(atomic_uint c, uint data)",
    "uint atomicCounterSubtract(atomic_uint c, uint data)",
    "uint atomicCounterMin(atomic_uint c, uint data)",
    "uint atomicCounterMax(atomic_uint c, uint data)",
    "uint atomicCounterAnd(atomic_uint c, uint data)",
    "uint atomicCounterOr(atomic_uint c, uint data)",
    "uint atomicCounterXor(atomic_uint c, uint data)",
    "uint atomicCounterExchange(atomic_uint c, uint data)",
    "uint atomicCounterCompSwap(atomic_uint c, uint compare, uint data)",

    # Atomic Memory Functions
    "uint atomicAdd(inout uint mem, uint data)",
    "int atomicAdd(inout int mem, int data)",
    "uint atomicMin(inout uint mem, uint data)",
    "int atomicMin(inout int mem, int data)",
    "uint atomicMax(inout uint mem, uint data)",
    "int atomicMax(inout int mem, int data)",
    "uint atomicAnd(inout uint mem, uint data)",
    "int atomicAnd(inout int mem, int data)",
    "uint atomicOr(inout uint mem, uint data)",
    "int atomicOr(inout int mem, int data)",
    "uint atomicXor(inout uint mem, uint data)",
    "int atomicXor(inout int mem, int data)",
    "uint atomicExchange(inout uint mem, uint data)",
    "int atomicExchange(inout int mem, int data)",
    "uint atomicCompSwap(inout uint mem, uint compare, uint data)",
    "int atomicCompSwap(inout int mem, int compare, int data)",

    # Image Functions
    "int imageSize(readonly writeonly gimage1D image)",
    "ivec2 imageSize(readonly writeonly gimage2D image)",
    "ivec3 imageSize(readonly writeonly gimage3D image)",
    "ivec2 imageSize(readonly writeonly gimageCube image)",
    "ivec3 imageSize(readonly writeonly gimageCubeArray image)",
    "ivec3 imageSize(readonly writeonly gimage2DArray image)",
    "ivec2 imageSize(readonly writeonly gimageRect image)",
    "ivec2 imageSize(readonly writeonly gimage1DArray image)",
    "ivec2 imageSize(readonly writeonly gimage2DMS image)",
    "ivec3 imageSize(readonly writeonly gimage2DMSArray image)",
    "int imageSize(readonly writeonly gimageBuffer image)",
    "int imageSamples(readonly writeonly gimage2DMS image)",
    "int imageSamples(readonly writeonly gimage2DMSArray image)",
    "gvec4 imageLoad(readonly IMAGE_PARAMS)",
    "void imageStore(writeonly IMAGE_PARAMS, gvec4 data)",
    "uint imageAtomicAdd(IMAGE_PARAMS, uint data)",
    "int imageAtomicAdd(IMAGE_PARAMS, int data)",
    "uint imageAtomicMin(IMAGE_PARAMS, uint data)",
    "int imageAtomicMin(IMAGE_PARAMS, int data)",
    "uint imageAtomicMax(IMAGE_PARAMS, uint data)",
    "int imageAtomicMax(IMAGE_PARAMS, int data)",
    "uint imageAtomicAnd(IMAGE_PARAMS, uint data)",
    "int imageAtomicAnd(IMAGE_PARAMS, int data)",
    "uint imageAtomicOr(IMAGE_PARAMS, uint data)",
    "int imageAtomicOr(IMAGE_PARAMS, int data)",
    "uint imageAtomicXor(IMAGE_PARAMS, uint data)",
    "int imageAtomicXor(IMAGE_PARAMS, int data)",
    "uint imageAtomicExchange(IMAGE_PARAMS, uint data)",
    "int imageAtomicExchange(IMAGE_PARAMS, int data)",
    "float imageAtomicExchange(IMAGE_PARAMS, float data)",
    "uint imageAtomicCompSwap(IMAGE_PARAMS, uint compare, uint data)",
    "int imageAtomicCompSwap(IMAGE_PARAMS, int compare, int data)",

    # Geometry Shader Functions
    "void EmitStreamVertex(int stream)",
    "void EndStreamPrimitive(int stream)",
    "void EmitVertex()",
    "void EndPrimitive()",

    # Derivative Functions
    "genFType dFdx(genFType p)",
    "genFType dFdy(genFType p)",
    "genFType dFdxFine(genFType p)",
    "genFType dFdyFine(genFType p)",
    "genFType dFdxCoarse(genFType p)",
    "genFType dFdyCoarse(genFType p)",
    "genFType fwidth(genFType p)",
    "genFType fwidthFine(genFType p)",
    "genFType fwidthCoarse(genFType p)",

    # Interpolation Functions
    "float interpolateAtCentroid(float interpolant)",
    "vec2 interpolateAtCentroid(vec2 interpolant)",
    "vec3 interpolateAtCentroid(vec3 interpolant)",
    "vec4 interpolateAtCentroid(vec4 interpolant)",
    "float interpolateAtSample(float interpolant, int sample)",
    "vec2 interpolateAtSample(vec2 interpolant, int sample)",
    "vec3 interpolateAtSample(vec3 interpolant, int sample)",
    "vec4 interpolateAtSample(vec4 interpolant, int sample)",
    "float interpolateAtOffset(float interpolant, vec2 offset)",
    "vec2 interpolateAtOffset(vec2 interpolant, vec2 offset)",
    "vec3 interpolateAtOffset(vec3 interpolant, vec2 offset)",
    "vec4 interpolateAtOffset(vec4 interpolant, vec2 offset)",

    # Noise Functions
    # skipped

    # Shader Invocation Control Functions
    "void barrier()",

    # Shader Memory Control Functions
    "void memoryBarrier()",
    "void memoryBarrierAtomicCounter()",
    "void memoryBarrierBuffer()",
    "void memoryBarrierShared()",
    "void memoryBarrierImage()",
    "void groupMemoryBarrier()",

    # Subpass-Input Functions
    "gvec4 subpassLoad(gsubpassInput subpass)",
    "gvec4 subpassLoad(gsubpassInputMS subpass, int sample)",

    # Shader Invocation Group Functions
    "bool anyInvocation(bool value)",
    "bool allInvocations(bool value)",
    "bool allInvocationsEqual(bool value)",
]

TypeTemplate = {
    "genFType": ["float", "vec2", "vec3", "vec4"],
    "genIType": ["int", "ivec2", "ivec3", "ivec4"],
    "genUType": ["uint", "uvec2", "uvec3", "uvec4"],
    "genBType": ["bool", "bvec2", "bvec3", "bvec4"],
    "genDType": ["double", "dvec2", "dvec3", "dvec4"],

    # These have to be handled specially at the end
    "bvec ": ["bvec2 ", "bvec3 ", "bvec4 "],
    "ivec ": ["ivec2 ", "ivec3 ", "ivec4 "],
    "uvec ": ["uvec2 ", "uvec3 ", "uvec4 "],
    "vec ": ["vec2 ", "vec3 ", "vec4 ", "dvec2 ", "dvec3 ", "dvec4 "],
    "mat ": ["mat2 ",   "mat3 ",   "mat4 ",
             "mat2x2 ", "mat2x3 ", "mat2x4 ",
             "mat3x2 ", "mat3x3 ", "mat3x4 ",
             "mat4x2 ", "mat4x3 ", "mat4x4 "],
    "dmat ": ["dmat2 ",   "dmat3 ",   "dmat4 ",
              "dmat2x2 ", "dmat2x3 ", "dmat2x4 ",
              "dmat3x2 ", "dmat3x3 ", "dmat3x4 ",
              "dmat4x2 ", "dmat4x3 ", "dmat4x4 "],

    "IMAGE_PARAMS": [
        "gimage2D image, ivec2 P",
        "gimage3D image, ivec3 P",
        "gimageCube image, ivec3 P",
        "gimageBuffer image, int P",
        "gimage2DArray image, ivec3 P",
        "gimageCubeArray image, ivec3 P",
        "gimage1D image, int P",
        "gimage1DArray image, ivec2 P",
        "gimage2DRect image, ivec2 P",
        "gimage2DMS image, ivec2 P, int sample",
        "gimage2DMSArray image, ivec3 P, int sample",
    ],

    "gvec": ["vec", "bvec", "ivec", "uvec", "dvec"],
    "gsampler": ["sampler", "isampler", "usampler"],
    "gimage": ["image", "iimage", "uimage"],
    "gsubpassInput": ["subpassInput", "isubpassInput", "usubpassInput"],

    "[, float bias]": [
        "",
        ", float bias",
    ],
    "[, int comp]": [
        "",
        ", int comp",
    ],
}


def GenerateBuiltinFunction():
    for signature in LibraryFunctions:
        buffer = [signature]
        nextBuffer: list[str] = []
        for key, expansionList in TypeTemplate.items():
            for sig in buffer:
                if sig.count(key) > 0:
                    nextBuffer += [sig.replace(key, expansion)
                                   for expansion in expansionList]
                else:
                    nextBuffer += [sig]

            buffer = nextBuffer
            nextBuffer = []

        for sig in buffer:
            print(f'"{sig};"')


def GenerateBuiltinVariable():
    vertexBuiltinVars = """
in int gl_VertexID; // only present when not targeting Vulkan
in int gl_InstanceID; // only present when not targeting Vulkan
in int gl_VertexIndex; // only present when targeting Vulkan
in int gl_InstanceIndex; // only present when targeting Vulkan
in int gl_DrawID;
in int gl_BaseVertex;
in int gl_BaseInstance;
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
};
    """
    tessControlBuiltinVars = """
in gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
} gl_in[gl_MaxPatchVertices];
in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
in int gl_InvocationID;
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
} gl_out[];
patch out float gl_TessLevelOuter[4];
patch out float gl_TessLevelInner[2];
    """
    tessEvaluationBuiltinVars = """
in gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
} gl_in[gl_MaxPatchVertices];
in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
in vec3 gl_TessCoord;
patch in float gl_TessLevelOuter[4];
patch in float gl_TessLevelInner[2];
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
};
    """
    geometryBuiltinVars = """
in gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
} gl_in[];
in int gl_PrimitiveIDIn;
in int gl_InvocationID;
out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
  float gl_CullDistance[];
};
out int gl_PrimitiveID;
out int gl_Layer;
out int gl_ViewportIndex;
    """
    fragmentBuiltinVars = """
in vec4 gl_FragCoord;
in bool gl_FrontFacing;
in float gl_ClipDistance[];
in float gl_CullDistance[];
in vec2 gl_PointCoord;
in int gl_PrimitiveID;
in int gl_SampleID;
in vec2 gl_SamplePosition;
in int gl_SampleMaskIn[];
in int gl_Layer;
in int gl_ViewportIndex;
in bool gl_HelperInvocation;
out float gl_FragDepth;
out int gl_SampleMask[];
    """
    computeBuiltinVars = """
// workgroup dimensions
in uvec3 gl_NumWorkGroups;
const uvec3 gl_WorkGroupSize;
// workgroup and invocation IDs
in uvec3 gl_WorkGroupID;
in uvec3 gl_LocalInvocationID;
// derived variables
in uvec3 gl_GlobalInvocationID;
in uint gl_LocalInvocationIndex;
    """
    # compatibility builtin passed
    builtinConstant = """
const int gl_MaxVertexAttribs = 16;
const int gl_MaxVertexUniformVectors = 256;
const int gl_MaxVertexUniformComponents = 1024;
const int gl_MaxVertexOutputComponents = 64;
const int gl_MaxVaryingComponents = 60;
const int gl_MaxVaryingVectors = 15;
const int gl_MaxVertexTextureImageUnits = 16;
const int gl_MaxVertexImageUniforms = 0;
const int gl_MaxVertexAtomicCounters = 0;
const int gl_MaxVertexAtomicCounterBuffers = 0;
const int gl_MaxTessPatchComponents = 120;
const int gl_MaxPatchVertices = 32;
const int gl_MaxTessGenLevel = 64;
const int gl_MaxTessControlInputComponents = 128;
const int gl_MaxTessControlOutputComponents = 128;
const int gl_MaxTessControlTextureImageUnits = 16;
const int gl_MaxTessControlUniformComponents = 1024;
const int gl_MaxTessControlTotalOutputComponents = 4096;
const int gl_MaxTessControlImageUniforms = 0;
const int gl_MaxTessControlAtomicCounters = 0;
const int gl_MaxTessControlAtomicCounterBuffers = 0;
const int gl_MaxTessEvaluationInputComponents = 128;
const int gl_MaxTessEvaluationOutputComponents = 128;
const int gl_MaxTessEvaluationTextureImageUnits = 16;
const int gl_MaxTessEvaluationUniformComponents = 1024;
const int gl_MaxTessEvaluationImageUniforms = 0;
const int gl_MaxTessEvaluationAtomicCounters = 0;
const int gl_MaxTessEvaluationAtomicCounterBuffers = 0;
const int gl_MaxGeometryInputComponents = 64;
const int gl_MaxGeometryOutputComponents = 128;
const int gl_MaxGeometryImageUniforms = 0;
const int gl_MaxGeometryTextureImageUnits = 16;
const int gl_MaxGeometryOutputVertices = 256;
const int gl_MaxGeometryTotalOutputComponents = 1024;
const int gl_MaxGeometryUniformComponents = 1024;
const int gl_MaxGeometryVaryingComponents = 64; // deprecated
const int gl_MaxGeometryAtomicCounters = 0;
const int gl_MaxGeometryAtomicCounterBuffers = 0;
const int gl_MaxFragmentImageUniforms = 8;
const int gl_MaxFragmentInputComponents = 128;
const int gl_MaxFragmentUniformVectors = 256;
const int gl_MaxFragmentUniformComponents = 1024;
const int gl_MaxFragmentAtomicCounters = 8;
const int gl_MaxFragmentAtomicCounterBuffers = 1;
const int gl_MaxDrawBuffers = 8;
const int gl_MaxTextureImageUnits = 16;
const int gl_MinProgramTexelOffset = -8;
const int gl_MaxProgramTexelOffset = 7;
const int gl_MaxImageUnits = 8;
const int gl_MaxSamples = 4;
const int gl_MaxImageSamples = 0;
const int gl_MaxClipDistances = 8;
const int gl_MaxCullDistances = 8;
const int gl_MaxViewports = 16;
const int gl_MaxComputeImageUniforms = 8;
const ivec3 gl_MaxComputeWorkGroupCount = { 65535, 65535, 65535 };
const ivec3 gl_MaxComputeWorkGroupSize = { 1024, 1024, 64 };
const int gl_MaxComputeUniformComponents = 1024;
const int gl_MaxComputeTextureImageUnits = 16;
const int gl_MaxComputeAtomicCounters = 8;
const int gl_MaxComputeAtomicCounterBuffers = 8;
const int gl_MaxCombinedTextureImageUnits = 96;
const int gl_MaxCombinedImageUniforms = 48;
const int gl_MaxCombinedImageUnitsAndFragmentOutputs = 8; // deprecated
const int gl_MaxCombinedShaderOutputResources = 16;
const int gl_MaxCombinedAtomicCounters = 8;
const int gl_MaxCombinedAtomicCounterBuffers = 1;
const int gl_MaxCombinedClipAndCullDistances = 8;
const int gl_MaxAtomicCounterBindings = 1;
const int gl_MaxAtomicCounterBufferSize = 32;
const int gl_MaxTransformFeedbackBuffers = 4;
const int gl_MaxTransformFeedbackInterleavedComponents = 64;
const highp int gl_MaxInputAttachments = 1; // only present when targeting Vulkan
    """


GenerateBuiltinFunction()
