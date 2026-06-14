#define GL_EXT_expect_assume 1

// GL_EXT_expect_assume

// Optimization hint functions
void assumeEXT(bool _Condition);

bool expectEXT(bool _Value, bool _ExpectedValue);
bvec2 expectEXT(bvec2 _Value, bvec2 _ExpectedValue);
bvec3 expectEXT(bvec3 _Value, bvec3 _ExpectedValue);
bvec4 expectEXT(bvec4 _Value, bvec4 _ExpectedValue);

int expectEXT(int _Value, int _ExpectedValue);
ivec2 expectEXT(ivec2 _Value, ivec2 _ExpectedValue);
ivec3 expectEXT(ivec3 _Value, ivec3 _ExpectedValue);
ivec4 expectEXT(ivec4 _Value, ivec4 _ExpectedValue);

uint expectEXT(uint _Value, uint _ExpectedValue);
uvec2 expectEXT(uvec2 _Value, uvec2 _ExpectedValue);
uvec3 expectEXT(uvec3 _Value, uvec3 _ExpectedValue);
uvec4 expectEXT(uvec4 _Value, uvec4 _ExpectedValue);

// Conditionally available with GL_EXT_shader_explicit_arithmetic_types_int8:
int8_t expectEXT(int8_t _Value, int8_t _ExpectedValue);
i8vec2 expectEXT(i8vec2 _Value, i8vec2 _ExpectedValue);
i8vec3 expectEXT(i8vec3 _Value, i8vec3 _ExpectedValue);
i8vec4 expectEXT(i8vec4 _Value, i8vec4 _ExpectedValue);

uint8_t expectEXT(uint8_t _Value, uint8_t _ExpectedValue);
u8vec2 expectEXT(u8vec2 _Value, u8vec2 _ExpectedValue);
u8vec3 expectEXT(u8vec3 _Value, u8vec3 _ExpectedValue);
u8vec4 expectEXT(u8vec4 _Value, u8vec4 _ExpectedValue);

// Conditionally available with GL_EXT_shader_explicit_arithmetic_types_int16:
int16_t expectEXT(int16_t _Value, int16_t _ExpectedValue);
i16vec2 expectEXT(i16vec2 _Value, i16vec2 _ExpectedValue);
i16vec3 expectEXT(i16vec3 _Value, i16vec3 _ExpectedValue);
i16vec4 expectEXT(i16vec4 _Value, i16vec4 _ExpectedValue);

uint16_t expectEXT(uint16_t _Value, uint16_t _ExpectedValue);
u16vec2 expectEXT(u16vec2 _Value, u16vec2 _ExpectedValue);
u16vec3 expectEXT(u16vec3 _Value, u16vec3 _ExpectedValue);
u16vec4 expectEXT(u16vec4 _Value, u16vec4 _ExpectedValue);

// Conditionally available with GL_EXT_shader_explicit_arithmetic_types_int64:
int64_t expectEXT(int64_t _Value, int64_t _ExpectedValue);
i64vec2 expectEXT(i64vec2 _Value, i64vec2 _ExpectedValue);
i64vec3 expectEXT(i64vec3 _Value, i64vec3 _ExpectedValue);
i64vec4 expectEXT(i64vec4 _Value, i64vec4 _ExpectedValue);

uint64_t expectEXT(uint64_t _Value, uint64_t _ExpectedValue);
u64vec2 expectEXT(u64vec2 _Value, u64vec2 _ExpectedValue);
u64vec3 expectEXT(u64vec3 _Value, u64vec3 _ExpectedValue);
u64vec4 expectEXT(u64vec4 _Value, u64vec4 _ExpectedValue);
