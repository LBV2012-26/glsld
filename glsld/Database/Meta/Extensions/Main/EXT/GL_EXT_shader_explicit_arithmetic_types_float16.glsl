#define GL_EXT_shader_explicit_arithmetic_types_float16 1

// Types (keyword additions):
// float16_t, f16vec2, f16vec3, f16vec4
// f16mat2, f16mat3, f16mat4
// f16mat2x2, f16mat2x3, f16mat2x4
// f16mat3x2, f16mat3x3, f16mat3x4
// f16mat4x2, f16mat4x3, f16mat4x4

uint32_t packFloat2x16(f16vec2 _Value);
f16vec2 unpackFloat2x16(uint32_t _Value);

int16_t halfBitsToInt16(float16_t _Value);
i16vec2 halfBitsToInt16(f16vec2 _Value);
i16vec3 halfBitsToInt16(f16vec3 _Value);
i16vec4 halfBitsToInt16(f16vec4 _Value);

uint16_t halfBitsToUint16(float16_t _Value);
u16vec2 halfBitsToUint16(f16vec2 _Value);
u16vec3 halfBitsToUint16(f16vec3 _Value);
u16vec4 halfBitsToUint16(f16vec4 _Value);

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

float16_t int16BitsToFloat16(int16_t _Value);
f16vec2 int16BitsToFloat16(i16vec2 _Value);
f16vec3 int16BitsToFloat16(i16vec3 _Value);
f16vec4 int16BitsToFloat16(i16vec4 _Value);

float16_t uint16BitsToFloat16(uint16_t _Value);
f16vec2 uint16BitsToFloat16(u16vec2 _Value);
f16vec3 uint16BitsToFloat16(u16vec3 _Value);
f16vec4 uint16BitsToFloat16(u16vec4 _Value);
