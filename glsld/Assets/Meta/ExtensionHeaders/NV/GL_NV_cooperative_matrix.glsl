#define GL_NV_cooperative_matrix 1

void coopMatLoadNV(out fcoopmatNV _M, volatile coherent float16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent float[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent float64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent uint8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent uint16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent uint[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent uint64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent uvec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _M, volatile coherent uvec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);

void coopMatLoadNV(out icoopmatNV _M, volatile coherent int8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent int16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent int[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent int64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent ivec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent ivec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent uint8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent uint16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent uint[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent uint64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent uvec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _M, volatile coherent uvec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);

void coopMatLoadNV(out ucoopmatNV _M, volatile coherent int8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent int16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent int[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent int64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent ivec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent ivec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent uint8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent uint16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent uint[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent uint64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent uvec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _M, volatile coherent uvec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);

void coopMatStoreNV(fcoopmatNV _M, volatile coherent out float16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out float[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out float64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out uint8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out uint16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out uint[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out uint64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out uvec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _M, volatile coherent out uvec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);

void coopMatStoreNV(icoopmatNV _M, volatile coherent out int8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out int16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out int[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out int64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out ivec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out ivec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out uint8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out uint16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out uint[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out uint64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out uvec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _M, volatile coherent out uvec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);

void coopMatStoreNV(ucoopmatNV _M, volatile coherent out int8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out int16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out int[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out int64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out ivec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out ivec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out uint8_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out uint16_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out uint[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out uint64_t[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out uvec2[] _Buf, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _M, volatile coherent out uvec4[] _Buf, uint _Element, uint _Stride, bool _ColMajor);

fcoopmatNV coopMatMulAddNV(fcoopmatNV _A, fcoopmatNV _B, fcoopmatNV _C);
icoopmatNV coopMatMulAddNV(icoopmatNV _A, icoopmatNV _B, icoopmatNV _C);
ucoopmatNV coopMatMulAddNV(ucoopmatNV _A, ucoopmatNV _B, ucoopmatNV _C);
