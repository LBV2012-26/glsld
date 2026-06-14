#define GL_EXT_ray_tracing_position_fetch 1

#include "GL_EXT_ray_query.glsl"

in vec3 gl_HitTriangleVertexPositionsEXT[3];

void rayQueryGetIntersectionTriangleVertexPositionsEXT(rayQueryEXT _Q, bool _Committed, out vec3 _Positions[3]);
