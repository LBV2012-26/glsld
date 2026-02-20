#version 460 core

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

void main() {
    LightBuffer l;
    ReturnLightBuffer()[0][1].light[0].strength;
}
