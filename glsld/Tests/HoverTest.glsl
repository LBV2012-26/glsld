#version 460 core
#pragma shader_stage(fragment)

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_control_flow_attributes2 : require
#extension GL_EXT_debug_printf : require
#extension GL_EXT_maximal_reconvergence : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_spirv_intrinsics : require
#extension GL_EXT_subgroup_uniform_control_flow : require
#extension GL_EXT_structured_descriptor_heap : require
#extension GL_KHR_memory_scope_semantics : require
#extension GL_KHR_cooperative_matrix : require

layout(location = 0) in vec3 InPosition;
layout(location = 0) out vec4 FragColor;

layout(set = 1, binding = 0) uniform sampler my_sampler;
layout(set = 2, binding = 0) uniform texture2D my_texture;

layout(std140, set = 0, binding = 1) uniform MyUniformBuffer {
    mat4 my_matrix;
} ubo;

layout(location = 1) pervertexEXT in FragInput {
    mat3 tbn_matrix;
    vec2 tex_coord;
    vec3 frag_pos;
    vec4 light_space_frag_pos;
} input_data;

layout(location = 2) in struct InStruct {
    vec3 field;
} input_struct;

struct LightData {
    vec3 position;
};

layout(std430, binding = 2) readonly buffer LightBuffer {
    LightData lights[];
};

layout(buffer_reference, std430) buffer LightDataBuffer {
    LightBuffer light_buffer;
    LightData data;
};

layout(push_constant) uniform PushConstants {
    layout(offset = 0) uint64_t data;
} push_constants;

layout(buffer_type, scalar) buffer Material {
    vec4 data;
};

layout(heap_offset = push_constants.data) resourceheap ResourceHeap {
    uint global_time;
    layout(descriptor_size = 64) LightData light;
    Material materials[];
} resource_heap;

layout(heap_offset = push_constants.data) samplerheap SamplerHeap {
    sampler repeat;
    sampler shadow;
} sampler_heap;

#define MAX_RETURN_ARRAY_SIZE 24 + 42
#define SQUARE(x) ((x) * (x))

#ifdef MACRO_NAME
#define CONDITIONAL_ASSIGN(x) if (x) { x = 10; }
#endif

int[MAX_RETURN_ARRAY_SIZE] ReturnArray(int source[25 + MAX_RETURN_ARRAY_SIZE][MAX_RETURN_ARRAY_SIZE]) {
    int result[MAX_RETURN_ARRAY_SIZE] = source[0];
    return result;
}

LightData ReturnLightData(int index) {
    LightData result;
    result.position = lights[index].position;
    return result;
}

#ifdef _GLSLD
[[subgroup_uniform_control_flow]]
#endif
void ApplyParameters(const in float input_value, out float output_value, inout float accumulated_value) {
    output_value = input_value + SQUARE(sin(accumulated_value));
    accumulated_value += output_value;

    texture(sampler2D(my_texture, my_sampler), vec2(0.5));
    texture2D(my_texture, vec2(0.5));
}

coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> CreateCooperativeMatrix() {
    coopmat<float16_t, /* template argument comment */ gl_ScopeSubgroup, 16, 16, gl_MatrixUseA> result;
    return result;
}

#define CLAMP_VALUE(value, low, high) clamp(value, low, high)
#define FORWARD(expr) expr
#define WRAP(value) FORWARD(value)

const float kX = WRAP(CLAMP_VALUE(sin(1.2), 0.0, 1.0));

// SPIR-V intrinsics Hover cases.
// Hover declarations and their references in TestSpirvHover().
// Ordinary layout qualifiers stay in the declaration, without a Layout row.
// SPIR-V Storage and Decorate rows should still show complete arguments.

// Storage: Private = 6.
spirv_storage_class(6) float hover_spirv_private;

// Ordinary buffer block: no separate Layout/Storage rows.
// Member decorations: Offset = 35 should still have a Decorate row.
layout(std430, set = 3, binding = 0)
buffer HoverSpirvBlock {
    spirv_decorate(35, 0) vec4 color;
    spirv_decorate(35, 16) float scale;
} hover_spirv_block;

// Literal decoration: Location = 30.
spirv_decorate(30, 8) in vec4 hover_spirv_input;

// ID decoration: HlslCounterBufferGOOGLE = 5634.
// Hover the counted block: Decorate should retain the counter's name.
// Manual Hover fixture: the SDK emits a forward buffer ID reference here,
// which spirv-val rejects even though glslang accepts the declaration.
layout(std430, set = 3, binding = 1) buffer HoverSpirvCounter {
    uint count;
} hover_spirv_counter;

layout(std430, set = 3, binding = 2)
spirv_decorate_id(
    extensions = ["SPV_GOOGLE_hlsl_functionality1"],
    5634, hover_spirv_counter)
buffer HoverSpirvCountedBlock {
    vec4 value;
} hover_spirv_counted;

// String decoration: HlslSemanticGOOGLE = 5635.
layout(location = 9)
spirv_decorate_string(
    extensions = ["SPV_GOOGLE_decorate_string", "SPV_GOOGLE_hlsl_functionality1"],
    5635, "HOVER_COLOR")
in vec4 hover_spirv_semantic;

// Direct and macro-aliased SPIR-V types: OpTypeFloat = 22, width = 32.
spirv_type(id = 22, 32) hover_spirv_raw_float;
#define HoverSpirvFloat spirv_type(id = 22, 32)
HoverSpirvFloat hover_spirv_alias_float;

// Extended instruction: GLSL.std.450 FAbs = 4.
spirv_instruction(set = "GLSL.std.450", id = 4)
float HoverSpirvAbs(float value);

// Core instructions and parameter qualifiers: OpLoad = 61,
// OpCompositeInsert = 82. Hover both the functions and their parameters.
spirv_instruction(id = 61)
float HoverSpirvLoad(spirv_by_reference float source);

spirv_instruction(id = 82)
vec4 HoverSpirvInsert(float component, vec4 value, spirv_literal uint index);

void TestSpirvHover() {
    hover_spirv_private = HoverSpirvAbs(-2.0);
    float loaded = HoverSpirvLoad(hover_spirv_private);
    vec4 inserted = HoverSpirvInsert(loaded, hover_spirv_input, 2u);
    hover_spirv_counted.value = inserted + hover_spirv_semantic;
    hover_spirv_block.color = hover_spirv_counted.value;
    hover_spirv_block.scale = loaded;

    HoverSpirvFloat local_alias_float;
    spirv_type(id = 22, 32) local_raw_float;
}

void main() {
    auto matrix = CreateCooperativeMatrix();
    vector<float, 16> long_vector = {};

    InStruct local_input = input_struct;
    local_input.field = vec3(__VERSION__);

    LightDataBuffer data_buffer = LightDataBuffer(push_constants.data);
    const mat4 model_matrix = ubo.my_matrix;

    int source_array[25 + MAX_RETURN_ARRAY_SIZE][MAX_RETURN_ARRAY_SIZE] = {};
    int returned_array[MAX_RETURN_ARRAY_SIZE] = ReturnArray(source_array);

    float kernel[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    float output_value;
    float accumulated_value = 1.0;
    ApplyParameters((float)ReturnArray(source_array)[0], output_value, accumulated_value);

    vec3 direct_light = normalize(lights[0].position - InPosition), ambient_light = vec3(gl_FragCoord.xyz);

    const int max_iterations = 10;
#define MIN_ITERATIONS max_iterations

    [[unroll, max_iterations(max_iterations), min_iterations(MIN_ITERATIONS)]]
    for (int i = 0; i != max_iterations; ++i) {}

    LightData[24] light_grid[max_iterations][MAX_RETURN_ARRAY_SIZE], light_gird2[42];
    vec3 indexed_position = light_grid[ReturnArray(source_array)[0]][3][25].position + input_data.frag_pos;
    LightData selected_light = ReturnLightData(returned_array[0]);

    vec3 heap_light = resource_heap.light.position;
    vec4 material_data = resource_heap.materials[0].data;
    sampler heap_sampler = sampler_heap.repeat;

    vec3 color = direct_light + ambient_light * selected_light.position + indexed_position + heap_light;
    FragColor = vec4(color, 1.0) * material_data;
    (debugPrintfEXT)("Color: %v3", FragColor);
}
