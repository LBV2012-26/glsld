// [Note] The "_Func" primitive is used for LSP placeholders
// and should not be used directly in shader code.
// Decode function signatures remain wildcard _Func: their first parameter is an arbitrary
// user-defined buffer_reference type and their uint32_t array sizes depend on the tensor dimension.

#define GL_NV_cooperative_matrix_decode_vector 1

void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, _Func _DecodeFunc, _Func _DecodeVectorFunc);

void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent int64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint8_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint32_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent uint64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float16_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent float64_t[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec2[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent i64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u8vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent u64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f16vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f32vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
void coopMatLoadTensorNV(inout coopmat _Matrix, volatile coherent f64vec4[] _Buffer, uint _ElementOffset, tensorLayoutNV _Layout, tensorViewNV _View, _Func _DecodeFunc, _Func _DecodeVectorFunc);
