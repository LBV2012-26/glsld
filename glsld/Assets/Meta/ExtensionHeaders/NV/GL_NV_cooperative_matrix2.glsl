const int gl_CooperativeMatrixClampModeUndefinedNV    = 0;
const int gl_CooperativeMatrixClampModeConstantNV     = 1;
const int gl_CooperativeMatrixClampModeClampToEdgeNV  = 2;
const int gl_CooperativeMatrixClampModeRepeatNV       = 3;
const int gl_CooperativeMatrixClampModeMirrorRepeatNV = 4;

const int gl_CooperativeMatrixReduceRowNV             = 0x1;
const int gl_CooperativeMatrixReduceColumnNV          = 0x2;
const int gl_CooperativeMatrixReduceRowAndColumnNV    = 0x3;
const int gl_CooperativeMatrixReduce2x2NV             = 0x4;

tensorLayoutNV<Dim, Mode> createTensorLayoutNV(uint32_t _Dim, uint32_t _Mode);
tensorLayoutNV<N, ...> setTensorLayoutBlockSizeNV(tensorLayoutNV<N, ...> _T, uint32_t _BlockSize0, ..., uint32_t _BlockSizeN1);
tensorLayoutNV<N, ...> setTensorLayoutDimensionNV(tensorLayoutNV<N, ...> _T, uint32_t _Dim0, ..., uint32_t _DimN1);
tensorLayoutNV<N, ...> setTensorLayoutStrideNV(tensorLayoutNV<N, ...> _T, uint32_t _S0, ..., uint32_t _SN1);
tensorLayoutNV<N, ...> sliceTensorLayoutNV(tensorLayoutNV<N, ...> _T, int32_t _Offset0, uint32_t _Span0, ..., int32_t _OffsetN1, uint32_t _SpanN1);
tensorLayoutNV<...> setTensorLayoutClampValueNV(tensorLayoutNV<...> _T, uint32_t _Value);

tensorViewNV<Dim, hasDimensions, p0, ..., pDim1> createTensorViewNV(uint32_t _Dim, bool _HasDimensions, uint32_t _P0, ..., uint32_t _PDim1);
tensorViewNV<N> setTensorViewDimensionsNV(tensorViewNV<N> _V, uint32_t _Dim0, ..., uint32_t _DimN1);
tensorViewNV<N, ...> setTensorViewStrideNV(tensorViewNV<N, ...> _V, uint32_t _S0, ..., uint32_t _SN1);
tensorViewNV<N> setTensorViewClipNV(tensorViewNV<N> _V, uint _ClipRowOffset, uint _ClipRowSpan, uint _ClipColOffset, uint _ClipColSpan);

void coopMatLoadTensorNV(inout coopmat _M, volatile coherent T[] _Buf, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _M, volatile coherent T[] _Buf, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _M, volatile coherent T[] _Buf, uint _ElementOffset, tensorLayoutNV _Layout, T2 _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _M, volatile coherent T[] _Buf, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, T2 _DecodeFunc);

void coopMatStoreTensorNV(coopmat _M, volatile coherent out T[] _Buf, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _M, volatile coherent out T[] _Buf, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);

void coopMatReduceNV(out coopmat _Result, coopmat _M, int _ReduceMask, T _CombineOp);

void coopMatPerElementNV(out coopmat _Result, coopmat _M, T _ElemOp, ...);

void coopMatTransposeNV(out coopmat _Result, coopmat _M);
