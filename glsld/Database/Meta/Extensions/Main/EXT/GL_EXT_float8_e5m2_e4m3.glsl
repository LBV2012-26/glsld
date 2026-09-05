#define GL_EXT_float8_e5m2_e4m3 1

// Umbrella document "EXT_float8_e5m2_e4m3" defines:
//   GL_EXT_float_e5m2 and GL_EXT_float_e4m3.

#include "GL_EXT_float_e5m2.glsl"
#include "GL_EXT_float_e4m3.glsl"

// Saturated convert for cooperative matrix types
// valid if the result type is constructible from the value type
void saturatedConvertEXT(out coopmat _Result, coopmat _Value);
