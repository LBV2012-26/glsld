#version 460 core

layout(std140, set = 0, binding = 1) uniform MyUniformBuffer { // without set, auto fill set to 0
    mat4 my_matrix;
} ubo;

void main() {
}
