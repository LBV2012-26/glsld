// GL_EXT_shader_atomic_int64

// Atomic memory functions for 64-bit integer types

uint64_t atomicMin(inout uint64_t _Mem, uint64_t _Data);
uint64_t atomicMax(inout uint64_t _Mem, uint64_t _Data);
uint64_t atomicAnd(inout uint64_t _Mem, uint64_t _Data);
uint64_t atomicOr(inout uint64_t _Mem, uint64_t _Data);
uint64_t atomicXor(inout uint64_t _Mem, uint64_t _Data);

int64_t atomicMin(inout int64_t _Mem, int64_t _Data);
int64_t atomicMax(inout int64_t _Mem, int64_t _Data);
int64_t atomicAnd(inout int64_t _Mem, int64_t _Data);
int64_t atomicOr(inout int64_t _Mem, int64_t _Data);
int64_t atomicXor(inout int64_t _Mem, int64_t _Data);

uint64_t atomicAdd(inout uint64_t _Mem, uint64_t _Data);
uint64_t atomicExchange(inout uint64_t _Mem, uint64_t _Data);
uint64_t atomicCompSwap(inout uint64_t _Mem, uint64_t _Compare, uint64_t _Data);

int64_t atomicAdd(inout int64_t _Mem, int64_t _Data);
int64_t atomicExchange(inout int64_t _Mem, int64_t _Data);
int64_t atomicCompSwap(inout int64_t _Mem, int64_t _Compare, int64_t _Data);
