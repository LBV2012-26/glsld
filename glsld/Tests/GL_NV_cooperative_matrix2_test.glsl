#version 460 core
#pragma shader_stage(compute)

#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_long_vector : require
#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_KHR_cooperative_matrix : require
#extension GL_KHR_memory_scope_semantics : require
#extension GL_NV_cooperative_matrix2 : require

layout(buffer_reference, std430) buffer ScalarBuffer {
    float values[];
};

layout(buffer_reference, std430) buffer HalfBuffer {
    float16_t values[];
};

layout(buffer_reference, std430) buffer PackedBuffer {
    u16vec2 values[];
};

layout(push_constant) uniform DeviceAddress {
    uint64_t scalar_address;
    uint64_t half_address;
    uint64_t packed_address;
} addresses;

float DecodeFloat(const in ScalarBuffer scalar_buffer, const in uint32_t block_coord[2], const in uint32_t coord_in_block[2]) {
    return scalar_buffer.values[block_coord[0]] + float(coord_in_block[1]);
}

float16_t AddHalf(const in float16_t x, const in float16_t y) {
    return x + y;
}

float AddFloat(const in float x, const in float y) {
    return x + y;
}

float32_t MaxFloat32(const in float32_t x, const in float32_t y) {
    return max(x, y);
}

double AddDouble(const in double x, const in double y) {
    return x + y;
}

float64_t MinFloat64(const in float64_t x, const in float64_t y) {
    return min(x, y);
}

float TransformElement(const in uint row, const in uint column, const in float value) {
    return value + float(row + column);
}

float TransformElementScaled(const in uint row, const in uint column, const in float value, const in float scale, const in float bias) {
    return value * scale + bias + float(row + column);
}

float TransformTwoMatrices(const in uint row, const in uint column, const in float lhs, const in float rhs, const in float scale) {
    return (lhs + rhs) * scale + float(row + column);
}

void TestTensorLayouts() {
    tensorLayoutNV<1, 0> layout1 = createTensorLayoutNV(1u, gl_CooperativeMatrixClampModeUndefinedNV);
    layout1 = setTensorLayoutDimensionNV(layout1, 64u);
    layout1 = setTensorLayoutStrideNV(layout1, 1u);
    layout1 = setTensorLayoutBlockSizeNV(layout1, 8u);
    layout1 = sliceTensorLayoutNV(layout1, 4, 16u);

    tensorLayoutNV<2, 1> layout2 = createTensorLayoutNV(2u, gl_CooperativeMatrixClampModeConstantNV);
    layout2 = setTensorLayoutDimensionNV(layout2, 32u, 64u);
    layout2 = setTensorLayoutStrideNV(layout2, 64u, 1u);
    layout2 = setTensorLayoutBlockSizeNV(layout2, 4u, 8u);
    layout2 = setTensorLayoutClampValueNV(layout2, 0u);
    layout2 = sliceTensorLayoutNV(layout2, 2, 16u, 4, 32u);

    tensorLayoutNV<3, 2> layout3 = createTensorLayoutNV(3u, gl_CooperativeMatrixClampModeClampToEdgeNV);
    layout3 = setTensorLayoutDimensionNV(layout3, 4u, 8u, 16u);
    layout3 = setTensorLayoutStrideNV(layout3, 128u, 16u, 1u);
    layout3 = setTensorLayoutBlockSizeNV(layout3, 1u, 2u, 4u);
    layout3 = sliceTensorLayoutNV(layout3, 0, 4u, 1, 4u, 2, 8u);

    tensorLayoutNV<4, 3> layout4 = createTensorLayoutNV(4u, gl_CooperativeMatrixClampModeRepeatNV);
    layout4 = setTensorLayoutDimensionNV(layout4, 2u, 4u, 8u, 16u);
    layout4 = setTensorLayoutStrideNV(layout4, 512u, 128u, 16u, 1u);
    layout4 = setTensorLayoutBlockSizeNV(layout4, 1u, 1u, 2u, 4u);
    layout4 = sliceTensorLayoutNV(layout4, 0, 2u, 0, 4u, 2, 4u, 4, 8u);

    tensorLayoutNV<5, 4> layout5 = createTensorLayoutNV(5u, gl_CooperativeMatrixClampModeMirrorRepeatNV);
    layout5 = setTensorLayoutDimensionNV(layout5, 2u, 3u, 4u, 5u, 6u);
    layout5 = setTensorLayoutStrideNV(layout5, 360u, 120u, 30u, 6u, 1u);
    layout5 = setTensorLayoutBlockSizeNV(layout5, 1u, 1u, 2u, 1u, 3u);
    layout5 = sliceTensorLayoutNV(layout5, 0, 2u, 0, 3u, 1, 2u, 0, 5u, 3, 3u);
}

void TestTensorViews() {
    tensorViewNV<1, false, 0> view1 = createTensorViewNV(1u, false, 0u);
    view1 = setTensorViewDimensionsNV(view1, 64u);
    view1 = setTensorViewStrideNV(view1, 1u);

    tensorViewNV<2, true, 1, 0> view2 = createTensorViewNV(2u, true, 1u, 0u);
    view2 = setTensorViewDimensionsNV(view2, 64u, 32u);
    view2 = setTensorViewStrideNV(view2, 1u, 64u);
    view2 = setTensorViewClipNV(view2, 2u, 16u, 4u, 8u);

    tensorViewNV<3, true, 2, 1, 0> view3 = createTensorViewNV(3u, true, 2u, 1u, 0u);
    view3 = setTensorViewDimensionsNV(view3, 16u, 8u, 4u);
    view3 = setTensorViewStrideNV(view3, 1u, 16u, 128u);

    tensorViewNV<4, true, 3, 2, 1, 0> view4 = createTensorViewNV(4u, true, 3u, 2u, 1u, 0u);
    view4 = setTensorViewDimensionsNV(view4, 16u, 8u, 4u, 2u);
    view4 = setTensorViewStrideNV(view4, 1u, 16u, 128u, 512u);

    tensorViewNV<5, true, 4, 3, 2, 1, 0> view5 = createTensorViewNV(5u, true, 4u, 3u, 2u, 1u, 0u);
    view5 = setTensorViewDimensionsNV(view5, 6u, 5u, 4u, 3u, 2u);
    view5 = setTensorViewStrideNV(view5, 1u, 6u, 30u, 120u, 360u);
}

void TestLoadStoreAndDecode(ScalarBuffer scalar_buffer, HalfBuffer half_buffer, PackedBuffer packed_buffer) {
    coopmat<float, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> float_matrix;
    coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> half_matrix;
    tensorLayoutNV<2, 1> layout2 = createTensorLayoutNV(2u, gl_CooperativeMatrixClampModeConstantNV);
    layout2 = setTensorLayoutDimensionNV(layout2, 16u, 16u);
    tensorViewNV<2, false, 1, 0> view2 = createTensorViewNV(2u, false, 1u, 0u);

    coopMatLoadTensorNV(float_matrix, scalar_buffer.values, 0u, layout2);
    coopMatLoadTensorNV(float_matrix, scalar_buffer.values, 4u, layout2, view2);
    coopMatLoadTensorNV(float_matrix, scalar_buffer.values, 8u, layout2, DecodeFloat);
    coopMatLoadTensorNV(float_matrix, scalar_buffer.values, 12u, layout2, view2, DecodeFloat);
    coopMatLoadTensorNV(half_matrix, half_buffer.values, 0u, layout2);
    coopMatLoadTensorNV(half_matrix, packed_buffer.values, 0u, layout2, view2);

    coopMatStoreTensorNV(float_matrix, scalar_buffer.values, 0u, layout2);
    coopMatStoreTensorNV(float_matrix, scalar_buffer.values, 4u, layout2, view2);
    coopMatStoreTensorNV(half_matrix, half_buffer.values, 0u, layout2);
    coopMatStoreTensorNV(half_matrix, packed_buffer.values, 0u, layout2, view2);
}

void TestMatrixOperations() {
    coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> half_source;
    coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> half_result;
    coopmat<float, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> float_source;
    coopmat<float, gl_ScopeSubgroup, 16, 16, gl_MatrixUseAccumulator> float_result;
    coopmat<double, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> double_source;
    coopmat<double, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> double_result;
    coopmat<double, gl_ScopeSubgroup, 4, 4, gl_MatrixUseAccumulator> double_2x2_result;
    coopmat<float, gl_ScopeSubgroup, 16, 16, gl_MatrixUseB> transposed;

    coopMatReduceNV(half_result, half_source, gl_CooperativeMatrixReduceRowNV, AddHalf);
    coopMatReduceNV(float_result, float_source, gl_CooperativeMatrixReduceColumnNV, AddFloat);
    coopMatReduceNV(float_result, float_source, gl_CooperativeMatrixReduceRowAndColumnNV, MaxFloat32);
    coopMatReduceNV(double_2x2_result, double_source, gl_CooperativeMatrixReduce2x2NV, AddDouble);
    coopMatReduceNV(double_result, double_source, gl_CooperativeMatrixReduceRowNV, MinFloat64);

    coopMatPerElementNV(float_result, float_source, TransformElement);
    coopMatPerElementNV(float_result, float_source, TransformElementScaled, 2.0, 1.0);
    coopMatPerElementNV(float_result, float_source, TransformTwoMatrices, float_result, 0.5);
    coopMatTransposeNV(transposed, float_source);
}

void main() {
    ScalarBuffer scalar_buffer = ScalarBuffer(addresses.scalar_address);
    HalfBuffer half_buffer = HalfBuffer(addresses.half_address);
    PackedBuffer packed_buffer = PackedBuffer(addresses.packed_address);

    TestTensorLayouts();
    TestTensorViews();
    TestLoadStoreAndDecode(scalar_buffer, half_buffer, packed_buffer);
    TestMatrixOperations();
}
