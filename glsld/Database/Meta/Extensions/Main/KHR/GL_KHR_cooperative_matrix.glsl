#define GL_KHR_cooperative_matrix 1

const int gl_MatrixUseA           = 0;
const int gl_MatrixUseB           = 1;
const int gl_MatrixUseAccumulator = 2;

const int gl_CooperativeMatrixLayoutRowMajor    = 0;
const int gl_CooperativeMatrixLayoutColumnMajor = 1;

const int gl_MatrixOperandsSaturatingAccumulation = 0x10;

#include "../../Common/GL_KHR_cooperative_matrix_defines.glsl"

#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent int[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent uint[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent float[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif

#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent ivec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif

#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent ivec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatLoad(out coopmat _Matrix, volatile coherent vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoad(out coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif

#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out int8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out int16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out int[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out int64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out uint[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out float16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out float[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out float64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif

#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out i8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out i16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out ivec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out i64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out u8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out u16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out u64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out f16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out f64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif

#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out i8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out i16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out ivec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out i64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out u8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out u16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out u64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out f16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif
void coopMatStore(coopmat _Matrix, volatile coherent out vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void coopMatStore(coopmat _Matrix, volatile coherent out f64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
#endif

coopmat coopMatMulAdd(coopmat _MatA, coopmat _MatB, coopmat _MatC, int _MatrixOperands);
coopmat coopMatMulAdd(coopmat _MatA, coopmat _MatB, coopmat _MatC);
