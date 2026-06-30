#define GL_KHR_memory_scope_semantics 1

const int gl_ScopeDevice      = 1;
const int gl_ScopeWorkgroup   = 2;
const int gl_ScopeSubgroup    = 3;
const int gl_ScopeInvocation  = 4;
const int gl_ScopeQueueFamily = 5;

const int gl_SemanticsRelaxed        = 0x0;
const int gl_SemanticsAcquire        = 0x2;
const int gl_SemanticsRelease        = 0x4;
const int gl_SemanticsAcquireRelease = 0x8;
const int gl_SemanticsMakeAvailable  = 0x2000;
const int gl_SemanticsMakeVisible    = 0x4000;
const int gl_SemanticsVolatile       = 0x8000;

const int gl_StorageSemanticsNone   = 0x0;
const int gl_StorageSemanticsBuffer = 0x40;
const int gl_StorageSemanticsShared = 0x100;
const int gl_StorageSemanticsImage  = 0x800;
const int gl_StorageSemanticsOutput = 0x1000;

uint atomicAdd(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicAdd(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicMin(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicMin(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicMax(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicMax(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicAnd(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicAnd(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicOr(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicOr(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicXor(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicXor(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicExchange(inout uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
int atomicExchange(inout int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
uint atomicCompSwap(inout uint _Mem, uint _Compare, uint _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);
int atomicCompSwap(inout int _Mem, int _Compare, int _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);

uint64_t atomicAdd(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicAdd(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicMin(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicMin(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicMax(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicMax(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicAnd(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicAnd(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicOr(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicOr(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicXor(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicXor(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicExchange(inout uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
int64_t atomicExchange(inout int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);
uint64_t atomicCompSwap(inout uint64_t _Mem, uint64_t _Compare, uint64_t _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);
int64_t atomicCompSwap(inout int64_t _Mem, int64_t _Compare, int64_t _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);

uint atomicLoad(in uint _Mem, int _Scope, int _Storage, int _Sem);
int atomicLoad(in int _Mem, int _Scope, int _Storage, int _Sem);
uint64_t atomicLoad(in uint64_t _Mem, int _Scope, int _Storage, int _Sem);
int64_t atomicLoad(in int64_t _Mem, int _Scope, int _Storage, int _Sem);

void atomicStore(out uint _Mem, uint _Data, int _Scope, int _Storage, int _Sem);
void atomicStore(out int _Mem, int _Data, int _Scope, int _Storage, int _Sem);
void atomicStore(out uint64_t _Mem, uint64_t _Data, int _Scope, int _Storage, int _Sem);
void atomicStore(out int64_t _Mem, int64_t _Data, int _Scope, int _Storage, int _Sem);

uint imageAtomicAdd(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicAdd(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicAdd(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicMin(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicMin(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicMin(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicMax(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicMax(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicMax(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicAnd(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicAnd(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicAnd(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicOr(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicOr(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicOr(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicXor(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicXor(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicXor(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicExchange(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
int imageAtomicExchange(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
float imageAtomicExchange(image2D _Image, ivec2 _Coord, float _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicExchange(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
uint imageAtomicCompSwap(image2D _Image, ivec2 _Coord, uint _Compare, uint _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);
int imageAtomicCompSwap(iimage2D _Image, ivec2 _Coord, int _Compare, int _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);
uint imageAtomicCompSwap(uimage2D _Image, ivec2 _Coord, uint _Compare, uint _Data, int _Scope, int _StorageEqual, int _SemEqual, int _StorageUnequal, int _SemUnequal);

uint imageAtomicLoad(image2D _Image, ivec2 _Coord, int _Scope, int _Storage, int _Sem);
int imageAtomicLoad(iimage2D _Image, ivec2 _Coord, int _Scope, int _Storage, int _Sem);
uint imageAtomicLoad(uimage2D _Image, ivec2 _Coord, int _Scope, int _Storage, int _Sem);

void imageAtomicStore(image2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);
void imageAtomicStore(iimage2D _Image, ivec2 _Coord, int _Data, int _Scope, int _Storage, int _Sem);
void imageAtomicStore(uimage2D _Image, ivec2 _Coord, uint _Data, int _Scope, int _Storage, int _Sem);

void controlBarrier(int _Execution, int _Memory, int _Storage, int _Sem);

void memoryBarrier(int _Memory, int _Storage, int _Sem);
void memoryBarrier();
void memoryBarrierBuffer();
void memoryBarrierShared();
void memoryBarrierImage();
void groupMemoryBarrier();
