#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference : require

layout(location = 0) in  vec3 InPosition;
layout(location = 0) out vec4 FragColor;

layout(set = 1, binding = 0) uniform sampler   my_sampler;
layout(set = 2, binding = 0) uniform texture2D my_texture;

layout(std140, set = 0, binding = 1) uniform MyUniformBuffer { // without set, auto fill set to 0
    mat4 my_matrix;
} ubo;

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
#define MAX_ARRAY_SIZE_1      20

#define MACRO_FUNC(x) (x * x)

int[MAX_RETURN_ARRAY_SIZE] ReturnArray(int mdarray[25][MAX_ARRAY_SIZE_1]) {
    int array[MAX_RETURN_ARRAY_SIZE];
    return array;
}

LightData ReturnLightData() {
    LightData data;
    return data;
}

void Func(const in float input_arg, out float output_arg, inout float param) {
    output_arg = input_arg + MACRO_FUNC(param);

    texture(sampler(my_sampler, my_texture), vec2(0.5));
    texture2D(my_texture, vec2(0.5)); // test constructor
}

void main() {
    LightDataBuffer data_buffer = LightDataBuffer(push_constants.push_constant_value);
    mat4 my_matrix = ubo.my_matrix;
    int mdarray[25][MAX_ARRAY_SIZE_1];
    int array[MAX_RETURN_ARRAY_SIZE] = ReturnArray(mdarray);

    vec3 light = normalize(lights[0].position - InPosition), ambient = vec3(0.1);

    LightData mddata[10][5];
    vec3 data = mddata[2][3].position;

    LightData result = ReturnLightData();

    FragColor = vec4(light + ambient, 1.0);
}
