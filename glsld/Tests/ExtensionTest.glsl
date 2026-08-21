#version 460
#pragma shader_stage(raygen)

#define MACRO_VALUE 1
#if defined(SHARED_MACRO) && UNIQUE_MACRO && !defined(INVERSE_MACRO)
#include "Include.glsl"
#endif

#extension GL_EXT_ray_tracing : require

#ifdef GL_RAY_GENERATION_SHADER_EXT
const int kArraySize = 10;
#endif

layout(location = 0) rayPayloadEXT vec4 payload;
layout(binding  = 0) uniform accelerationStructureEXT tlas;

struct HitAttribute {
    vec3 barycentrics;
};

void main() {
    DECLARE_STRUCT_ARRAY(vec3, kArraySize);
    vec3_buffer[kIncludeValue].data;

    vec3 origin    = vec3(0.0);
    vec3 direction = vec3(0.0, 0.0, -1.0);
    traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, origin, 0.001,
                direction, 10000.0, 0);
}
