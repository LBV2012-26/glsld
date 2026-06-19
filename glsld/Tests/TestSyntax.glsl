#version 460
#pragma shader_stage(raygen)

#include "Include.glsl"

#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadEXT vec4 payload;
layout(binding  = 0) uniform accelerationStructureEXT TLAS;

struct HitAttribute {
    vec3 barycentrics;
};

void main() {
    DECLARE_STRUCT_ARRAY(vec3, 10);
    vec3_buffer[0].data;

    vec3 origin = vec3(0.0);
    vec3 direction = vec3(0.0, 0.0, -1.0);
    traceRayEXT(TLAS, gl_RayFlagsOpaqueEXT,
        0xFF, 0, 0,
        0, origin, 0.001, direction, 10000.0, 0);
}
