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

#define RETURN_LIGNT_BUFFER ReturnLightBuffer()
#define vec3 vec4

void main() {
    RETURN_LIGNT_BUFFER[0][0].light[0].strength;
    vec3 color = vec4(1.0);
}
