#define GL_EXT_float8_e5m2_e4m3 1

// GL_EXT_float_e5m2 / GL_EXT_float_e4m3

// Types
// floate5m2_t, fe5m2vec2, fe5m2vec3, fe5m2vec4
// floate4m3_t, fe4m3vec2, fe4m3vec3, fe4m3vec4

// Type aliases
// genFE5M2Type = floate5m2_t, fe5m2vec2, fe5m2vec3, fe5m2vec4
// genFE4M3Type = floate4m3_t, fe4m3vec2, fe4m3vec3, fe4m3vec4
// genI8Type = int8_t, i8vec2, i8vec3, i8vec4
// genU8Type = uint8_t, u8vec2, u8vec3, u8vec4
// genF16Type = float16_t, f16vec2, f16vec3, f16vec4
// genBF16Type = bfloat16_t, bf16vec2, bf16vec3, bf16vec4
// genFType = float
// genDType = double

// Bitcast functions for floate5m2_t
int8_t floate5m2BitsToIntEXT(floate5m2_t _Value);
i8vec2 floate5m2BitsToIntEXT(fe5m2vec2 _Value);
i8vec3 floate5m2BitsToIntEXT(fe5m2vec3 _Value);
i8vec4 floate5m2BitsToIntEXT(fe5m2vec4 _Value);

uint8_t floate5m2BitsToUintEXT(floate5m2_t _Value);
u8vec2 floate5m2BitsToUintEXT(fe5m2vec2 _Value);
u8vec3 floate5m2BitsToUintEXT(fe5m2vec3 _Value);
u8vec4 floate5m2BitsToUintEXT(fe5m2vec4 _Value);

floate5m2_t intBitsToFloate5m2EXT(int8_t _Value);
fe5m2vec2 intBitsToFloate5m2EXT(i8vec2 _Value);
fe5m2vec3 intBitsToFloate5m2EXT(i8vec3 _Value);
fe5m2vec4 intBitsToFloate5m2EXT(i8vec4 _Value);

floate5m2_t uintBitsToFloate5m2EXT(uint8_t _Value);
fe5m2vec2 uintBitsToFloate5m2EXT(u8vec2 _Value);
fe5m2vec3 uintBitsToFloate5m2EXT(u8vec3 _Value);
fe5m2vec4 uintBitsToFloate5m2EXT(u8vec4 _Value);

// Bitcast functions for floate4m3_t
int8_t floate4m3BitsToIntEXT(floate4m3_t _Value);
i8vec2 floate4m3BitsToIntEXT(fe4m3vec2 _Value);
i8vec3 floate4m3BitsToIntEXT(fe4m3vec3 _Value);
i8vec4 floate4m3BitsToIntEXT(fe4m3vec4 _Value);

uint8_t floate4m3BitsToUintEXT(floate4m3_t _Value);
u8vec2 floate4m3BitsToUintEXT(fe4m3vec2 _Value);
u8vec3 floate4m3BitsToUintEXT(fe4m3vec3 _Value);
u8vec4 floate4m3BitsToUintEXT(fe4m3vec4 _Value);

floate4m3_t intBitsToFloate4m3EXT(int8_t _Value);
fe4m3vec2 intBitsToFloate4m3EXT(i8vec2 _Value);
fe4m3vec3 intBitsToFloate4m3EXT(i8vec3 _Value);
fe4m3vec4 intBitsToFloate4m3EXT(i8vec4 _Value);

floate4m3_t uintBitsToFloate4m3EXT(uint8_t _Value);
fe4m3vec2 uintBitsToFloate4m3EXT(u8vec2 _Value);
fe4m3vec3 uintBitsToFloate4m3EXT(u8vec3 _Value);
fe4m3vec4 uintBitsToFloate4m3EXT(u8vec4 _Value);

// Saturated convert functions - floate5m2
void saturatedConvertEXT(out floate5m2_t _Result, float16_t _Value);
void saturatedConvertEXT(out fe5m2vec2 _Result, f16vec2 _Value);
void saturatedConvertEXT(out fe5m2vec3 _Result, f16vec3 _Value);
void saturatedConvertEXT(out fe5m2vec4 _Result, f16vec4 _Value);

void saturatedConvertEXT(out floate5m2_t _Result, bfloat16_t _Value);
void saturatedConvertEXT(out fe5m2vec2 _Result, bf16vec2 _Value);
void saturatedConvertEXT(out fe5m2vec3 _Result, bf16vec3 _Value);
void saturatedConvertEXT(out fe5m2vec4 _Result, bf16vec4 _Value);

void saturatedConvertEXT(out floate5m2_t _Result, float _Value);
void saturatedConvertEXT(out fe5m2vec2 _Result, vec2 _Value);
void saturatedConvertEXT(out fe5m2vec3 _Result, vec3 _Value);
void saturatedConvertEXT(out fe5m2vec4 _Result, vec4 _Value);

void saturatedConvertEXT(out floate5m2_t _Result, double _Value);
void saturatedConvertEXT(out fe5m2vec2 _Result, dvec2 _Value);
void saturatedConvertEXT(out fe5m2vec3 _Result, dvec3 _Value);
void saturatedConvertEXT(out fe5m2vec4 _Result, dvec4 _Value);

// Saturated convert functions - floate4m3
void saturatedConvertEXT(out floate4m3_t _Result, float16_t _Value);
void saturatedConvertEXT(out fe4m3vec2 _Result, f16vec2 _Value);
void saturatedConvertEXT(out fe4m3vec3 _Result, f16vec3 _Value);
void saturatedConvertEXT(out fe4m3vec4 _Result, f16vec4 _Value);

void saturatedConvertEXT(out floate4m3_t _Result, bfloat16_t _Value);
void saturatedConvertEXT(out fe4m3vec2 _Result, bf16vec2 _Value);
void saturatedConvertEXT(out fe4m3vec3 _Result, bf16vec3 _Value);
void saturatedConvertEXT(out fe4m3vec4 _Result, bf16vec4 _Value);

void saturatedConvertEXT(out floate4m3_t _Result, float _Value);
void saturatedConvertEXT(out fe4m3vec2 _Result, vec2 _Value);
void saturatedConvertEXT(out fe4m3vec3 _Result, vec3 _Value);
void saturatedConvertEXT(out fe4m3vec4 _Result, vec4 _Value);

void saturatedConvertEXT(out floate4m3_t _Result, double _Value);
void saturatedConvertEXT(out fe4m3vec2 _Result, dvec2 _Value);
void saturatedConvertEXT(out fe4m3vec3 _Result, dvec3 _Value);
void saturatedConvertEXT(out fe4m3vec4 _Result, dvec4 _Value);

// Saturated convert for cooperative matrix types
// valid if the result type is constructible from the value type
void saturatedConvertEXT(out coopmat _Result, coopmat _Value);
