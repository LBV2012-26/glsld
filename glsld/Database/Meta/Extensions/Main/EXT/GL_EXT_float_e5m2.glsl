#define GL_EXT_float_e5m2 1

// GL_EXT_float_e5m2 (defined by the GL_EXT_float8_e5m2_e4m3 document)

// Types
// floate5m2_t, fe5m2vec2, fe5m2vec3, fe5m2vec4

// Type aliases
// genFE5M2Type = floate5m2_t, fe5m2vec2, fe5m2vec3, fe5m2vec4
// genI8Type = int8_t, i8vec2, i8vec3, i8vec4
// genU8Type = uint8_t, u8vec2, u8vec3, u8vec4

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

// Cooperative matrix load/store overloads (spec "Modify Section 8.X")
void coopMatLoad(out coopmat _Matrix, volatile coherent floate5m2_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent fe5m2vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent fe5m2vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatStore(coopmat _Matrix, volatile coherent out floate5m2_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out fe5m2vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out fe5m2vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
