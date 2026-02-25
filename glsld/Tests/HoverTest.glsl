#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_subgroup_uniform_control_flow : require
#extension GL_EXT_maximal_reconvergence : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_control_flow_attributes2 : require

layout(location = 0) in  vec3 InPosition;
layout(location = 0) out vec4 FragColor;

layout(set = 1, binding = 0) uniform sampler   my_sampler;
layout(set = 2, binding = 0) uniform texture2D my_texture;

layout(std140, set = 0, binding = 1) uniform MyUniformBuffer { // without set, auto fill set to 0
    mat4 my_matrix;
} ubo;

layout(location = 1) in FragInput {
	mat3 tbn_matrix;
	vec2 tex_coord;
	vec3 frag_pos;
	vec4 light_space_frag_pos;
} input;

struct LightData {
    vec3 position;
};

layout(std430, binding = 2) buffer LightBuffer {
    LightData lights[];
};

layout(buffer_reference, std430) buffer LightDataBuffer {
    LightData data;
} light_data_buffer;

layout(push_constant) uniform PushConstants {
    layout(offset = 0) uint64_t push_constant_value;
} push_constants;

#define MAX_RETURN_ARRAY_SIZE 5
#define MACRO_FUNC(x) (x * x)

int[MAX_RETURN_ARRAY_SIZE] ReturnArray(int mdarray[25 + MAX_RETURN_ARRAY_SIZE][MAX_RETURN_ARRAY_SIZE]) {
    int array[MAX_RETURN_ARRAY_SIZE] = mdarray[0];
    return array;
}

LightData ReturnLightData(int param) {
    LightData data;
    return data;
}

[[subgroup_uniform_control_flow]] void Func(const in float input_arg, out float output_arg, inout float param) {
    output_arg = input_arg + MACRO_FUNC(param);

    texture(sampler2D(my_texture, my_sampler), vec2(0.5));
    // texture2D(my_texture, vec2(0.5)); // test constructor
}

void main() {
    LightDataBuffer data_buffer = LightDataBuffer(push_constants.push_constant_value);
    mat4 my_matrix = ubo.my_matrix;
    int mdarray[25 + MAX_RETURN_ARRAY_SIZE][MAX_RETURN_ARRAY_SIZE];
    int array[MAX_RETURN_ARRAY_SIZE] = ReturnArray(mdarray);

    vec3 light = normalize(lights[0].position - InPosition), ambient = vec3(0.1);

    const int max_iter = 10;
    #define MIN_ITER max_iter
    [[unroll, max_iterations(max_iter), min_iterations(MIN_ITER)]] for (int i = 0; i != max_iter; ++i);

    LightData mddata[max_iter][5];
    vec3 data = mddata[ReturnArray(mdarray)[0]][3].position;

    LightData result = ReturnLightData(MAX_RETURN_ARRAY_SIZE);

    FragColor = vec4(light + ambient, 1.0);
}
