#version 460 core

layout(location = 0) in  vec3 InPosition;
layout(location = 0) out vec4 FragColor;

void MyFunc();

void main() {
    FragColor = vec4(InPosition, 1.0);
}
