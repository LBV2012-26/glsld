#define GL_EXT_float_e3m2 1

#include "../../Common/GL_EXT_long_vector_defines.glsl"

// GL_EXT_float_e3m2 (defined by the GL_EXT_ocp_microscaling_types document)

// Types
// floate3m2_t, fe3m2vec2, fe3m2vec3, fe3m2vec4

// bitcastExtract functions
floate3m2_t bitcastExtractfe3m2EXT(uint8_t _Value, uint _Offset);
fe3m2vec2 bitcastExtractfe3m2EXT(u8vec2 _Value, uint _Offset);
fe3m2vec3 bitcastExtractfe3m2EXT(u8vec3 _Value, uint _Offset);
fe3m2vec4 bitcastExtractfe3m2EXT(u8vec4 _Value, uint _Offset);

// pack and unpack functions
fe3m2vec4 unpackFloat4xfe3m2EXT(u8vec3 _Value);
vector<floate3m2_t, 8> unpackFloat8xfe3m2EXT(u16vec3 _Value);
vector<floate3m2_t, 16> unpackFloat16xfe3m2EXT(uvec3 _Value);

u8vec3 packFloat4xfe3m2EXT(fe3m2vec4 _Value);

#if defined(GL_EXT_long_vector)
u16vec3 packFloat8xfe3m2EXT(vector<floate3m2_t, 8> _Value);
uvec3 packFloat16xfe3m2EXT(vector<floate3m2_t, 16> _Value);
#endif
