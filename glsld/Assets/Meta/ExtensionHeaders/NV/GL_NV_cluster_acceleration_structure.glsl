#include "../EXT/GL_EXT_ray_query.glsl"

in    int gl_ClusterIDNV;
const int gl_ClusterIDNoneNV = -1;

int rayQueryGetIntersectionClusterIdNV(rayQueryEXT _Q, bool _Committed);
int hitObjectGetClusterIdNV(hitObjectNV _HitObject);
