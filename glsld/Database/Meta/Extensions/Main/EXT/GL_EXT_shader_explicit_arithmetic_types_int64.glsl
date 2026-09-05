#define GL_EXT_shader_explicit_arithmetic_types_int64 1

// Types (keyword additions):
// int64_t, i64vec2, i64vec3, i64vec4
// uint64_t, u64vec2, u64vec3, u64vec4

// 8.x Integer Pack and Unpack Functions
int64_t pack64(i32vec2 _Value);
uint64_t pack64(u32vec2 _Value);
int64_t pack64(i16vec4 _Value);
uint64_t pack64(u16vec4 _Value);

// Alias pack/unpack functions
int64_t packInt2x32(ivec2 _Value);
uint64_t packUint2x32(uvec2 _Value);
ivec2 unpackInt2x32(int64_t _Value);
uvec2 unpackUint2x32(uint64_t _Value);

int64_t doubleBitsToInt64(double _Value);
i64vec2 doubleBitsToInt64(dvec2 _Value);
i64vec3 doubleBitsToInt64(dvec3 _Value);
i64vec4 doubleBitsToInt64(dvec4 _Value);

uint64_t doubleBitsToUint64(double _Value);
u64vec2 doubleBitsToUint64(dvec2 _Value);
u64vec3 doubleBitsToUint64(dvec3 _Value);
u64vec4 doubleBitsToUint64(dvec4 _Value);

double int64BitsToDouble(int64_t _Value);
dvec2 int64BitsToDouble(i64vec2 _Value);
dvec3 int64BitsToDouble(i64vec3 _Value);
dvec4 int64BitsToDouble(i64vec4 _Value);

double uint64BitsToDouble(uint64_t _Value);
dvec2 uint64BitsToDouble(u64vec2 _Value);
dvec3 uint64BitsToDouble(u64vec3 _Value);
dvec4 uint64BitsToDouble(u64vec4 _Value);
