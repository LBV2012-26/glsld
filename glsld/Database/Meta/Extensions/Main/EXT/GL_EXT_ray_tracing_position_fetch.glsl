#define GL_EXT_ray_tracing_position_fetch 1

#include "../../Common/GL_EXT_ray_query_defines.glsl"

#if defined(GL_EXT_ray_tracing)
in vec3 gl_HitTriangleVertexPositionsEXT[3];
#endif

#if defined(GL_EXT_ray_query)
void rayQueryGetIntersectionTriangleVertexPositionsEXT(rayQueryEXT _RayQuery, bool _Committed, out vec3 _Positions[3]);
#endif
