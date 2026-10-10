#define GL_NV_cluster_acceleration_structure 1

#include "../../Common/GL_EXT_ray_query_defines.glsl"
#include "../../Common/GL_NV_shader_invocation_reorder_defines.glsl"

#if defined(GL_EXT_ray_tracing)
in int gl_ClusterIDNV;
#endif
const int gl_ClusterIDNoneNV = -1;

#if defined(GL_EXT_ray_query)
int rayQueryGetIntersectionClusterIdNV(rayQueryEXT _RayQuery, bool _Committed);
#endif
#if defined(GL_NV_shader_invocation_reorder)
int hitObjectGetClusterIdNV(hitObjectNV _HitObject);
#endif
