#define GL_EXT_shader_explicit_arithmetic_types_int32 1

// Types (keyword additions):
// int32_t, i32vec2, i32vec3, i32vec4
// uint32_t, u32vec2, u32vec3, u32vec4

// 8.x Integer Pack and Unpack Functions
int32_t pack32(i16vec2 _Value);
uint32_t pack32(u16vec2 _Value);
int32_t pack32(i8vec4 _Value);
uint32_t pack32(u8vec4 _Value);

i32vec2 unpack32(int64_t _Value);
u32vec2 unpack32(uint64_t _Value);
