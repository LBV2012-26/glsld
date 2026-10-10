#define GL_NV_linear_swept_spheres 1

#include "../../Common/GL_EXT_ray_query_defines.glsl"
#include "../../Common/GL_NV_shader_invocation_reorder_defines.glsl"

#if defined(GL_EXT_ray_tracing)
in vec3  gl_HitSpherePositionNV;
in float gl_HitSphereRadiusNV;
in vec3  gl_HitLSSPositionsNV[2];
in float gl_HitLSSRadiiNV[2];
in bool  gl_HitIsSphereNV;
in bool  gl_HitIsLSSNV;
#endif

#if defined(GL_EXT_ray_flags_primitive_culling)
const uint gl_RayFlagsSkipBuiltinPrimitivesNV = 256U;
#endif

#if defined(GL_EXT_ray_query)
vec3 rayQueryGetIntersectionSpherePositionNV(rayQueryEXT _RayQuery, bool _Committed);
float rayQueryGetIntersectionSphereRadiusNV(rayQueryEXT _RayQuery, bool _Committed);
void rayQueryGetIntersectionLSSPositionsNV(rayQueryEXT _RayQuery, bool _Committed, out vec3 _Positions[2]);
void rayQueryGetIntersectionLSSRadiiNV(rayQueryEXT _RayQuery, bool _Committed, out float _Radii[2]);
float rayQueryGetIntersectionLSSHitValueNV(rayQueryEXT _RayQuery, bool _Committed);
bool rayQueryIsSphereHitNV(rayQueryEXT _RayQuery, bool _Committed);
bool rayQueryIsLSSHitNV(rayQueryEXT _RayQuery, bool _Committed);
#endif

#if defined(GL_NV_shader_invocation_reorder)
vec3 hitObjectGetSpherePositionNV(hitObjectNV _HitObject);
float hitObjectGetSphereRadiusNV(hitObjectNV _HitObject);
void hitObjectGetLSSPositionsNV(hitObjectNV _HitObject, out vec3 _Positions[2]);
vec3 hitObjectGetLSSRadiiNV(hitObjectNV _HitObject, out float _Radii[2]);
bool hitObjectIsSphereHitNV(hitObjectNV _HitObject);
bool hitObjectIsLSSHitNV(hitObjectNV _HitObject);
#endif
