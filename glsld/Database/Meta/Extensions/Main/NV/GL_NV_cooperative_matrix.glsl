#define GL_NV_cooperative_matrix 1

void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent float16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent float32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent float64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out fcoopmatNV _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out icoopmatNV _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatLoadNV(out ucoopmatNV _Matrix, volatile coherent uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out float16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out float32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out float64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(fcoopmatNV _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(icoopmatNV _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out int64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out ivec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out ivec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint32_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uvec2[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);
void coopMatStoreNV(ucoopmatNV _Matrix, volatile coherent out uvec4[] _Buffer, uint _Element, uint _Stride, bool _ColMajor);

fcoopmatNV coopMatMulAddNV(fcoopmatNV a, fcoopmatNV b, fcoopmatNV c);
icoopmatNV coopMatMulAddNV(icoopmatNV a, icoopmatNV b, icoopmatNV c);
ucoopmatNV coopMatMulAddNV(ucoopmatNV a, ucoopmatNV b, ucoopmatNV c);
