bvec2 lessThan(vec2 x,vec2 y);
bvec3 lessThan(vec3 x,vec3 y);
bvec4 lessThan(vec4 x,vec4 y);
bvec2 lessThan(ivec2 x,ivec2 y);
bvec3 lessThan(ivec3 x,ivec3 y);
bvec4 lessThan(ivec4 x,ivec4 y);
bvec2 lessThan(uvec2 x,uvec2 y);
bvec3 lessThan(uvec3 x,uvec3 y);
bvec4 lessThan(uvec4 x,uvec4 y);
bvec2 lessThan(dvec2 x, dvec2 y);
bvec3 lessThan(dvec3 x, dvec3 y);
bvec4 lessThan(dvec4 x, dvec4 y);
bvec2 lessThanEqual(vec2 x,vec2 y);
bvec3 lessThanEqual(vec3 x,vec3 y);
bvec4 lessThanEqual(vec4 x,vec4 y);
bvec2 lessThanEqual(ivec2 x,ivec2 y);
bvec3 lessThanEqual(ivec3 x,ivec3 y);
bvec4 lessThanEqual(ivec4 x,ivec4 y);
bvec2 lessThanEqual(uvec2 x,uvec2 y);
bvec3 lessThanEqual(uvec3 x,uvec3 y);
bvec4 lessThanEqual(uvec4 x,uvec4 y);
bvec2 lessThanEqual(dvec2 x, dvec2 y);
bvec3 lessThanEqual(dvec3 x, dvec3 y);
bvec4 lessThanEqual(dvec4 x, dvec4 y);
bvec2 greaterThan(vec2 x,vec2 y);
bvec3 greaterThan(vec3 x,vec3 y);
bvec4 greaterThan(vec4 x,vec4 y);
bvec2 greaterThan(ivec2 x,ivec2 y);
bvec3 greaterThan(ivec3 x,ivec3 y);
bvec4 greaterThan(ivec4 x,ivec4 y);
bvec2 greaterThan(uvec2 x,uvec2 y);
bvec3 greaterThan(uvec3 x,uvec3 y);
bvec4 greaterThan(uvec4 x,uvec4 y);
bvec2 greaterThan(dvec2 x, dvec2 y);
bvec3 greaterThan(dvec3 x, dvec3 y);
bvec4 greaterThan(dvec4 x, dvec4 y);
bvec2 greaterThanEqual(vec2 x,vec2 y);
bvec3 greaterThanEqual(vec3 x,vec3 y);
bvec4 greaterThanEqual(vec4 x,vec4 y);
bvec2 greaterThanEqual(ivec2 x,ivec2 y);
bvec3 greaterThanEqual(ivec3 x,ivec3 y);
bvec4 greaterThanEqual(ivec4 x,ivec4 y);
bvec2 greaterThanEqual(uvec2 x,uvec2 y);
bvec3 greaterThanEqual(uvec3 x,uvec3 y);
bvec4 greaterThanEqual(uvec4 x,uvec4 y);
bvec2 greaterThanEqual(dvec2 x, dvec2 y);
bvec3 greaterThanEqual(dvec3 x, dvec3 y);
bvec4 greaterThanEqual(dvec4 x, dvec4 y);
bvec2 equal(bvec2 x,bvec2 y);
bvec3 equal(bvec3 x,bvec3 y);
bvec4 equal(bvec4 x,bvec4 y);
bvec2 equal(vec2 x,vec2 y);
bvec3 equal(vec3 x,vec3 y);
bvec4 equal(vec4 x,vec4 y);
bvec2 equal(ivec2 x,ivec2 y);
bvec3 equal(ivec3 x,ivec3 y);
bvec4 equal(ivec4 x,ivec4 y);
bvec2 equal(uvec2 x,uvec2 y);
bvec3 equal(uvec3 x,uvec3 y);
bvec4 equal(uvec4 x,uvec4 y);
bvec2 equal(dvec2 x, dvec2 y);
bvec3 equal(dvec3 x, dvec3 y);
bvec4 equal(dvec4 x, dvec4 y);
bvec2 notEqual(bvec2 x,bvec2 y);
bvec3 notEqual(bvec3 x,bvec3 y);
bvec4 notEqual(bvec4 x,bvec4 y);
bvec2 notEqual(vec2 x,vec2 y);
bvec3 notEqual(vec3 x,vec3 y);
bvec4 notEqual(vec4 x,vec4 y);
bvec2 notEqual(ivec2 x,ivec2 y);
bvec3 notEqual(ivec3 x,ivec3 y);
bvec4 notEqual(ivec4 x,ivec4 y);
bvec2 notEqual(uvec2 x,uvec2 y);
bvec3 notEqual(uvec3 x,uvec3 y);
bvec4 notEqual(uvec4 x,uvec4 y);
bvec2 notEqual(dvec2 x, dvec2 y);
bvec3 notEqual(dvec3 x, dvec3 y);
bvec4 notEqual(dvec4 x, dvec4 y);
bool any(bvec2 x);
bool any(bvec3 x);
bool any(bvec4 x);
bool all(bvec2 x);
bool all(bvec3 x);
bool all(bvec4 x);
bvec2 not(bvec2 x);
bvec3 not(bvec3 x);
bvec4 not(bvec4 x);

#if __GLSLD_FEATURE_ENABLE_INT64_TYPE

bvec2 lessThan(i64vec2 x, i64vec2 y);
bvec3 lessThan(i64vec3 x, i64vec3 y);
bvec4 lessThan(i64vec4 x, i64vec4 y);
bvec2 lessThanEqual(i64vec2 x, i64vec2 y);
bvec3 lessThanEqual(i64vec3 x, i64vec3 y);
bvec4 lessThanEqual(i64vec4 x, i64vec4 y);
bvec2 greaterThan(i64vec2 x, i64vec2 y);
bvec3 greaterThan(i64vec3 x, i64vec3 y);
bvec4 greaterThan(i64vec4 x, i64vec4 y);
bvec2 greaterThanEqual(i64vec2 x, i64vec2 y);
bvec3 greaterThanEqual(i64vec3 x, i64vec3 y);
bvec4 greaterThanEqual(i64vec4 x, i64vec4 y);
bvec2 equal(i64vec2 x, i64vec2 y);
bvec3 equal(i64vec3 x, i64vec3 y);
bvec4 equal(i64vec4 x, i64vec4 y);
bvec2 notEqual(i64vec2 x, i64vec2 y);
bvec3 notEqual(i64vec3 x, i64vec3 y);
bvec4 notEqual(i64vec4 x, i64vec4 y);

bvec2 lessThan(u64vec2 x, u64vec2 y);
bvec3 lessThan(u64vec3 x, u64vec3 y);
bvec4 lessThan(u64vec4 x, u64vec4 y);
bvec2 lessThanEqual(u64vec2 x, u64vec2 y);
bvec3 lessThanEqual(u64vec3 x, u64vec3 y);
bvec4 lessThanEqual(u64vec4 x, u64vec4 y);
bvec2 greaterThan(u64vec2 x, u64vec2 y);
bvec3 greaterThan(u64vec3 x, u64vec3 y);
bvec4 greaterThan(u64vec4 x, u64vec4 y);
bvec2 greaterThanEqual(u64vec2 x, u64vec2 y);
bvec3 greaterThanEqual(u64vec3 x, u64vec3 y);
bvec4 greaterThanEqual(u64vec4 x, u64vec4 y);
bvec2 equal(u64vec2 x, u64vec2 y);
bvec3 equal(u64vec3 x, u64vec3 y);
bvec4 equal(u64vec4 x, u64vec4 y);
bvec2 notEqual(u64vec2 x, u64vec2 y);
bvec3 notEqual(u64vec3 x, u64vec3 y);
bvec4 notEqual(u64vec4 x, u64vec4 y);

#endif

#if __GLSLD_FEATURE_ENABLE_INT16_TYPE

bvec2 lessThan(i16vec2 x, i16vec2 y);
bvec3 lessThan(i16vec3 x, i16vec3 y);
bvec4 lessThan(i16vec4 x, i16vec4 y);
bvec2 lessThanEqual(i16vec2 x, i16vec2 y);
bvec3 lessThanEqual(i16vec3 x, i16vec3 y);
bvec4 lessThanEqual(i16vec4 x, i16vec4 y);
bvec2 greaterThan(i16vec2 x, i16vec2 y);
bvec3 greaterThan(i16vec3 x, i16vec3 y);
bvec4 greaterThan(i16vec4 x, i16vec4 y);
bvec2 greaterThanEqual(i16vec2 x, i16vec2 y);
bvec3 greaterThanEqual(i16vec3 x, i16vec3 y);
bvec4 greaterThanEqual(i16vec4 x, i16vec4 y);
bvec2 equal(i16vec2 x, i16vec2 y);
bvec3 equal(i16vec3 x, i16vec3 y);
bvec4 equal(i16vec4 x, i16vec4 y);
bvec2 notEqual(i16vec2 x, i16vec2 y);
bvec3 notEqual(i16vec3 x, i16vec3 y);
bvec4 notEqual(i16vec4 x, i16vec4 y);

bvec2 lessThan(u16vec2 x, u16vec2 y);
bvec3 lessThan(u16vec3 x, u16vec3 y);
bvec4 lessThan(u16vec4 x, u16vec4 y);
bvec2 lessThanEqual(u16vec2 x, u16vec2 y);
bvec3 lessThanEqual(u16vec3 x, u16vec3 y);
bvec4 lessThanEqual(u16vec4 x, u16vec4 y);
bvec2 greaterThan(u16vec2 x, u16vec2 y);
bvec3 greaterThan(u16vec3 x, u16vec3 y);
bvec4 greaterThan(u16vec4 x, u16vec4 y);
bvec2 greaterThanEqual(u16vec2 x, u16vec2 y);
bvec3 greaterThanEqual(u16vec3 x, u16vec3 y);
bvec4 greaterThanEqual(u16vec4 x, u16vec4 y);
bvec2 equal(u16vec2 x, u16vec2 y);
bvec3 equal(u16vec3 x, u16vec3 y);
bvec4 equal(u16vec4 x, u16vec4 y);
bvec2 notEqual(u16vec2 x, u16vec2 y);
bvec3 notEqual(u16vec3 x, u16vec3 y);
bvec4 notEqual(u16vec4 x, u16vec4 y);

#endif

#if __GLSLD_FEATURE_ENABLE_INT8_TYPE

bvec2 lessThan(i8vec2 x, i8vec2 y);
bvec3 lessThan(i8vec3 x, i8vec3 y);
bvec4 lessThan(i8vec4 x, i8vec4 y);
bvec2 lessThanEqual(i8vec2 x, i8vec2 y);
bvec3 lessThanEqual(i8vec3 x, i8vec3 y);
bvec4 lessThanEqual(i8vec4 x, i8vec4 y);
bvec2 greaterThan(i8vec2 x, i8vec2 y);
bvec3 greaterThan(i8vec3 x, i8vec3 y);
bvec4 greaterThan(i8vec4 x, i8vec4 y);
bvec2 greaterThanEqual(i8vec2 x, i8vec2 y);
bvec3 greaterThanEqual(i8vec3 x, i8vec3 y);
bvec4 greaterThanEqual(i8vec4 x, i8vec4 y);
bvec2 equal(i8vec2 x, i8vec2 y);
bvec3 equal(i8vec3 x, i8vec3 y);
bvec4 equal(i8vec4 x, i8vec4 y);
bvec2 notEqual(i8vec2 x, i8vec2 y);
bvec3 notEqual(i8vec3 x, i8vec3 y);
bvec4 notEqual(i8vec4 x, i8vec4 y);

bvec2 lessThan(u8vec2 x, u8vec2 y);
bvec3 lessThan(u8vec3 x, u8vec3 y);
bvec4 lessThan(u8vec4 x, u8vec4 y);
bvec2 lessThanEqual(u8vec2 x, u8vec2 y);
bvec3 lessThanEqual(u8vec3 x, u8vec3 y);
bvec4 lessThanEqual(u8vec4 x, u8vec4 y);
bvec2 greaterThan(u8vec2 x, u8vec2 y);
bvec3 greaterThan(u8vec3 x, u8vec3 y);
bvec4 greaterThan(u8vec4 x, u8vec4 y);
bvec2 greaterThanEqual(u8vec2 x, u8vec2 y);
bvec3 greaterThanEqual(u8vec3 x, u8vec3 y);
bvec4 greaterThanEqual(u8vec4 x, u8vec4 y);
bvec2 equal(u8vec2 x, u8vec2 y);
bvec3 equal(u8vec3 x, u8vec3 y);
bvec4 equal(u8vec4 x, u8vec4 y);
bvec2 notEqual(u8vec2 x, u8vec2 y);
bvec3 notEqual(u8vec3 x, u8vec3 y);
bvec4 notEqual(u8vec4 x, u8vec4 y);

#endif

#if __GLSLD_FEATURE_ENABLE_FLOAT16_TYPE

bvec2 lessThan(f16vec2 x, f16vec2 y);
bvec3 lessThan(f16vec3 x, f16vec3 y);
bvec4 lessThan(f16vec4 x, f16vec4 y);
bvec2 lessThanEqual(f16vec2 x, f16vec2 y);
bvec3 lessThanEqual(f16vec3 x, f16vec3 y);
bvec4 lessThanEqual(f16vec4 x, f16vec4 y);
bvec2 greaterThan(f16vec2 x, f16vec2 y);
bvec3 greaterThan(f16vec3 x, f16vec3 y);
bvec4 greaterThan(f16vec4 x, f16vec4 y);
bvec2 greaterThanEqual(f16vec2 x, f16vec2 y);
bvec3 greaterThanEqual(f16vec3 x, f16vec3 y);
bvec4 greaterThanEqual(f16vec4 x, f16vec4 y);
bvec2 equal(f16vec2 x, f16vec2 y);
bvec3 equal(f16vec3 x, f16vec3 y);
bvec4 equal(f16vec4 x, f16vec4 y);
bvec2 notEqual(f16vec2 x, f16vec2 y);
bvec3 notEqual(f16vec3 x, f16vec3 y);
bvec4 notEqual(f16vec4 x, f16vec4 y);

#endif