#define GL_KHR_cooperative_matrix 1

const int gl_MatrixUseA           = 0;
const int gl_MatrixUseB           = 1;
const int gl_MatrixUseAccumulator = 2;

const int gl_CooperativeMatrixLayoutRowMajor    = 0;
const int gl_CooperativeMatrixLayoutColumnMajor = 1;

const int gl_MatrixOperandsSaturatingAccumulation = 0x10;

void coopMatLoad(out coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent float[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatLoad(out coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatLoad(out coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatStore(coopmat _Matrix, volatile coherent out int8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out int16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out int32_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out int64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out uint8_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out uint16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out uint32_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out uint64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out float16_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out float[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out float64_t[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatStore(coopmat _Matrix, volatile coherent out i8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out i16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out i32vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out i64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u8vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u32vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out f16vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out f32vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out f64vec2[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatStore(coopmat _Matrix, volatile coherent out i8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out i16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out i32vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out i64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u8vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u32vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out u64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out f16vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out f32vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _Matrix, volatile coherent out f64vec4[] _Buffer, uint _Element, uint _Stride, int _MatrixLayout);

coopmat coopMatMulAdd(coopmat _A, coopmat _B, coopmat _C, int _MatrixOperands);
coopmat coopMatMulAdd(coopmat _A, coopmat _B, coopmat _C);
