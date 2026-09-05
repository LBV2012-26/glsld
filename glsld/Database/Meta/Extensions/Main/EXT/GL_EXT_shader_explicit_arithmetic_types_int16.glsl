#define GL_EXT_shader_explicit_arithmetic_types_int16 1

// Types (keyword additions):
// int16_t, i16vec2, i16vec3, i16vec4
// uint16_t, u16vec2, u16vec3, u16vec4

int16_t pack16(i8vec2 _Value);
uint16_t pack16(u8vec2 _Value);

i16vec4 unpack16(int64_t _Value);
u16vec4 unpack16(uint64_t _Value);
i16vec2 unpack16(int32_t _Value);
u16vec2 unpack16(uint32_t _Value);

int32_t packInt2x16(i16vec2 _Value);
int64_t packInt4x16(i16vec4 _Value);
uint32_t packUint2x16(u16vec2 _Value);
uint64_t packUint4x16(u16vec4 _Value);

i16vec2 unpackInt2x16(int32_t _Value);
i16vec4 unpackInt4x16(int64_t _Value);
u16vec2 unpackUint2x16(uint32_t _Value);
u16vec4 unpackUint4x16(uint64_t _Value);
