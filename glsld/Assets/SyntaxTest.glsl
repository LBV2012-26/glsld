#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require

layout(location = 0) in  vec3 InPosition;
layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform sampler my_sampler;

struct LightData {
    vec3 position;
};

layout(std430, set = 0, binding = 0) buffer LightBuffer {
    LightData lights[];
};

#define MAX_RETURN_ARRAY_SIZE 5
#define MAX_TEST_ARRAY_1_SIZE 20

int[MAX_RETURN_ARRAY_SIZE] ReturnArray(int mdarray[25][MAX_TEST_ARRAY_1_SIZE]) {
    int array[5];
    return array;
}

LightData ReturnLightData() {
    LightData data;
    return data;
}

void TestOverloadFunction(bool) {}
void TestOverloadFunction(int) {}
void TestOverloadFunction(uint) {}
void TestOverloadFunction(float) {}
void TestOverloadFunction(double) {}
void TestOverloadFunction(int16_t, int32_t, int64_t, float32_t) {}
void TestOverloadFunction(uint16_t, uint32_t, uint64_t, float16_t) {}

void main() {
    mat4 m;
    vec4 v = m[0].xyzw;

    vec4 v1;
    float f = v1[0];

    bvec3 a;

    int mdarray[25][MAX_TEST_ARRAY_1_SIZE];
    int array[MAX_RETURN_ARRAY_SIZE] = ReturnArray(mdarray);
    vec3 light = normalize(lights[0].position - InPosition);

    LightData mddata[10][5];
    vec3 data = mddata[2][3].position;
    
    int8_t int8arg;
    uint8_t uint8arg;
    int16_t int16arg;
    uint16_t uint16arg;
    int32_t int32arg;
    uint32_t uint32arg;
    int64_t int64arg;
    uint64_t uint64arg;
    float16_t float16arg;
    float32_t float32arg;
    float64_t float64arg;

    vec4 vec = vec4(1.0);
    float varg = vec.xxyy.x;
    TestOverloadFunction(varg);

    TestOverloadFunction(true);
    TestOverloadFunction(1);
    TestOverloadFunction(1u);
    TestOverloadFunction(1.0);
    TestOverloadFunction(1.0f);
    TestOverloadFunction(1.0lf);

    TestOverloadFunction(int16arg, int32arg, int64arg, float32arg);
    TestOverloadFunction(int16arg, uint32arg, uint64arg, float16arg);

    TestOverloadFunction(int8arg);
    TestOverloadFunction(uint8arg);

    TestOverloadFunction(int16arg, int8arg, int8arg, float16arg);
    TestOverloadFunction(uint16arg, uint8arg, uint8arg, float32arg);

    mat4 matrix = mat4(1.0);
    vec4 vector = matrix[0].wzyx;

    LightData result = ReturnLightData();

    FragColor = vec4(light, 1.0);
}
