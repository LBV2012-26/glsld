#define GL_EXT_float_e2m3 1

// GL_EXT_float_e2m3 (defined by the GL_EXT_ocp_microscaling_types document)

// Types
// floate2m3_t, fe2m3vec2, fe2m3vec3, fe2m3vec4

// bitcastExtract functions
floate2m3_t bitcastExtractfe2m3EXT(uint8_t _Value, uint _Offset);
fe2m3vec2 bitcastExtractfe2m3EXT(u8vec2 _Value, uint _Offset);
fe2m3vec3 bitcastExtractfe2m3EXT(u8vec3 _Value, uint _Offset);
fe2m3vec4 bitcastExtractfe2m3EXT(u8vec4 _Value, uint _Offset);

// pack and unpack functions
fe2m3vec4 unpackFloat4xfe2m3EXT(u8vec3 _Value);
vector<floate2m3_t, 8> unpackFloat8xfe2m3EXT(u16vec3 _Value);
vector<floate2m3_t, 16> unpackFloat16xfe2m3EXT(u32vec3 _Value);

u8vec3 packFloat4xfe2m3EXT(fe2m3vec4 _Value);
u16vec3 packFloat8xfe2m3EXT(vector<floate2m3_t, 8> _Value);
u32vec3 packFloat16xfe2m3EXT(vector<floate2m3_t, 16> _Value);
