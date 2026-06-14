#version 460 core

#extension GL_EXT_long_vector : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_KHR_cooperative_matrix : require
#extension GL_KHR_memory_scope_semantics : require
#extension GL_NV_cooperative_matrix_decode_vector : require
#extension GL_NV_cooperative_matrix2 : require

layout(buffer_reference, std430) readonly buffer WeightBuffer {
    uint packed_weights[];
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

void main() {
    coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> matrix;
    
    tensorLayoutNV<2> tensor_layout = createTensorLayoutNV(2, gl_CooperativeMatrixClampModeConstantNV);
    tensor_layout = setTensorLayoutBlockSizeNV(tensor_layout, 1, 32);

    WeightBuffer _Buf = WeightBuffer(device_address.address);

    coopMatLoadTensorNV(matrix, _Buf.packed_weights, 0u,
        tensor_layout, ScalarDecoder, VectorDecoder);
}
