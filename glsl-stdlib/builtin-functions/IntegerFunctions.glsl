uint uaddCarry(highp  uint x, highp  uint y, out lowp  uint carry);
uvec2 uaddCarry(highp uvec2 x, highp uvec2 y, out lowp uvec2 carry);
uvec3 uaddCarry(highp uvec3 x, highp uvec3 y, out lowp uvec3 carry);
uvec4 uaddCarry(highp uvec4 x, highp uvec4 y, out lowp uvec4 carry);
uint usubBorrow(highp  uint x, highp  uint y, out lowp  uint borrow);
uvec2 usubBorrow(highp uvec2 x, highp uvec2 y, out lowp uvec2 borrow);
uvec3 usubBorrow(highp uvec3 x, highp uvec3 y, out lowp uvec3 borrow);
uvec4 usubBorrow(highp uvec4 x, highp uvec4 y, out lowp uvec4 borrow);
void umulExtended(highp  uint x, highp  uint y, out highp  uint msb, out highp  uint lsb);
void umulExtended(highp uvec2 x, highp uvec2 y, out highp uvec2 msb, out highp uvec2 lsb);
void umulExtended(highp uvec3 x, highp uvec3 y, out highp uvec3 msb, out highp uvec3 lsb);
void umulExtended(highp uvec4 x, highp uvec4 y, out highp uvec4 msb, out highp uvec4 lsb);
void imulExtended(highp   int x, highp   int y, out highp   int msb, out highp   int lsb);
void imulExtended(highp ivec2 x, highp ivec2 y, out highp ivec2 msb, out highp ivec2 lsb);
void imulExtended(highp ivec3 x, highp ivec3 y, out highp ivec3 msb, out highp ivec3 lsb);
void imulExtended(highp ivec4 x, highp ivec4 y, out highp ivec4 msb, out highp ivec4 lsb);
int bitfieldExtract(  int value, int offset, int bits);
ivec2 bitfieldExtract(ivec2 value, int offset, int bits);
ivec3 bitfieldExtract(ivec3 value, int offset, int bits);
ivec4 bitfieldExtract(ivec4 value, int offset, int bits);
uint bitfieldExtract( uint value, int offset, int bits);
uvec2 bitfieldExtract(uvec2 value, int offset, int bits);
uvec3 bitfieldExtract(uvec3 value, int offset, int bits);
uvec4 bitfieldExtract(uvec4 value, int offset, int bits);
int bitfieldInsert(  int base,   int insert, int offset, int bits);
ivec2 bitfieldInsert(ivec2 base, ivec2 insert, int offset, int bits);
ivec3 bitfieldInsert(ivec3 base, ivec3 insert, int offset, int bits);
ivec4 bitfieldInsert(ivec4 base, ivec4 insert, int offset, int bits);
uint bitfieldInsert( uint base,  uint insert, int offset, int bits);
uvec2 bitfieldInsert(uvec2 base, uvec2 insert, int offset, int bits);
uvec3 bitfieldInsert(uvec3 base, uvec3 insert, int offset, int bits);
uvec4 bitfieldInsert(uvec4 base, uvec4 insert, int offset, int bits);
int bitfieldReverse(highp   int value);
ivec2 bitfieldReverse(highp ivec2 value);
ivec3 bitfieldReverse(highp ivec3 value);
ivec4 bitfieldReverse(highp ivec4 value);
uint bitfieldReverse(highp  uint value);
uvec2 bitfieldReverse(highp uvec2 value);
uvec3 bitfieldReverse(highp uvec3 value);
uvec4 bitfieldReverse(highp uvec4 value);
int bitCount(  int value);
ivec2 bitCount(ivec2 value);
ivec3 bitCount(ivec3 value);
ivec4 bitCount(ivec4 value);
int bitCount( uint value);
ivec2 bitCount(uvec2 value);
ivec3 bitCount(uvec3 value);
ivec4 bitCount(uvec4 value);
int findLSB(  int value);
ivec2 findLSB(ivec2 value);
ivec3 findLSB(ivec3 value);
ivec4 findLSB(ivec4 value);
int findLSB( uint value);
ivec2 findLSB(uvec2 value);
ivec3 findLSB(uvec3 value);
ivec4 findLSB(uvec4 value);
int findMSB(highp   int value);
ivec2 findMSB(highp ivec2 value);
ivec3 findMSB(highp ivec3 value);
ivec4 findMSB(highp ivec4 value);
int findMSB(highp  uint value);
ivec2 findMSB(highp uvec2 value);
ivec3 findMSB(highp uvec3 value);
ivec4 findMSB(highp uvec4 value);


// FXIME: support these
// int64_t bitCount(int64_t);
// i64vec2 bitCount(i64vec2);
// i64vec3 bitCount(i64vec3);
// i64vec4 bitCount(i64vec4);
// int64_t bitCount(uint64_t);
// i64vec2 bitCount(u64vec2);
// i64vec3 bitCount(u64vec3);
// i64vec4 bitCount(u64vec4);
// int64_t findLSB(int64_t);
// i64vec2 findLSB(i64vec2);
// i64vec3 findLSB(i64vec3);
// i64vec4 findLSB(i64vec4);
// int64_t findLSB(uint64_t);
// i64vec2 findLSB(u64vec2);
// i64vec3 findLSB(u64vec3);
// i64vec4 findLSB(u64vec4);
// int64_t findMSB(int64_t);
// i64vec2 findMSB(i64vec2);
// i64vec3 findMSB(i64vec3);
// i64vec4 findMSB(i64vec4);
// int64_t findMSB(uint64_t);
// i64vec2 findMSB(u64vec2);
// i64vec3 findMSB(u64vec3);
// i64vec4 findMSB(u64vec4);
// int8_t bitfieldExtract(  int8_t, int8_t, int8_t);
// i8vec2 bitfieldExtract(i8vec2, int8_t, int8_t);
// i8vec3 bitfieldExtract(i8vec3, int8_t, int8_t);
// i8vec4 bitfieldExtract(i8vec4, int8_t, int8_t);
// uint8_t bitfieldExtract( uint8_t, int8_t, int8_t);
// u8vec2 bitfieldExtract(u8vec2, int8_t, int8_t);
// u8vec3 bitfieldExtract(u8vec3, int8_t, int8_t);
// u8vec4 bitfieldExtract(u8vec4, int8_t, int8_t);
// int8_t bitfieldInsert(  int8_t base,   int8_t, int8_t, int8_t);
// i8vec2 bitfieldInsert(i8vec2 base, i8vec2, int8_t, int8_t);
// i8vec3 bitfieldInsert(i8vec3 base, i8vec3, int8_t, int8_t);
// i8vec4 bitfieldInsert(i8vec4 base, i8vec4, int8_t, int8_t);
// uint8_t bitfieldInsert( uint8_t base,  uint8_t, int8_t, int8_t);
// u8vec2 bitfieldInsert(u8vec2 base, u8vec2, int8_t, int8_t);
// u8vec3 bitfieldInsert(u8vec3 base, u8vec3, int8_t, int8_t);
// u8vec4 bitfieldInsert(u8vec4 base, u8vec4, int8_t, int8_t);
// int8_t bitCount(  int8_t);
// i8vec2 bitCount(i8vec2);
// i8vec3 bitCount(i8vec3);
// i8vec4 bitCount(i8vec4);
// int8_t bitCount( uint8_t);
// i8vec2 bitCount(u8vec2);
// i8vec3 bitCount(u8vec3);
// i8vec4 bitCount(u8vec4);
// int8_t findLSB(  int8_t);
// i8vec2 findLSB(i8vec2);
// i8vec3 findLSB(i8vec3);
// i8vec4 findLSB(i8vec4);
// int8_t findLSB( uint8_t);
// i8vec2 findLSB(u8vec2);
// i8vec3 findLSB(u8vec3);
// i8vec4 findLSB(u8vec4);
// int8_t findMSB(  int8_t);
// i8vec2 findMSB(i8vec2);
// i8vec3 findMSB(i8vec3);
// i8vec4 findMSB(i8vec4);
// int8_t findMSB( uint8_t);
// i8vec2 findMSB(u8vec2);
// i8vec3 findMSB(u8vec3);
// i8vec4 findMSB(u8vec4);
// int16_t bitfieldExtract(  int16_t, int16_t, int16_t);
// i16vec2 bitfieldExtract(i16vec2, int16_t, int16_t);
// i16vec3 bitfieldExtract(i16vec3, int16_t, int16_t);
// i16vec4 bitfieldExtract(i16vec4, int16_t, int16_t);
// uint16_t bitfieldExtract( uint16_t, int16_t, int16_t);
// u16vec2 bitfieldExtract(u16vec2, int16_t, int16_t);
// u16vec3 bitfieldExtract(u16vec3, int16_t, int16_t);
// u16vec4 bitfieldExtract(u16vec4, int16_t, int16_t);
// int16_t bitfieldInsert(  int16_t base,   int16_t, int16_t, int16_t);
// i16vec2 bitfieldInsert(i16vec2 base, i16vec2, int16_t, int16_t);
// i16vec3 bitfieldInsert(i16vec3 base, i16vec3, int16_t, int16_t);
// i16vec4 bitfieldInsert(i16vec4 base, i16vec4, int16_t, int16_t);
// uint16_t bitfieldInsert( uint16_t base,  uint16_t, int16_t, int16_t);
// u16vec2 bitfieldInsert(u16vec2 base, u16vec2, int16_t, int16_t);
// u16vec3 bitfieldInsert(u16vec3 base, u16vec3, int16_t, int16_t);
// u16vec4 bitfieldInsert(u16vec4 base, u16vec4, int16_t, int16_t);
// int16_t bitCount(  int16_t);
// i16vec2 bitCount(i16vec2);
// i16vec3 bitCount(i16vec3);
// i16vec4 bitCount(i16vec4);
// int16_t bitCount( uint16_t);
// i16vec2 bitCount(u16vec2);
// i16vec3 bitCount(u16vec3);
// i16vec4 bitCount(u16vec4);
// int16_t findLSB(  int16_t);
// i16vec2 findLSB(i16vec2);
// i16vec3 findLSB(i16vec3);
// i16vec4 findLSB(i16vec4);
// int16_t findLSB( uint16_t);
// i16vec2 findLSB(u16vec2);
// i16vec3 findLSB(u16vec3);
// i16vec4 findLSB(u16vec4);
// int16_t findMSB(  int16_t);
// i16vec2 findMSB(i16vec2);
// i16vec3 findMSB(i16vec3);
// i16vec4 findMSB(i16vec4);
// int16_t findMSB( uint16_t);
// i16vec2 findMSB(u16vec2);
// i16vec3 findMSB(u16vec3);
// i16vec4 findMSB(u16vec4);



