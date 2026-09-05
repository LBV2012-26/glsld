// [Note] The "_Func" primitive is used for LSP placeholders
// and should not be used directly in shader code.

#define GL_NV_cooperative_matrix2 1

#include "../KHR/GL_KHR_cooperative_matrix.glsl"

const int gl_CooperativeMatrixClampModeUndefinedNV    = 0;
const int gl_CooperativeMatrixClampModeConstantNV     = 1;
const int gl_CooperativeMatrixClampModeClampToEdgeNV  = 2;
const int gl_CooperativeMatrixClampModeRepeatNV       = 3;
const int gl_CooperativeMatrixClampModeMirrorRepeatNV = 4;

const int gl_CooperativeMatrixReduceRowNV             = 0x1;
const int gl_CooperativeMatrixReduceColumnNV          = 0x2;
const int gl_CooperativeMatrixReduceRowAndColumnNV    = 0x3;
const int gl_CooperativeMatrixReduce2x2NV             = 0x4;

tensorLayoutNV createTensorLayoutNV(uint _Dim, uint _Mode);

tensorLayoutNV setTensorLayoutBlockSizeNV(tensorLayoutNV _Layout, uint _BlockSize0);
tensorLayoutNV setTensorLayoutBlockSizeNV(tensorLayoutNV _Layout, uint _BlockSize0, uint _BlockSize1);
tensorLayoutNV setTensorLayoutBlockSizeNV(tensorLayoutNV _Layout, uint _BlockSize0, uint _BlockSize1, uint _BlockSize2);
tensorLayoutNV setTensorLayoutBlockSizeNV(tensorLayoutNV _Layout, uint _BlockSize0, uint _BlockSize1, uint _BlockSize2, uint _BlockSize3);
tensorLayoutNV setTensorLayoutBlockSizeNV(tensorLayoutNV _Layout, uint _BlockSize0, uint _BlockSize1, uint _BlockSize2, uint _BlockSize3, uint _BlockSize4);

tensorLayoutNV setTensorLayoutDimensionNV(tensorLayoutNV _Layout, uint _Dim0);
tensorLayoutNV setTensorLayoutDimensionNV(tensorLayoutNV _Layout, uint _Dim0, uint _Dim1);
tensorLayoutNV setTensorLayoutDimensionNV(tensorLayoutNV _Layout, uint _Dim0, uint _Dim1, uint _Dim2);
tensorLayoutNV setTensorLayoutDimensionNV(tensorLayoutNV _Layout, uint _Dim0, uint _Dim1, uint _Dim2, uint _Dim3);
tensorLayoutNV setTensorLayoutDimensionNV(tensorLayoutNV _Layout, uint _Dim0, uint _Dim1, uint _Dim2, uint _Dim3, uint _Dim4);

tensorLayoutNV setTensorLayoutStrideNV(tensorLayoutNV _Layout, uint _S0);
tensorLayoutNV setTensorLayoutStrideNV(tensorLayoutNV _Layout, uint _S0, uint _S1);
tensorLayoutNV setTensorLayoutStrideNV(tensorLayoutNV _Layout, uint _S0, uint _S1, uint _S2);
tensorLayoutNV setTensorLayoutStrideNV(tensorLayoutNV _Layout, uint _S0, uint _S1, uint _S2, uint _S3);
tensorLayoutNV setTensorLayoutStrideNV(tensorLayoutNV _Layout, uint _S0, uint _S1, uint _S2, uint _S3, uint _S4);

tensorLayoutNV sliceTensorLayoutNV(tensorLayoutNV _Layout, int _Offset0, uint _Span0);
tensorLayoutNV sliceTensorLayoutNV(tensorLayoutNV _Layout, int _Offset0, uint _Span0, int _Offset1, uint _Span1);
tensorLayoutNV sliceTensorLayoutNV(tensorLayoutNV _Layout, int _Offset0, uint _Span0, int _Offset1, uint _Span1, int _Offset2, uint _Span2);
tensorLayoutNV sliceTensorLayoutNV(tensorLayoutNV _Layout, int _Offset0, uint _Span0, int _Offset1, uint _Span1, int _Offset2, uint _Span2, int _Offset3, uint _Span3);
tensorLayoutNV sliceTensorLayoutNV(tensorLayoutNV _Layout, int _Offset0, uint _Span0, int _Offset1, uint _Span1, int _Offset2, uint _Span2, int _Offset3, uint _Span3, int _Offset4, uint _Span4);

tensorLayoutNV setTensorLayoutClampValueNV(tensorLayoutNV _Layout, uint _Value);

tensorViewNV createTensorViewNV(uint _Dim, bool _HasDimensions, uint _P0);
tensorViewNV createTensorViewNV(uint _Dim, bool _HasDimensions, uint _P0, uint _P1);
tensorViewNV createTensorViewNV(uint _Dim, bool _HasDimensions, uint _P0, uint _P1, uint _P2);
tensorViewNV createTensorViewNV(uint _Dim, bool _HasDimensions, uint _P0, uint _P1, uint _P2, uint _P3);
tensorViewNV createTensorViewNV(uint _Dim, bool _HasDimensions, uint _P0, uint _P1, uint _P2, uint _P3, uint _P4);

tensorViewNV setTensorViewDimensionsNV(tensorViewNV _View, uint _Dim0);
tensorViewNV setTensorViewDimensionsNV(tensorViewNV _View, uint _Dim0, uint _Dim1);
tensorViewNV setTensorViewDimensionsNV(tensorViewNV _View, uint _Dim0, uint _Dim1, uint _Dim2);
tensorViewNV setTensorViewDimensionsNV(tensorViewNV _View, uint _Dim0, uint _Dim1, uint _Dim2, uint _Dim3);
tensorViewNV setTensorViewDimensionsNV(tensorViewNV _View, uint _Dim0, uint _Dim1, uint _Dim2, uint _Dim3, uint _Dim4);

tensorViewNV setTensorViewStrideNV(tensorViewNV _View, uint _S0);
tensorViewNV setTensorViewStrideNV(tensorViewNV _View, uint _S0, uint _S1);
tensorViewNV setTensorViewStrideNV(tensorViewNV _View, uint _S0, uint _S1, uint _S2);
tensorViewNV setTensorViewStrideNV(tensorViewNV _View, uint _S0, uint _S1, uint _S2, uint _S3);
tensorViewNV setTensorViewStrideNV(tensorViewNV _View, uint _S0, uint _S1, uint _S2, uint _S3, uint _S4);

tensorViewNV setTensorViewClipNV(tensorViewNV _View, uint _ClipRowOffset, uint _ClipRowSpan, uint _ClipColOffset, uint _ClipColSpan);

void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);

void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);

void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc);

void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc);

void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out float32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout);

void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out float32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);
void coopMatStoreTensorNV(coopmat _Matrix, volatile coherent out f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View);

void coopMatReduceNV(out coopmat _Result, coopmat _Matrix, int _ReduceMask, _Func<float16_t(float16_t, float16_t)> _CombineOp);
void coopMatReduceNV(out coopmat _Result, coopmat _Matrix, int _ReduceMask, _Func<float32_t(float32_t, float32_t)> _CombineOp);
void coopMatReduceNV(out coopmat _Result, coopmat _Matrix, int _ReduceMask, _Func<float64_t(float64_t, float64_t)> _CombineOp);

// _ElemOp may have an arbitrary number of additional parameters matched by the trailing arguments.
// Keep it as a wildcard until _Func supports prefix signatures.
void coopMatPerElementNV(out coopmat _Result, coopmat _Matrix, _Func _ElemOp, ...);

void coopMatTransposeNV(out coopmat _Result, coopmat _Matrix);
