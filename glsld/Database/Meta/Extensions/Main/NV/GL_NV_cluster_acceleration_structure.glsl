#define GL_NV_cluster_acceleration_structure 1

#include "../EXT/GL_EXT_ray_query.glsl"

in    int gl_ClusterIDNV;
const int gl_ClusterIDNoneNV = -1;

int rayQueryGetIntersectionClusterIdNV(rayQueryEXT _RayQuery, bool _Committed);
int hitObjectGetClusterIdNV(hitObjectNV _HitObject);
