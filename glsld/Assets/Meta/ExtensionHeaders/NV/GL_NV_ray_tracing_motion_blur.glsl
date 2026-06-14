#define GL_NV_ray_tracing_motion_blur 1

#define GL_EXT_ray_tracing
#include "../EXT/GL_EXT_RT_Common.glsl"

in float gl_CurrentRayTimeNV;

void traceRayMotionNV(accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, float _CurrentTime, int _Payload);
