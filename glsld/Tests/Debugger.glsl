#version 460 core
#extension GL_EXT_subgroup_uniform_control_flow : require
#extension GL_EXT_maximal_reconvergence : require
#extension GL_EXT_control_flow_attributes : require
#extension GL_EXT_control_flow_attributes2 : require

struct LightData {
    vec3 strength;
};

struct LightBuffer {
    LightData light[6];
};

LightBuffer[10][2] ReturnLightBuffer() {
    LightBuffer[10][2] data;
    return data;
}

#define RETURN_LIGNT_BUFFER ReturnLightBuffer()
#define vec3 vec4

void main() [[subgroup_uniform_control_flow]] {
    RETURN_LIGNT_BUFFER[0][0].light[0].strength;
    vec3 color = vec4(1.0);

    const int max_iter = 10;
    #define MIN_ITER 1
    [[unroll, max_iterations(max_iter), min_iterations(MIN_ITER)]]
    for (int i = 0; i != 10; ++i);
}
