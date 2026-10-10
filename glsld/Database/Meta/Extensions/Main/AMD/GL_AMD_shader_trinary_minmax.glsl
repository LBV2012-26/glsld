#define GL_AMD_shader_trinary_minmax 1

#define SPV_FMIN3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 1)
#define SPV_UMIN3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 2)
#define SPV_SMIN3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 3)
#define SPV_FMAX3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 4)
#define SPV_UMAX3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 5)
#define SPV_SMAX3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 6)
#define SPV_FMID3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 7)
#define SPV_UMID3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 8)
#define SPV_SMID3_AMD spirv_instruction(extensions = ["SPV_AMD_shader_trinary_minmax"], set = "SPV_AMD_shader_trinary_minmax", id = 9)

SPV_FMIN3_AMD float min3(float x, float y, float z);
SPV_FMIN3_AMD vec2 min3(vec2 x, vec2 y, vec2 z);
SPV_FMIN3_AMD vec3 min3(vec3 x, vec3 y, vec3 z);
SPV_FMIN3_AMD vec4 min3(vec4 x, vec4 y, vec4 z);

#if defined(GL_AMD_gpu_shader_half_float) || defined(GL_EXT_shader_explicit_arithmetic_types_float16)
SPV_FMIN3_AMD float16_t min3(float16_t x, float16_t y, float16_t z);
SPV_FMIN3_AMD f16vec2 min3(f16vec2 x, f16vec2 y, f16vec2 z);
SPV_FMIN3_AMD f16vec3 min3(f16vec3 x, f16vec3 y, f16vec3 z);
SPV_FMIN3_AMD f16vec4 min3(f16vec4 x, f16vec4 y, f16vec4 z);
#endif

SPV_UMIN3_AMD uint min3(uint x, uint y, uint z);
SPV_UMIN3_AMD uvec2 min3(uvec2 x, uvec2 y, uvec2 z);
SPV_UMIN3_AMD uvec3 min3(uvec3 x, uvec3 y, uvec3 z);
SPV_UMIN3_AMD uvec4 min3(uvec4 x, uvec4 y, uvec4 z);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
SPV_UMIN3_AMD uint16_t min3(uint16_t x, uint16_t y, uint16_t z);
SPV_UMIN3_AMD u16vec2 min3(u16vec2 x, u16vec2 y, u16vec2 z);
SPV_UMIN3_AMD u16vec3 min3(u16vec3 x, u16vec3 y, u16vec3 z);
SPV_UMIN3_AMD u16vec4 min3(u16vec4 x, u16vec4 y, u16vec4 z);
#endif

SPV_SMIN3_AMD int min3(int x, int y, int z);
SPV_SMIN3_AMD ivec2 min3(ivec2 x, ivec2 y, ivec2 z);
SPV_SMIN3_AMD ivec3 min3(ivec3 x, ivec3 y, ivec3 z);
SPV_SMIN3_AMD ivec4 min3(ivec4 x, ivec4 y, ivec4 z);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
SPV_SMIN3_AMD int16_t min3(int16_t x, int16_t y, int16_t z);
SPV_SMIN3_AMD i16vec2 min3(i16vec2 x, i16vec2 y, i16vec2 z);
SPV_SMIN3_AMD i16vec3 min3(i16vec3 x, i16vec3 y, i16vec3 z);
SPV_SMIN3_AMD i16vec4 min3(i16vec4 x, i16vec4 y, i16vec4 z);
#endif

SPV_FMAX3_AMD float max3(float x, float y, float z);
SPV_FMAX3_AMD vec2 max3(vec2 x, vec2 y, vec2 z);
SPV_FMAX3_AMD vec3 max3(vec3 x, vec3 y, vec3 z);
SPV_FMAX3_AMD vec4 max3(vec4 x, vec4 y, vec4 z);

#if defined(GL_AMD_gpu_shader_half_float) || defined(GL_EXT_shader_explicit_arithmetic_types_float16)
SPV_FMAX3_AMD float16_t max3(float16_t x, float16_t y, float16_t z);
SPV_FMAX3_AMD f16vec2 max3(f16vec2 x, f16vec2 y, f16vec2 z);
SPV_FMAX3_AMD f16vec3 max3(f16vec3 x, f16vec3 y, f16vec3 z);
SPV_FMAX3_AMD f16vec4 max3(f16vec4 x, f16vec4 y, f16vec4 z);
#endif

SPV_UMAX3_AMD uint max3(uint x, uint y, uint z);
SPV_UMAX3_AMD uvec2 max3(uvec2 x, uvec2 y, uvec2 z);
SPV_UMAX3_AMD uvec3 max3(uvec3 x, uvec3 y, uvec3 z);
SPV_UMAX3_AMD uvec4 max3(uvec4 x, uvec4 y, uvec4 z);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
SPV_UMAX3_AMD uint16_t max3(uint16_t x, uint16_t y, uint16_t z);
SPV_UMAX3_AMD u16vec2 max3(u16vec2 x, u16vec2 y, u16vec2 z);
SPV_UMAX3_AMD u16vec3 max3(u16vec3 x, u16vec3 y, u16vec3 z);
SPV_UMAX3_AMD u16vec4 max3(u16vec4 x, u16vec4 y, u16vec4 z);
#endif

SPV_SMAX3_AMD int max3(int x, int y, int z);
SPV_SMAX3_AMD ivec2 max3(ivec2 x, ivec2 y, ivec2 z);
SPV_SMAX3_AMD ivec3 max3(ivec3 x, ivec3 y, ivec3 z);
SPV_SMAX3_AMD ivec4 max3(ivec4 x, ivec4 y, ivec4 z);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
SPV_SMAX3_AMD int16_t max3(int16_t x, int16_t y, int16_t z);
SPV_SMAX3_AMD i16vec2 max3(i16vec2 x, i16vec2 y, i16vec2 z);
SPV_SMAX3_AMD i16vec3 max3(i16vec3 x, i16vec3 y, i16vec3 z);
SPV_SMAX3_AMD i16vec4 max3(i16vec4 x, i16vec4 y, i16vec4 z);
#endif

SPV_FMID3_AMD float mid3(float x, float y, float z);
SPV_FMID3_AMD vec2 mid3(vec2 x, vec2 y, vec2 z);
SPV_FMID3_AMD vec3 mid3(vec3 x, vec3 y, vec3 z);
SPV_FMID3_AMD vec4 mid3(vec4 x, vec4 y, vec4 z);

#if defined(GL_AMD_gpu_shader_half_float) || defined(GL_EXT_shader_explicit_arithmetic_types_float16)
SPV_FMID3_AMD float16_t mid3(float16_t x, float16_t y, float16_t z);
SPV_FMID3_AMD f16vec2 mid3(f16vec2 x, f16vec2 y, f16vec2 z);
SPV_FMID3_AMD f16vec3 mid3(f16vec3 x, f16vec3 y, f16vec3 z);
SPV_FMID3_AMD f16vec4 mid3(f16vec4 x, f16vec4 y, f16vec4 z);
#endif

SPV_UMID3_AMD uint mid3(uint x, uint y, uint z);
SPV_UMID3_AMD uvec2 mid3(uvec2 x, uvec2 y, uvec2 z);
SPV_UMID3_AMD uvec3 mid3(uvec3 x, uvec3 y, uvec3 z);
SPV_UMID3_AMD uvec4 mid3(uvec4 x, uvec4 y, uvec4 z);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
SPV_UMID3_AMD uint16_t mid3(uint16_t x, uint16_t y, uint16_t z);
SPV_UMID3_AMD u16vec2 mid3(u16vec2 x, u16vec2 y, u16vec2 z);
SPV_UMID3_AMD u16vec3 mid3(u16vec3 x, u16vec3 y, u16vec3 z);
SPV_UMID3_AMD u16vec4 mid3(u16vec4 x, u16vec4 y, u16vec4 z);
#endif

SPV_SMID3_AMD int mid3(int x, int y, int z);
SPV_SMID3_AMD ivec2 mid3(ivec2 x, ivec2 y, ivec2 z);
SPV_SMID3_AMD ivec3 mid3(ivec3 x, ivec3 y, ivec3 z);
SPV_SMID3_AMD ivec4 mid3(ivec4 x, ivec4 y, ivec4 z);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
SPV_SMID3_AMD int16_t mid3(int16_t x, int16_t y, int16_t z);
SPV_SMID3_AMD i16vec2 mid3(i16vec2 x, i16vec2 y, i16vec2 z);
SPV_SMID3_AMD i16vec3 mid3(i16vec3 x, i16vec3 y, i16vec3 z);
SPV_SMID3_AMD i16vec4 mid3(i16vec4 x, i16vec4 y, i16vec4 z);
#endif

#undef SPV_FMIN3_AMD
#undef SPV_UMIN3_AMD
#undef SPV_SMIN3_AMD
#undef SPV_FMAX3_AMD
#undef SPV_UMAX3_AMD
#undef SPV_SMAX3_AMD
#undef SPV_FMID3_AMD
#undef SPV_UMID3_AMD
#undef SPV_SMID3_AMD
