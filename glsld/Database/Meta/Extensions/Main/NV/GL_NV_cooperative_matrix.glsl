#define __GLSLD_EXTENSION_ENABLED_GL_NV_cooperative_matrix 1

// Umbrella document "GLSL_NV_cooperative_matrix" defines:
//   GL_NV_cooperative_matrix and GL_NV_integer_cooperative_matrix.
// Enabling GL_NV_integer_cooperative_matrix implicitly enables
// GL_NV_cooperative_matrix.

#include "../KHR/GL_KHR_memory_scope_semantics.glsl"
#if defined(__GLSLD_EXTENSION_ENABLED_GL_NV_integer_cooperative_matrix)
#include "GL_NV_integer_cooperative_matrix.glsl"
#endif

class fcoopmatNV;

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_float16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent float16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent float[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_float64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent float64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_float16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out float16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out float[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_float64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out float64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

fcoopmatNV coopMatMulAddNV(fcoopmatNV _MatA, fcoopmatNV _MatB, fcoopmatNV _MatC);
