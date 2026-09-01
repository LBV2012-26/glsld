#version 460 core
#pragma shader_stage(compute)

#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_long_vector : require
#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_KHR_cooperative_matrix : require
#extension GL_KHR_memory_scope_semantics : require
#extension GL_NV_cooperative_matrix_decode_vector : require
#extension GL_NV_cooperative_matrix2 : require

layout(buffer_reference, std430) readonly buffer WeightBuffer {
    uint packed_weights[];
};

layout(buffer_reference, std430) readonly buffer PackedPairBuffer {
    u16vec2 packed_pairs[];
};

layout(buffer_reference, std430) readonly buffer FloatWeightBuffer {
    float packed_weights[];
};

layout(push_constant) uniform DeviceAddress {
    uint64_t address;
} device_address;

float16_t ScalarDecoder(
    const in WeightBuffer weight_buffer,
    const in uint32_t     block_coord[2],
    const in uint32_t     coord_in_block[2])
{
    uint element_index = coord_in_block[1];
    uint dword_index   = element_index / 8;
    uint bit_shift     = (element_index % 8) * 4;

    uint raw_bit = (weight_buffer.packed_weights[dword_index] >> bit_shift) & 0xFu;
    return float16_t(raw_bit);
}

vector<float16_t, 8> VectorDecoder(
    const in WeightBuffer weight_buffer,
    const in uint32_t     block_coord[2],
    const in uint32_t     coord_in_block[2])
{
    uint dword_index = coord_in_block[1] / 8;
    uint weights     = weight_buffer.packed_weights[dword_index];

    return vector<float16_t, 8>(
        float16_t((weights >> 0)  & 0xFu),
        float16_t((weights >> 4)  & 0xFu),
        float16_t((weights >> 8)  & 0xFu),
        float16_t((weights >> 12) & 0xFu),
        float16_t((weights >> 16) & 0xFu),
        float16_t((weights >> 20) & 0xFu),
        float16_t((weights >> 24) & 0xFu),
        float16_t((weights >> 28) & 0xFu)
    );
}

float16_t ScalarDecoderPair(const in PackedPairBuffer weight_buffer, const in uint32_t block_coord[3], const in uint32_t coord_in_block[3]) {
    return float16_t(weight_buffer.packed_pairs[block_coord[0]].x + coord_in_block[2]);
}

f16vec2 VectorDecoder2(const in PackedPairBuffer weight_buffer, const in uint32_t block_coord[3], const in uint32_t coord_in_block[3]) {
    u16vec2 packed = weight_buffer.packed_pairs[block_coord[0]];
    return f16vec2(packed) + f16vec2(coord_in_block[2]);
}

float ScalarDecoderFloat(const in FloatWeightBuffer weight_buffer, const in uint32_t block_coord[4], const in uint32_t coord_in_block[4]) {
    return weight_buffer.packed_weights[block_coord[0]] + float(coord_in_block[3]);
}

vec4 VectorDecoder4(const in FloatWeightBuffer weight_buffer, const in uint32_t block_coord[4], const in uint32_t coord_in_block[4]) {
    float base = weight_buffer.packed_weights[block_coord[0]];
    return vec4(base) + vec4(coord_in_block[3], 1u, 2u, 3u);
}

void TestVectorDecode2(PackedPairBuffer weight_buffer) {
    coopmat<float16_t, gl_ScopeSubgroup, 8, 8, gl_MatrixUseAccumulator> matrix;
    tensorLayoutNV<3, 1> layout3 = createTensorLayoutNV(3, gl_CooperativeMatrixClampModeConstantNV);
    layout3 = setTensorLayoutDimensionNV(layout3, 4u, 8u, 16u);
    layout3 = setTensorLayoutBlockSizeNV(layout3, 1u, 2u, 8u);
    layout3 = setTensorLayoutStrideNV(layout3, 128u, 16u, 1u);
    layout3 = sliceTensorLayoutNV(layout3, 0, 4u, 1, 8u, 2, 8u);
    coopMatLoadTensorNV(matrix, weight_buffer.packed_pairs, 2u, layout3, ScalarDecoderPair, VectorDecoder2);
}

void TestVectorDecode4WithView(FloatWeightBuffer weight_buffer) {
    coopmat<float, gl_ScopeSubgroup, 16, 8, gl_MatrixUseB> matrix;
    tensorLayoutNV<4, 2> layout4 = createTensorLayoutNV(4, gl_CooperativeMatrixClampModeClampToEdgeNV);
    layout4 = setTensorLayoutDimensionNV(layout4, 2u, 4u, 8u, 16u);
    layout4 = setTensorLayoutBlockSizeNV(layout4, 1u, 1u, 2u, 8u);

    tensorViewNV<4, true, 3, 2, 1, 0> view4 = createTensorViewNV(4, true, 3u, 2u, 1u, 0u);
    view4 = setTensorViewDimensionsNV(view4, 16u, 8u, 4u, 2u);
    view4 = setTensorViewStrideNV(view4, 1u, 16u, 128u, 512u);
    view4 = setTensorViewClipNV(view4, 0u, 16u, 0u, 8u);

    coopMatLoadTensorNV(matrix, weight_buffer.packed_weights, 4u, layout4, view4, ScalarDecoderFloat, VectorDecoder4);
}

void TestVectorDecode8WithView(WeightBuffer weight_buffer) {
    coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matrix;
    tensorLayoutNV<2, 1> layout2 = createTensorLayoutNV(2, gl_CooperativeMatrixClampModeConstantNV);
    layout2 = setTensorLayoutDimensionNV(layout2, 16u, 32u);
    layout2 = setTensorLayoutBlockSizeNV(layout2, 1u, 32u);

    tensorViewNV<2, false, 1, 0> view2 = createTensorViewNV(2, false, 1u, 0u);
    view2 = setTensorViewStrideNV(view2, 1u, 32u);
    coopMatLoadTensorNV(matrix, weight_buffer.packed_weights, 8u, layout2, view2, ScalarDecoder, VectorDecoder);
}

void main() {
    coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matrix;
    
    tensorLayoutNV<2, 1> tensor_layout = createTensorLayoutNV(2, gl_CooperativeMatrixClampModeConstantNV);
    tensor_layout = setTensorLayoutBlockSizeNV(tensor_layout, 1, 32);

    WeightBuffer weight_buffer = WeightBuffer(device_address.address);

    coopMatLoadTensorNV(matrix, weight_buffer.packed_weights, 0u, tensor_layout,
                        ScalarDecoder, VectorDecoder);

    TestVectorDecode2(PackedPairBuffer(device_address.address));
    TestVectorDecode4WithView(FloatWeightBuffer(device_address.address));
    TestVectorDecode8WithView(weight_buffer);
}
