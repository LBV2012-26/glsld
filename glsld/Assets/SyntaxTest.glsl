#version 460 core

layout(location = 0) in  vec3 InPosition;
layout(location = 0) out vec4 FragColor;

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

LightData ReturnLightData(float) {
    LightData data;
    return data;
}

LightData ReturnLightData(uint) {
    LightData data;
    return data;
}

void main() {
    int mdarray[25][MAX_TEST_ARRAY_1_SIZE];
    int array[MAX_RETURN_ARRAY_SIZE] = ReturnArray(mdarray);
    vec3 light = normalize(lights[0].position - InPosition);
    
    int arg = 0;
    LightData result1 = ReturnLightData();
    LightData result2 = ReturnLightData(arg);

    FragColor = vec4(light, 1.0);
}
