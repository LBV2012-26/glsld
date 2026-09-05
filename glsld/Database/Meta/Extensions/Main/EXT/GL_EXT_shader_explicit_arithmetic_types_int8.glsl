#define GL_EXT_shader_explicit_arithmetic_types_int8 1

// Types (keyword additions):
// int8_t, i8vec2, i8vec3, i8vec4
// uint8_t, u8vec2, u8vec3, u8vec4

i8vec4 unpack8(int32_t _Value);
u8vec4 unpack8(uint32_t _Value);
i8vec2 unpack8(int16_t _Value);
u8vec2 unpack8(uint16_t _Value);
