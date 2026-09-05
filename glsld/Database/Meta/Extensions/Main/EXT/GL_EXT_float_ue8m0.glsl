#define GL_EXT_float_ue8m0 1

// GL_EXT_float_ue8m0 (defined by the GL_EXT_ocp_microscaling_types document)

// Types
// floatue8m0_t, fue8m0vec2, fue8m0vec3, fue8m0vec4

// Bitcast functions
int8_t floatue8m0BitsToIntEXT(floatue8m0_t _Value);
i8vec2 floatue8m0BitsToIntEXT(fue8m0vec2 _Value);
i8vec3 floatue8m0BitsToIntEXT(fue8m0vec3 _Value);
i8vec4 floatue8m0BitsToIntEXT(fue8m0vec4 _Value);

uint8_t floatue8m0BitsToUintEXT(floatue8m0_t _Value);
u8vec2 floatue8m0BitsToUintEXT(fue8m0vec2 _Value);
u8vec3 floatue8m0BitsToUintEXT(fue8m0vec3 _Value);
u8vec4 floatue8m0BitsToUintEXT(fue8m0vec4 _Value);

floatue8m0_t intBitsToFloatue8m0EXT(int8_t _Value);
fue8m0vec2 intBitsToFloatue8m0EXT(i8vec2 _Value);
fue8m0vec3 intBitsToFloatue8m0EXT(i8vec3 _Value);
fue8m0vec4 intBitsToFloatue8m0EXT(i8vec4 _Value);

floatue8m0_t uintBitsToFloatue8m0EXT(uint8_t _Value);
fue8m0vec2 uintBitsToFloatue8m0EXT(u8vec2 _Value);
fue8m0vec3 uintBitsToFloatue8m0EXT(u8vec3 _Value);
fue8m0vec4 uintBitsToFloatue8m0EXT(u8vec4 _Value);
