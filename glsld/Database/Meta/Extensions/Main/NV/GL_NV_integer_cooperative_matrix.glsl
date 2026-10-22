#define __GLSLD_EXTENSION_ENABLED_GL_NV_integer_cooperative_matrix 1

// GL_NV_integer_cooperative_matrix (defined by the GLSL_NV_cooperative_matrix document)
// Enables the icoopmatNV and ucoopmatNV types and any built-in functions that use them.

class icoopmatNV;
class ucoopmatNV;

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int8) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int64) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types)
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
#endif
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

icoopmatNV coopMatMulAddNV(icoopmatNV _MatA, icoopmatNV _MatB, icoopmatNV _MatC);
ucoopmatNV coopMatMulAddNV(ucoopmatNV _MatA, ucoopmatNV _MatB, ucoopmatNV _MatC);
