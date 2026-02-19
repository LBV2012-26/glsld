#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require

void TestOverloadFunction(float16_t floatarg) {}
void TestOverloadFunction(double doublearg) {}

void main() {
    TestOverloadFunction(1.0hf);
    TestOverloadFunction(1.0f);
}
