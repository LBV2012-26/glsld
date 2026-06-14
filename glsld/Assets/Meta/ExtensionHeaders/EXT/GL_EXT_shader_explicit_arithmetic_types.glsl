#define GL_EXT_shader_explicit_arithmetic_types 1

// GL_EXT_shader_explicit_arithmetic_types
// (also covers: _int8, _int16, _int32, _int64, _float16, _float32, _float64)

// Types (keyword additions):
// float64_t, f64vec2, f64vec3, f64vec4
// f64mat2, f64mat3, f64mat4
// f64mat2x2, f64mat2x3, f64mat2x4
// f64mat3x2, f64mat3x3, f64mat3x4
// f64mat4x2, f64mat4x3, f64mat4x4
// float32_t, f32vec2, f32vec3, f32vec4
// f32mat2, f32mat3, f32mat4
// f32mat2x2, f32mat2x3, f32mat2x4
// f32mat3x2, f32mat3x3, f32mat3x4
// f32mat4x2, f32mat4x3, f32mat4x4
// float16_t, f16vec2, f16vec3, f16vec4
// f16mat2, f16mat3, f16mat4
// f16mat2x2, f16mat2x3, f16mat2x4
// f16mat3x2, f16mat3x3, f16mat3x4
// f16mat4x2, f16mat4x3, f16mat4x4
// int64_t, i64vec2, i64vec3, i64vec4
// uint64_t, u64vec2, u64vec3, u64vec4
// int32_t, i32vec2, i32vec3, i32vec4
// uint32_t, u32vec2, u32vec3, u32vec4
// int16_t, i16vec2, i16vec3, i16vec4
// uint16_t, u16vec2, u16vec3, u16vec4
// int8_t, i8vec2, i8vec3, i8vec4
// uint8_t, u8vec2, u8vec3, u8vec4

// 8.x Integer Pack and Unpack Functions
int64_t pack64(i32vec2 _V);
uint64_t pack64(u32vec2 _V);
int64_t pack64(i16vec4 _V);
uint64_t pack64(u16vec4 _V);
int32_t pack32(i16vec2 _V);
uint32_t pack32(u16vec2 _V);
int32_t pack32(i8vec4 _V);
uint32_t pack32(u8vec4 _V);
int16_t pack16(i8vec2 _V);
uint16_t pack16(u8vec2 _V);

i32vec2 unpack32(int64_t _V);
u32vec2 unpack32(uint64_t _V);
i16vec4 unpack16(int64_t _V);
u16vec4 unpack16(uint64_t _V);
i16vec2 unpack16(int32_t _V);
u16vec2 unpack16(uint32_t _V);
i8vec4 unpack8(int32_t _V);
u8vec4 unpack8(uint32_t _V);
i8vec2 unpack8(int16_t _V);
u8vec2 unpack8(uint16_t _V);

// Alias pack/unpack functions
int64_t packInt2x32(ivec2 _V);
uint64_t packUint2x32(uvec2 _V);
ivec2 unpackInt2x32(int64_t _V);
uvec2 unpackUint2x32(uint64_t _V);

// 8.4 Floating-Point Pack and Unpack additions
uint32_t packFloat2x16(f16vec2 _V);
f16vec2 unpackFloat2x16(uint32_t _V);

int32_t packInt2x16(i16vec2 _V);
int64_t packInt4x16(i16vec4 _V);
uint32_t packUint2x16(u16vec2 _V);
uint64_t packUint4x16(u16vec4 _V);

i16vec2 unpackInt2x16(int32_t _V);
i16vec4 unpackInt4x16(int64_t _V);
u16vec2 unpackUint2x16(uint32_t _V);
u16vec4 unpackUint4x16(uint64_t _V);

// 8.3 Common Functions (bitcast additions)
int16_t halfBitsToInt16(float16_t _Value);
i16vec2 halfBitsToInt16(f16vec2 _Value);
i16vec3 halfBitsToInt16(f16vec3 _Value);
i16vec4 halfBitsToInt16(f16vec4 _Value);

uint16_t halfBitsToUint16(float16_t _Value);
u16vec2 halfBitsToUint16(f16vec2 _Value);
u16vec3 halfBitsToUint16(f16vec3 _Value);
u16vec4 halfBitsToUint16(f16vec4 _Value);

int32_t floatBitsToInt(float32_t _Value);
i32vec2 floatBitsToInt(f32vec2 _Value);
i32vec3 floatBitsToInt(f32vec3 _Value);
i32vec4 floatBitsToInt(f32vec4 _Value);

uint32_t floatBitsToUint(float32_t _Value);
u32vec2 floatBitsToUint(f32vec2 _Value);
u32vec3 floatBitsToUint(f32vec3 _Value);
u32vec4 floatBitsToUint(f32vec4 _Value);

int64_t doubleBitsToInt64(double _Value);
i64vec2 doubleBitsToInt64(dvec2 _Value);
i64vec3 doubleBitsToInt64(dvec3 _Value);
i64vec4 doubleBitsToInt64(dvec4 _Value);

uint64_t doubleBitsToUint64(double _Value);
u64vec2 doubleBitsToUint64(dvec2 _Value);
u64vec3 doubleBitsToUint64(dvec3 _Value);
u64vec4 doubleBitsToUint64(dvec4 _Value);

int16_t float16BitsToInt16(float16_t _Value);
i16vec2 float16BitsToInt16(f16vec2 _Value);
i16vec3 float16BitsToInt16(f16vec3 _Value);
i16vec4 float16BitsToInt16(f16vec4 _Value);

uint16_t float16BitsToUint16(float16_t _Value);
u16vec2 float16BitsToUint16(f16vec2 _Value);
u16vec3 float16BitsToUint16(f16vec3 _Value);
u16vec4 float16BitsToUint16(f16vec4 _Value);

float16_t int16BitsToHalf(int16_t _Value);
f16vec2 int16BitsToHalf(i16vec2 _Value);
f16vec3 int16BitsToHalf(i16vec3 _Value);
f16vec4 int16BitsToHalf(i16vec4 _Value);

float16_t uint16BitsToHalf(uint16_t _Value);
f16vec2 uint16BitsToHalf(u16vec2 _Value);
f16vec3 uint16BitsToHalf(u16vec3 _Value);
f16vec4 uint16BitsToHalf(u16vec4 _Value);

float32_t intBitsToFloat(int32_t _Value);
f32vec2 intBitsToFloat(i32vec2 _Value);
f32vec3 intBitsToFloat(i32vec3 _Value);
f32vec4 intBitsToFloat(i32vec4 _Value);

float32_t uintBitsToFloat(uint32_t _Value);
f32vec2 uintBitsToFloat(u32vec2 _Value);
f32vec3 uintBitsToFloat(u32vec3 _Value);
f32vec4 uintBitsToFloat(u32vec4 _Value);

double int64BitsToDouble(int64_t _Value);
dvec2 int64BitsToDouble(i64vec2 _Value);
dvec3 int64BitsToDouble(i64vec3 _Value);
dvec4 int64BitsToDouble(i64vec4 _Value);

double uint64BitsToDouble(uint64_t _Value);
dvec2 uint64BitsToDouble(u64vec2 _Value);
dvec3 uint64BitsToDouble(u64vec3 _Value);
dvec4 uint64BitsToDouble(u64vec4 _Value);

float16_t int16BitsToFloat16(int16_t _Value);
f16vec2 int16BitsToFloat16(i16vec2 _Value);
f16vec3 int16BitsToFloat16(i16vec3 _Value);
f16vec4 int16BitsToFloat16(i16vec4 _Value);

float16_t uint16BitsToFloat16(uint16_t _Value);
f16vec2 uint16BitsToFloat16(u16vec2 _Value);
f16vec3 uint16BitsToFloat16(u16vec3 _Value);
f16vec4 uint16BitsToFloat16(u16vec4 _Value);

// 8.3 Common Functions (frexp/ldexp with explicit type)
float frexp(float _X, out int32_t _Exp);
vec2 frexp(vec2 _X, out i32vec2 _Exp);
vec3 frexp(vec3 _X, out i32vec3 _Exp);
vec4 frexp(vec4 _X, out i32vec4 _Exp);

float ldexp(float _X, in int32_t _Exp);
vec2 ldexp(vec2 _X, in i32vec2 _Exp);
vec3 ldexp(vec3 _X, in i32vec3 _Exp);
vec4 ldexp(vec4 _X, in i32vec4 _Exp);
