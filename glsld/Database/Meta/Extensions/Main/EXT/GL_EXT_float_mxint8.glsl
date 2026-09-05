#define GL_EXT_float_mxint8 1

// GL_EXT_float_mxint8 (defined by the GL_EXT_ocp_microscaling_types document)

// Types
// floatmxint8_t, fmxint8vec2, fmxint8vec3, fmxint8vec4

// Bitcast functions
int8_t floatmxint8BitsToIntEXT(floatmxint8_t _Value);
i8vec2 floatmxint8BitsToIntEXT(fmxint8vec2 _Value);
i8vec3 floatmxint8BitsToIntEXT(fmxint8vec3 _Value);
i8vec4 floatmxint8BitsToIntEXT(fmxint8vec4 _Value);

uint8_t floatmxint8BitsToUintEXT(floatmxint8_t _Value);
u8vec2 floatmxint8BitsToUintEXT(fmxint8vec2 _Value);
u8vec3 floatmxint8BitsToUintEXT(fmxint8vec3 _Value);
u8vec4 floatmxint8BitsToUintEXT(fmxint8vec4 _Value);

floatmxint8_t intBitsToFloatmxint8EXT(int8_t _Value);
fmxint8vec2 intBitsToFloatmxint8EXT(i8vec2 _Value);
fmxint8vec3 intBitsToFloatmxint8EXT(i8vec3 _Value);
fmxint8vec4 intBitsToFloatmxint8EXT(i8vec4 _Value);

floatmxint8_t uintBitsToFloatmxint8EXT(uint8_t _Value);
fmxint8vec2 uintBitsToFloatmxint8EXT(u8vec2 _Value);
fmxint8vec3 uintBitsToFloatmxint8EXT(u8vec3 _Value);
fmxint8vec4 uintBitsToFloatmxint8EXT(u8vec4 _Value);
