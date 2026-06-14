#include "../EXT/GL_EXT_ray_query.glsl"

in vec3  gl_HitSpherePositionNV;
in float gl_HitSphereRadiusNV;
in vec3  gl_HitLSSPositionsNV[2];
in float gl_HitLSSRadiiNV[2];
in bool  gl_HitIsSphereNV;
in bool  gl_HitIsLSSNV;

const uint gl_RayFlagsSkipBuiltinPrimitivesNV = 256U;
vec3 rayQueryGetIntersectionSpherePositionNV(rayQueryEXT _Q, bool _Committed);
float rayQueryGetIntersectionSphereRadiusNV(rayQueryEXT _Q, bool _Committed);
void rayQueryGetIntersectionLSSPositionsNV(rayQueryEXT _Q, bool _Committed, out vec3 _Positions[2]);
void rayQueryGetIntersectionLSSRadiiNV(rayQueryEXT _Q, bool _Committed, out float _Radii[2]);
float rayQueryGetIntersectionLSSHitValueNV(rayQueryEXT _Rq, bool _Committed);
bool rayQueryIsSphereHitNV(rayQueryEXT _Rq, bool _Committed);
bool rayQueryIsLSSHitNV(rayQueryEXT _Rq, bool _Committed);
vec3 hitObjectGetSpherePositionNV(hitObjectNV _HitObject);
float hitObjectGetSphereRadiusNV(hitObjectNV _HitObject);
void hitObjectGetLSSPositionsNV(hitObjectNV _HitObject, out vec3 _Positions[2]);
vec3 hitObjectGetLSSRadiiNV(hitObjectNV _HitObject, out float _Radii[2]);
bool hitObjectIsSphereHitNV(hitObjectNV _HitObject);
bool hitObjectIsLSSHitNV(hitObjectNV _HitObject);
