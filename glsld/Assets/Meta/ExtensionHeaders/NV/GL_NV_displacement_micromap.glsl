#define GL_NV_displacement_micromap 1

#define GL_EXT_ray_tracing
#include "../EXT/GL_EXT_RT_Common.glsl"

in vec3 gl_HitMicroTriangleVertexPositionsNV[3];
in vec2 gl_HitMicroTriangleVertexBarycentricsNV[3];
in uint gl_HitKindFrontFacingMicroTriangleNV;
in uint gl_HitKindBackFacingMicroTriangleNV;

vec3 fetchMicroTriangleVertexPositionNV(accelerationStructureEXT _As, int _InstanceId, int _GeometryIndex, int _PrimitiveIndex, ivec2 _Barycentrics);
vec2 fetchMicroTriangleVertexBarycentricNV(accelerationStructureEXT _As, int _InstanceId, int _GeometryIndex, int _PrimitiveIndex, ivec2 _Barycentrics);
