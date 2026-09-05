#define GL_EXT_float_e2m1 1

// GL_EXT_float_e2m1 (defined by the GL_EXT_ocp_microscaling_types document)

// Types
// floate2m1_t, fe2m1vec2, fe2m1vec3, fe2m1vec4

// bitcastExtract functions
floate2m1_t bitcastExtractfe2m1EXT(uint8_t _Value, uint _Offset);
fe2m1vec2 bitcastExtractfe2m1EXT(u8vec2 _Value, uint _Offset);
fe2m1vec3 bitcastExtractfe2m1EXT(u8vec3 _Value, uint _Offset);
fe2m1vec4 bitcastExtractfe2m1EXT(u8vec4 _Value, uint _Offset);

// pack and unpack functions
fe2m1vec2 unpackFloat2xfe2m1EXT(uint8_t _Value);
fe2m1vec4 unpackFloat4xfe2m1EXT(uint16_t _Value);
vector<floate2m1_t, 8> unpackFloat8xfe2m1EXT(uint32_t _Value);
vector<floate2m1_t, 16> unpackFloat16xfe2m1EXT(u32vec2 _Value);

uint8_t packFloat2xfe2m1EXT(fe2m1vec2 _Value);
uint16_t packFloat4xfe2m1EXT(fe2m1vec4 _Value);
uint32_t packFloat8xfe2m1EXT(vector<floate2m1_t, 8> _Value);
u32vec2 packFloat16xfe2m1EXT(vector<floate2m1_t, 16> _Value);
