#version 460 core
#pragma shader_stage(compute)

#include "Include.glsl"

void main() {
    float value = glsldBackgroundIndexProbe(1.0);
}
