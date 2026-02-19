#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require

void TestOverloadFunction();
void TestOverloadFunction(bool boolarg) {}
void TestOverloadFunction(int intarg) {}
void TestOverloadFunction(uint uintarg) {}
void TestOverloadFunction(half halfarg) {}
void TestOverloadFunction(float floatarg) {}
void TestOverloadFunction(double doublearg) {}

void TestOverloadFunction(int16_t int16arg, int32_t int32arg, int64_t int64arg, float32_t float32arg) {}
void TestOverloadFunction(uint16_t uint16arg, uint32_t uint32arg, uint64_t uint64arg, float16_t float16arg) {}

void main() {
    int8_t    int8arg;
    int16_t   int16arg;
    int32_t   int32arg;
    int64_t   int64arg;

    uint8_t   uint8arg;
    uint16_t  uint16arg;
    uint32_t  uint32arg;
    uint64_t  uint64arg;

    float16_t float16arg;
    float32_t float32arg;
    float64_t float64arg;

    // test literal overloads
    TestOverloadFunction(true);
    TestOverloadFunction(1);
    TestOverloadFunction(1u);
    TestOverloadFunction(1.0);
    TestOverloadFunction(1.0hf);
    TestOverloadFunction(1.0f);
    TestOverloadFunction(1.0lf);

    // test non-literal overloads
    TestOverloadFunction(int16arg, int32arg, int64arg, float32arg);
    TestOverloadFunction(int16arg, uint32arg, uint64arg, float16arg);

    // test bits upgrade
    TestOverloadFunction(int8arg);
    TestOverloadFunction(uint8arg);

    // test ambiguous overloads
    TestOverloadFunction(int16arg, int8arg, int8arg, float16arg);
    TestOverloadFunction(uint16arg, uint8arg, uint8arg, float32arg);

    // test swizzle overloads
    mat4 matrix;
    vec4 vector = matrix[0].xyzw;
    float component1 = vector.x;
    float component2 = matrix[1].xyzw.x;

    TestOverloadFunction(component1);
    TestOverloadFunction(component2);
}
