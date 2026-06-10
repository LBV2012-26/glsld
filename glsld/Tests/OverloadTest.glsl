#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require

//#include "../Assets/Meta/BuiltinFunctions.glsl"

void TestOverloadFunction(int16_t int16arg, int32_t int32arg, int64_t int64arg, float32_t float32arg) {}
void TestOverloadFunction(uint16_t uint16arg, uint32_t uint32arg, uint64_t uint64arg, float16_t float16arg) {}

void TestOverloadFunction(); // Test declare and implement toggle
void TestOverloadFunction(bool boolarg) {}

void TestOverloadFunction(vec2 vec2arg) {}
void TestOverloadFunction(vec3 vec3arg) {}
void TestOverloadFunction(vec4 vec4arg) {}

void TestOverloadFunction(dvec2 dvec2arg) {}
void TestOverloadFunction(dvec3 dvec3arg) {}
void TestOverloadFunction(dvec4 dvec4arg) {}

void TestOverloadFunction(ivec2 ivec2arg) {}
void TestOverloadFunction(ivec3 ivec3arg) {}
void TestOverloadFunction(ivec4 ivec4arg) {}

void TestOverloadFunction(mat2 mat2arg) {}
void TestOverloadFunction(mat3 mat3arg) {}
void TestOverloadFunction(mat4 mat4arg) {}

void TestOverloadFunction(mat2x3 mat2x3arg) {}
void TestOverloadFunction(mat2x4 mat2x4arg) {}
void TestOverloadFunction(mat3x2 mat3x2arg) {}
void TestOverloadFunction(mat3x4 mat3x4arg) {}
void TestOverloadFunction(mat4x2 mat4x2arg) {}
void TestOverloadFunction(mat4x3 mat4x3arg) {}

void TestOverloadFunction(int8_t int8arg) {}
void TestOverloadFunction(uint8_t uint8arg) {}
void TestOverloadFunction(int16_t int16arg) {}
void TestOverloadFunction(uint16_t uint16arg) {}
void TestOverloadFunction(int32_t int32arg) {}
void TestOverloadFunction(uint32_t uint32arg) {}
void TestOverloadFunction(int64_t int64arg) {}
void TestOverloadFunction(uint64_t uint64arg) {}

void TestOverloadFunction(float16_t float16arg) {}
void TestOverloadFunction(float32_t float32arg) {}
void TestOverloadFunction(float64_t float64arg) {}

void TestOverloadFunction(int intArray1D_2[2]) {}
void TestOverloadFunction(int intArray1D_3[3]) {}
void TestOverloadFunction(int intArray2D_2x3[2][3]) {}
void TestOverloadFunction(uint uintArray1D_2[2]) {}
void TestOverloadFunction(uint uintArray1D_4[4]) {}
void TestOverloadFunction(uint16_t uint16Array1D_3[3]) {}
void TestOverloadFunction(float floatArray1D_5[5]) {}
void TestOverloadFunction(float floatArray2D_2x3[2][3]) {}
void TestOverloadFunction(float floatArray2D_3x4[3][4]) {}
void TestOverloadFunction(float floatArray3D_2x2x3[2][2][3]) {}
void TestOverloadFunction(double doubleArray1D_4[4]) {}
void TestOverloadFunction(vec2 vec2Array1D_3[3]) {}
void TestOverloadFunction(vec2 vec2Array1D_4[4]) {}
void TestOverloadFunction(vec2 vec2Array3D_2x2x2[2][2][2]) {}
void TestOverloadFunction(dvec2 dvec2Array1D_2[2]) {}
void TestOverloadFunction(ivec3 ivec3Array1D_2[2]) {}
void TestOverloadFunction(vec3 vec3Array1D_2[2]) {}
void TestOverloadFunction(vec3 vec3Array2D_2x2[2][2]) {}
void TestOverloadFunction(vec4 vec4Array1D_2[2]) {}
void TestOverloadFunction(mat2 mat2Array1D_2[2]) {}
void TestOverloadFunction(mat2 mat2Array2D_2x2[2][2]) {}
void TestOverloadFunction(mat3 mat3Array1D_2[2]) {}
void TestOverloadFunction(mat4 mat4Array1D_2[2]) {}

struct InnerData {
    float float_field;
    int   int_field;
    vec3  vec3_field;
    mat2  mat2_field[2];
    int   intArray2D_2x3[2][3];
};

struct MiddleData {
    InnerData inner;
    vec4      v4Array[3];
    mat4      mat4_field;
};

struct OuterData {
    MiddleData middle[2];
    dvec2      dvec2_field;
};

void TestOverloadFunction(InnerData inner_data) {}
void TestOverloadFunction(MiddleData middle_data) {}
void TestOverloadFunction(OuterData outer_data) {}

float GetFloat() { return 1.0f; }
double GetDouble() { return 1.0lf; }
int GetInt() { return 1; }
vec2 GetVec2() { return vec2(1.0f, 2.0f); }
vec3 GetVec3() { return vec3(1.0f, 2.0f, 3.0f); }
vec4 GetVec4() { return vec4(1.0f, 2.0f, 3.0f, 4.0f); }
dvec2 GetDVec2() { return dvec2(1.0lf, 2.0lf); }
dvec3 GetDVec3() { return dvec3(1.0lf, 2.0lf, 3.0lf); }
dvec4 GetDVec4() { return dvec4(1.0lf, 2.0lf, 3.0lf, 4.0lf); }
ivec2 GetIVec2() { return ivec2(1, 2); }
ivec3 GetIVec3() { return ivec3(1, 2, 3); }
ivec4 GetIVec4() { return ivec4(1, 2, 3, 4); }
mat2 GetMat2() { return mat2(1.0f); }
mat3 GetMat3() { return mat3(1.0f); }
mat4 GetMat4() { return mat4(1.0f); }
mat2x3 GetMat2x3() { return mat2x3(1.0f); }
mat3x2 GetMat3x2() { return mat3x2(1.0f); }
mat2x4 GetMat2x4() { return mat2x4(1.0f); }
mat4x2 GetMat4x2() { return mat4x2(1.0f); }
mat3x4 GetMat3x4() { return mat3x4(1.0f); }
mat4x3 GetMat4x3() { return mat4x3(1.0f); }

#define MySpirvType0 spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [5353], set = "GLSL.std.450", id = 5341)
#define MySpirvType1 spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [1145], set = "GLSL.std.450", id = 4514)
#define MySpirvType2 spirv_type(set = "GLSL.std.450", capabilities = [1145], extensions = ["SPV_KHR_ray_tracing"], id = 4514)
#define MySpirvType3 spirv_type(id = 32, spirv_id 64)
#define MySpirvType4 spirv_type(id = 32, spirv_id 32)
#define MySpirvType5 spirv_type(id = 21, 64, 1)
#define MySpirvType6 spirv_type(id = 32, 32, 1)
#define MySpirvType7 spirv_type(id = 32, 64)
#define MySpirvType8 spirv_type(id = 32, 32)

void TestOverloadFunction(MySpirvType0 param0);
void TestOverloadFunction(MySpirvType1 param1);
void TestOverloadFunction(MySpirvType3 param3);
void TestOverloadFunction(MySpirvType4 param4);
void TestOverloadFunction(MySpirvType5 param5);
void TestOverloadFunction(MySpirvType6 param6);
void TestOverloadFunction(MySpirvType7 param7);
void TestOverloadFunction(MySpirvType8 param8);
void TestOverloadFunction(MySpirvType0 param0, MySpirvType1 param1);
void TestOverloadFunction(MySpirvType1 param1, MySpirvType0 param0);

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

    // int8 -> int
    TestOverloadFunction(int8arg + int8arg);           // int8 + int8 -> int
    TestOverloadFunction(int8arg * int8arg);           // int8 * int8 -> int
    TestOverloadFunction(int8arg - int8arg);           // int8 - int8 -> int
    TestOverloadFunction(int8arg / int8arg);           // int8 / int8 -> int

    // uint8 -> uint
    TestOverloadFunction(uint8arg + uint8arg);         // uint8 + uint8 -> uint
    TestOverloadFunction(uint8arg * uint8arg);         // uint8 * uint8 -> uint
    TestOverloadFunction(uint8arg - uint8arg);         // uint8 - uint8 -> uint
    TestOverloadFunction(uint8arg / uint8arg);         // uint8 / uint8 -> uint

    // int16 -> int
    TestOverloadFunction(int16arg + int16arg);         // int16 + int16 -> int
    TestOverloadFunction(int16arg * int16arg);         // int16 * int16 -> int
    TestOverloadFunction(int16arg - int16arg);         // int16 - int16 -> int
    TestOverloadFunction(int16arg / int16arg);         // int16 / int16 -> int

    // uint16 -> uint
    TestOverloadFunction(uint16arg + uint16arg);       // uint16 + uint16 -> uint
    TestOverloadFunction(uint16arg * uint16arg);       // uint16 * uint16 -> uint
    TestOverloadFunction(uint16arg - uint16arg);       // uint16 - uint16 -> uint
    TestOverloadFunction(uint16arg / uint16arg);       // uint16 / uint16 -> uint

    // int32 -> int
    TestOverloadFunction(int32arg + int32arg);         // int32 + int32 -> int
    TestOverloadFunction(int32arg * int32arg);         // int32 * int32 -> int

    // uint32 -> uint
    TestOverloadFunction(uint32arg + uint32arg);       // uint32 + uint32 -> uint
    TestOverloadFunction(uint32arg * uint32arg);       // uint32 * uint32 -> uint

    // int64 -> int64
    TestOverloadFunction(int64arg + int64arg);         // int64 + int64 -> int64
    TestOverloadFunction(int64arg * int64arg);         // int64 * int64 -> int64

    // uint64 -> uint64
    TestOverloadFunction(uint64arg + uint64arg);       // uint64 + uint64 -> uint64
    TestOverloadFunction(uint64arg * uint64arg);       // uint64 * uint64 -> uint64

    // float16 -> float
    TestOverloadFunction(float16arg + float16arg);     // float16 + float16 -> float
    TestOverloadFunction(float16arg * float16arg);     // float16 * float16 -> float
    TestOverloadFunction(float16arg - float16arg);     // float16 - float16 -> float
    TestOverloadFunction(float16arg / float16arg);     // float16 / float16 -> float

    // float32 -> float
    TestOverloadFunction(float32arg + float32arg);     // float32 + float32 -> float
    TestOverloadFunction(float32arg * float32arg);     // float32 * float32 -> float

    // float64 -> double
    TestOverloadFunction(float64arg + float64arg);     // float64 + float64 -> double
    TestOverloadFunction(float64arg * float64arg);     // float64 * float64 -> double

    // mixed scalar type promotion
    TestOverloadFunction(int8arg + int16arg);          // int8 + int16 -> int
    TestOverloadFunction(int8arg + int32arg);          // int8 + int32 -> int
    TestOverloadFunction(int16arg + int32arg);         // int16 + int32 -> int
    TestOverloadFunction(uint8arg + uint16arg);        // uint8 + uint16 -> uint
    TestOverloadFunction(uint8arg + uint32arg);        // uint8 + uint32 -> uint
    TestOverloadFunction(uint16arg + uint32arg);       // uint16 + uint32 -> uint
    TestOverloadFunction(float16arg + float32arg);     // float16 + float32 -> float
    TestOverloadFunction(float16arg + float64arg);     // float16 + float64 -> double
    TestOverloadFunction(float32arg + float64arg);     // float32 + float64 -> double

    // vector type promotion
    ivec2 iv2a = ivec2(1, 2);
    ivec3 iv3a = ivec3(1, 2, 3);
    ivec4 iv4a = ivec4(1, 2, 3, 4);

    ivec2 iv2b = ivec2(3, 4);
    ivec3 iv3b = ivec3(4, 5, 6);
    ivec4 iv4b = ivec4(5, 6, 7, 8);

    TestOverloadFunction(iv2a + iv2b);                 // ivec2 + ivec2 -> ivec2
    TestOverloadFunction(iv2a * iv2b);                 // ivec2 * ivec2 -> ivec2
    TestOverloadFunction(iv3a + iv3b);                 // ivec3 + ivec3 -> ivec3
    TestOverloadFunction(iv3a * iv3b);                 // ivec3 * ivec3 -> ivec3
    TestOverloadFunction(iv4a + iv4b);                 // ivec4 + ivec4 -> ivec4
    TestOverloadFunction(iv4a * iv4b);                 // ivec4 * ivec4 -> ivec4

    // float vector type promotion
    vec2 v2a = vec2(1.0f, 2.0f);
    vec3 v3a = vec3(1.0f, 2.0f, 3.0f);
    vec4 v4a = vec4(1.0f, 2.0f, 3.0f, 4.0f);

    vec2 v2b = vec2(3.0f, 4.0f);
    vec3 v3b = vec3(4.0f, 5.0f, 6.0f);
    vec4 v4b = vec4(5.0f, 6.0f, 7.0f, 8.0f);

    TestOverloadFunction(v2a + v2b);                   // vec2 + vec2 -> vec2
    TestOverloadFunction(v2a * v2b);                   // vec2 * vec2 -> vec2
    TestOverloadFunction(v3a + v3b);                   // vec3 + vec3 -> vec3
    TestOverloadFunction(v3a * v3b);                   // vec3 * vec3 -> vec3
    TestOverloadFunction(v4a + v4b);                   // vec4 + vec4 -> vec4
    TestOverloadFunction(v4a * v4b);                   // vec4 * vec4 -> vec4

    // double vector type promotion
    dvec2 dv2a = dvec2(1.0lf, 2.0lf);
    dvec3 dv3a = dvec3(1.0lf, 2.0lf, 3.0lf);
    dvec4 dv4a = dvec4(1.0lf, 2.0lf, 3.0lf, 4.0lf);

    dvec2 dv2b = dvec2(3.0lf, 4.0lf);
    dvec3 dv3b = dvec3(4.0lf, 5.0lf, 6.0lf);
    dvec4 dv4b = dvec4(5.0lf, 6.0lf, 7.0lf, 8.0lf);

    TestOverloadFunction(dv2a + dv2b);                 // dvec2 + dvec2 -> dvec2
    TestOverloadFunction(dv2a * dv2b);                 // dvec2 * dvec2 -> dvec2
    TestOverloadFunction(dv3a + dv3b);                 // dvec3 + dvec3 -> dvec3
    TestOverloadFunction(dv3a * dv3b);                 // dvec3 * dvec3 -> dvec3
    TestOverloadFunction(dv4a + dv4b);                 // dvec4 + dvec4 -> dvec4
    TestOverloadFunction(dv4a * dv4b);                 // dvec4 * dvec4 -> dvec4

    // vector component scalar type promotion
    TestOverloadFunction((iv2a + iv2b).x);             // ivec2 + ivec2 -> ivec2, .x -> int
    TestOverloadFunction((v2a + v2b).x);               // vec2 + vec2 -> vec2, .x -> float
    TestOverloadFunction((dv2a + dv2b).x);             // dvec2 + dvec2 -> dvec2, .x -> double

    // test expression overloads
    TestOverloadFunction(1.0f + 1 * 1.0lf);
    TestOverloadFunction(-1.0f);
    TestOverloadFunction(float(1 + 2) * 3);

    // test scalar expression overloads
    float  s1 = 1.0f;
    double s2 = 1.0lf;
    int    s3 = 1;

    TestOverloadFunction(s1 + s1);          // float + float -> float
    TestOverloadFunction(s1 - s1);          // float - float -> float
    TestOverloadFunction(s1 * s1);          // float * float -> float
    TestOverloadFunction(s1 / s1);          // float / float -> float
    TestOverloadFunction(s2 + s2);          // double + double -> double
    TestOverloadFunction(s2 - s2);          // double - double -> double
    TestOverloadFunction(s2 * s2);          // double * double -> double
    TestOverloadFunction(s2 / s2);          // double / double -> double
    TestOverloadFunction(s3 + s3);          // int + int -> int
    TestOverloadFunction(s3 - s3);          // int - int -> int
    TestOverloadFunction(s3 * s3);          // int * int -> int
    TestOverloadFunction(s3 / s3);          // int / int -> int

    // test vec2 expression overloads
    TestOverloadFunction(v2a + v2b);        // vec2 + vec2 -> vec2
    TestOverloadFunction(v2a - v2b);        // vec2 - vec2 -> vec2
    TestOverloadFunction(v2a * v2b);        // vec2 * vec2 (component-wise) -> vec2
    TestOverloadFunction(v2a / v2b);        // vec2 / vec2 (component-wise) -> vec2
    TestOverloadFunction(v2a * s1);         // vec2 * scalar -> vec2
    TestOverloadFunction(s1 * v2a);         // scalar * vec2 -> vec2
    TestOverloadFunction(v2a / s1);         // vec2 / scalar -> vec2
    TestOverloadFunction(v2a + s1);         // vec2 + scalar -> vec2
    TestOverloadFunction(v2a - s1);         // vec2 - scalar -> vec2

    // test vec3 expression overloads
    TestOverloadFunction(v3a + v3b);
    TestOverloadFunction(v3a - v3b);
    TestOverloadFunction(v3a * v3b);
    TestOverloadFunction(v3a / v3b);
    TestOverloadFunction(v3a * s1);
    TestOverloadFunction(s1 * v3a);
    TestOverloadFunction(v3a / s1);
    TestOverloadFunction(v3a + s1);
    TestOverloadFunction(v3a - s1);

    // test vec4 expression overloads
    TestOverloadFunction(v4a + v4b);
    TestOverloadFunction(v4a - v4b);
    TestOverloadFunction(v4a * v4b);
    TestOverloadFunction(v4a / v4b);
    TestOverloadFunction(v4a * s1);
    TestOverloadFunction(s1 * v4a);
    TestOverloadFunction(v4a / s1);
    TestOverloadFunction(v4a + s1);
    TestOverloadFunction(v4a - s1);

    // test dvec expression overloads
    TestOverloadFunction(dv2a + dv2a);      // dvec2 + dvec2 -> dvec2
    TestOverloadFunction(dv2a * s2);        // dvec2 * double -> dvec2
    TestOverloadFunction(dv3a - dv3a);      // dvec3 - dvec3 -> dvec3
    TestOverloadFunction(dv3a * s2);        // dvec3 * double -> dvec3
    TestOverloadFunction(dv4a / dv4a);      // dvec4 / dvec4 -> dvec4
    TestOverloadFunction(dv4a * s2);        // dvec4 * double -> dvec4

    // test ivec expression overloads
    TestOverloadFunction(iv2a + iv2a);      // ivec2 + ivec2 -> ivec2
    TestOverloadFunction(iv2a * s3);        // ivec2 * int -> ivec2
    TestOverloadFunction(iv3a - iv3a);      // ivec3 - ivec3 -> ivec3
    TestOverloadFunction(iv3a * s3);        // ivec3 * int -> ivec3
    TestOverloadFunction(iv4a + iv4a);      // ivec4 + ivec4 -> ivec4
    TestOverloadFunction(iv4a / s3);        // ivec4 / int -> ivec4

    // test square matrix expression overloads
    mat2 m2a = mat2(1.0f);
    mat2 m2b = mat2(2.0f);
    mat3 m3a = mat3(1.0f);
    mat3 m3b = mat3(2.0f);
    mat4 m4a = mat4(1.0f);
    mat4 m4b = mat4(2.0f);

    TestOverloadFunction(m2a + m2b);        // mat2 + mat2 -> mat2
    TestOverloadFunction(m2a - m2b);        // mat2 - mat2 -> mat2
    TestOverloadFunction(m2a * m2b);        // mat2 * mat2 -> mat2
    TestOverloadFunction(m2a * s1);         // mat2 * scalar -> mat2
    TestOverloadFunction(s1 * m2a);         // scalar * mat2 -> mat2
    TestOverloadFunction(m2a * v2a);        // mat2 * vec2 -> vec2
    TestOverloadFunction(v2a * m2a);        // vec2 * mat2 -> vec2

    TestOverloadFunction(m3a + m3b);        // mat3 + mat3 -> mat3
    TestOverloadFunction(m3a - m3b);        // mat3 - mat3 -> mat3
    TestOverloadFunction(m3a * m3b);        // mat3 * mat3 -> mat3
    TestOverloadFunction(m3a * s1);         // mat3 * scalar -> mat3
    TestOverloadFunction(s1 * m3a);         // scalar * mat3 -> mat3
    TestOverloadFunction(m3a * v3a);        // mat3 * vec3 -> vec3
    TestOverloadFunction(v3a * m3a);        // vec3 * mat3 -> vec3

    TestOverloadFunction(m4a + m4b);        // mat4 + mat4 -> mat4
    TestOverloadFunction(m4a - m4b);        // mat4 - mat4 -> mat4
    TestOverloadFunction(m4a * m4b);        // mat4 * mat4 -> mat4
    TestOverloadFunction(m4a * s1);         // mat4 * scalar -> mat4
    TestOverloadFunction(s1 * m4a);         // scalar * mat4 -> mat4
    TestOverloadFunction(m4a * v4a);        // mat4 * vec4 -> vec4
    TestOverloadFunction(v4a * m4a);        // vec4 * mat4 -> vec4

    // test non-square matrix expression overloads
    mat2x3 m2x3 = mat2x3(1.0f);               // 2 columns, 3 rows
    mat3x2 m3x2 = mat3x2(1.0f);               // 3 columns, 2 rows
    mat2x4 m2x4 = mat2x4(1.0f);               // 2 columns, 4 rows
    mat4x2 m4x2 = mat4x2(1.0f);               // 4 columns, 2 rows
    mat3x4 m3x4 = mat3x4(1.0f);               // 3 columns, 4 rows
    mat4x3 m4x3 = mat4x3(1.0f);               // 4 columns, 3 rows

    TestOverloadFunction(m2x3 + m2x3);      // mat2x3 + mat2x3 -> mat2x3
    TestOverloadFunction(m2x3 - m2x3);      // mat2x3 - mat2x3 -> mat2x3
    TestOverloadFunction(m2x3 * s1);        // mat2x3 * scalar -> mat2x3
    TestOverloadFunction(s1 * m2x3);        // scalar * mat2x3 -> mat2x3
    TestOverloadFunction(m2x3 * v2a);       // mat2x3 * vec2 -> vec3
    TestOverloadFunction(v3a * m2x3);       // vec3 * mat2x3 -> vec2

    TestOverloadFunction(m3x2 + m3x2);      // mat3x2 + mat3x2 -> mat3x2
    TestOverloadFunction(m3x2 - m3x2);      // mat3x2 - mat3x2 -> mat3x2
    TestOverloadFunction(m3x2 * s1);        // mat3x2 * scalar -> mat3x2
    TestOverloadFunction(s1 * m3x2);        // scalar * mat3x2 -> mat3x2
    TestOverloadFunction(m3x2 * v3a);       // mat3x2 * vec3 -> vec2
    TestOverloadFunction(v2a * m3x2);       // vec2 * mat3x2 -> vec3

    TestOverloadFunction(m2x4 + m2x4);      // mat2x4 + mat2x4 -> mat2x4
    TestOverloadFunction(m2x4 - m2x4);      // mat2x4 - mat2x4 -> mat2x4
    TestOverloadFunction(m2x4 * s1);        // mat2x4 * scalar -> mat2x4
    TestOverloadFunction(s1 * m2x4);        // scalar * mat2x4 -> mat2x4
    TestOverloadFunction(m2x4 * v2a);       // mat2x4 * vec2 -> vec4
    TestOverloadFunction(v4a * m2x4);       // vec4 * mat2x4 -> vec2

    TestOverloadFunction(m4x2 + m4x2);      // mat4x2 + mat4x2 -> mat4x2
    TestOverloadFunction(m4x2 - m4x2);      // mat4x2 - mat4x2 -> mat4x2
    TestOverloadFunction(m4x2 * s1);        // mat4x2 * scalar -> mat4x2
    TestOverloadFunction(s1 * m4x2);        // scalar * mat4x2 -> mat4x2
    TestOverloadFunction(m4x2 * v4a);       // mat4x2 * vec4 -> vec2
    TestOverloadFunction(v2a * m4x2);       // vec2 * mat4x2 -> vec4

    TestOverloadFunction(m3x4 + m3x4);      // mat3x4 + mat3x4 -> mat3x4
    TestOverloadFunction(m3x4 - m3x4);      // mat3x4 - mat3x4 -> mat3x4
    TestOverloadFunction(m3x4 * s1);        // mat3x4 * scalar -> mat3x4
    TestOverloadFunction(s1 * m3x4);        // scalar * mat3x4 -> mat3x4
    TestOverloadFunction(m3x4 * v3a);       // mat3x4 * vec3 -> vec4
    TestOverloadFunction(v4a * m3x4);       // vec4 * mat3x4 -> vec3

    TestOverloadFunction(m4x3 + m4x3);      // mat4x3 + mat4x3 -> mat4x3
    TestOverloadFunction(m4x3 - m4x3);      // mat4x3 - mat4x3 -> mat4x3
    TestOverloadFunction(m4x3 * s1);        // mat4x3 * scalar -> mat4x3
    TestOverloadFunction(s1 * m4x3);        // scalar * mat4x3 -> mat4x3
    TestOverloadFunction(m4x3 * v4a);       // mat4x3 * vec4 -> vec3
    TestOverloadFunction(v3a * m4x3);       // vec3 * mat4x3 -> vec4

    // test non-square matrix * matrix overloads
    TestOverloadFunction(m2x3 * m3x2);     // mat2x3(2col,3row) * mat3x2(3col,2row) -> mat3x3 = mat3
    TestOverloadFunction(m3x2 * m2x3);     // mat3x2(3col,2row) * mat2x3(2col,3row) -> mat2x2 = mat2
    TestOverloadFunction(m2x4 * m4x2);     // mat2x4(2col,4row) * mat4x2(4col,2row) -> mat4x4 = mat4
    TestOverloadFunction(m4x2 * m2x4);     // mat4x2(4col,2row) * mat2x4(2col,4row) -> mat2x2 = mat2
    TestOverloadFunction(m3x4 * m4x3);     // mat3x4(3col,4row) * mat4x3(4col,3row) -> mat4x4 = mat4
    TestOverloadFunction(m4x3 * m3x4);     // mat4x3(4col,3row) * mat3x4(3col,4row) -> mat3x3 = mat3
    TestOverloadFunction(m3x4 * m2x3);     // mat3x4(3col,4row) * mat2x3(2col,3row) -> mat2x4
    TestOverloadFunction(m3x2 * m4x3);     // mat3x2(3col,2row) * mat4x3(4col,3row) -> mat4x2
    TestOverloadFunction(m4x2 * m3x4);     // mat4x2(4col,2row) * mat3x4(3col,4row) -> mat3x2
    TestOverloadFunction(m4x3 * m2x4);     // mat4x3(4col,3row) * mat2x4(2col,4row) -> mat2x3

    // test compound matrix/vector/scalar expression chains
    TestOverloadFunction((v2a + v2b) * s1 - v2a / s1);     // vec2
    TestOverloadFunction((v3a - v3b) * s1 + v3b / s1);     // vec3
    TestOverloadFunction((v4a * s1) - (v4b / s1) + v4a);   // vec4
    TestOverloadFunction(m4a * (v4a + v4b));               // mat4 * vec4 -> vec4
    TestOverloadFunction((m3a * m3b) * v3a);               // (mat3 * mat3) * vec3 -> vec3
    TestOverloadFunction(m2x3 * (v2a + v2b));              // mat2x3 * (vec2 + vec2) -> vec3
    TestOverloadFunction((m3x4 * m4x3) * v4a);             // mat4x4 * vec4 -> vec4

    // test non-literal overloads
    TestOverloadFunction(int16arg, int32arg, int64arg, float32arg);
    TestOverloadFunction(uint16arg, uint32arg, uint64arg, float16arg);

    // test bits upgrade
    TestOverloadFunction(int8arg);
    TestOverloadFunction(uint8arg);

    // test ambiguous overloads
    TestOverloadFunction(int16arg, int8arg, int8arg, float16arg);
    TestOverloadFunction(uint16arg, uint8arg, uint8arg, float32arg);

    // test swizzle overloads
    mat4 matrix = mat4(vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(5.0f, 6.0f, 7.0f, 8.0f), vec4(9.0f, 10.0f, 11.0f, 12.0f), vec4(13.0f, 14.0f, 15.0f, 16.0f));
    vec4 vector = matrix[0].xyzw;
    float component1 = vector.x;
    float component2 = matrix[1].xyzw.x;

    TestOverloadFunction(component1);
    TestOverloadFunction(component2);

#define CALL_TEST_OVERLOAD_FUNC TestOverloadFunction(uint16arg, uint32arg, uint64arg, float16arg)

    // --- Super complex compound expressions ---

    // Function call + arithmetic operations + swizzle
    TestOverloadFunction(GetFloat() + GetFloat() * GetFloat());
    TestOverloadFunction(GetDouble() - GetDouble() / GetDouble());
    TestOverloadFunction(GetInt() * GetInt() + GetInt());

    // Function call + vector swizzle + arithmetic
    TestOverloadFunction((GetVec2() + GetVec2()).xy);
    TestOverloadFunction((GetVec3() * GetVec3()).xyz);
    TestOverloadFunction((GetVec4() - GetVec4()).xyzw);
    TestOverloadFunction((GetDVec2() / GetDVec2()).yx);
    TestOverloadFunction((GetIVec3() + GetIVec3()).zyx);
    TestOverloadFunction((GetIVec4() * GetIVec4()).wwzz);

    // Nested function calls with arithmetic
    TestOverloadFunction(GetVec2() + GetVec2() * GetFloat());
    TestOverloadFunction(GetVec3() * GetFloat() - GetVec3() / GetFloat());
    TestOverloadFunction(GetFloat() * GetVec4() + GetVec4() * GetFloat());
    TestOverloadFunction(GetDVec3() + GetDouble() * GetDVec3());
    TestOverloadFunction(GetIVec2() * GetInt() + GetIVec2() / GetInt());

    // Array-like access + swizzle + arithmetic
    vec2  v2Array[3]  = vec2[](vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f));
    vec3  v3Array[2]  = vec3[](vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f));
    vec4  v4Array[2]  = vec4[](vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(5.0f, 6.0f, 7.0f, 8.0f));
    dvec2 dv2Array[2] = dvec2[](dvec2(1.0lf, 2.0lf), dvec2(3.0lf, 4.0lf));
    ivec3 iv3Array[2] = ivec3[](ivec3(1, 2, 3), ivec3(4, 5, 6));
    mat3  m3Array[2]  = mat3[](mat3(1.0f), mat3(2.0f));

    // Additional arrays to test whole-array overload matching
    uint uintArray1D[2] = uint[2](1u, 2u);
    uint uintArray1D_4[4] = uint[4](1u, 2u, 3u, 4u);
    mat4 m4Array[2] = mat4[](mat4(1.0f), mat4(2.0f));

    // also test overloads that accept whole arrays
    TestOverloadFunction(v2Array);
    TestOverloadFunction(v3Array);
    TestOverloadFunction(v4Array);
    TestOverloadFunction(dv2Array);
    TestOverloadFunction(iv3Array);
    TestOverloadFunction(m3Array);
    TestOverloadFunction(uintArray1D);
    TestOverloadFunction(uintArray1D_4);
    TestOverloadFunction(m4Array);

    TestOverloadFunction(v2Array[0] + v2Array[1]);
    TestOverloadFunction(v3Array[0] - v3Array[1]);
    TestOverloadFunction(v4Array[0] * v4Array[1]);
    TestOverloadFunction(dv2Array[0] / dv2Array[1]);
    TestOverloadFunction(iv3Array[0] + iv3Array[1]);
    TestOverloadFunction(m3Array[0] * m3Array[1]);

    // Array access with swizzle
    TestOverloadFunction((v2Array[0] + v2Array[2]).xy);
    TestOverloadFunction((v3Array[1] * v3Array[0]).zyx);
    TestOverloadFunction((v4Array[0] - v4Array[1]).xyzw);
    TestOverloadFunction((dv2Array[0] + dv2Array[1]).yx);
    TestOverloadFunction((iv3Array[1] / iv3Array[0]).xyz);

    // Constructor calls with nested expressions
    TestOverloadFunction(vec2(GetFloat() + GetFloat(), GetFloat() * GetFloat()));
    TestOverloadFunction(vec3(GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat()));
    TestOverloadFunction(vec4(GetFloat() + GetFloat(), GetFloat() - GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat()));
    TestOverloadFunction(dvec2(GetDouble() + GetDouble(), GetDouble() * GetDouble()));
    TestOverloadFunction(dvec3(GetDouble(), GetDouble() / GetDouble(), GetDouble() - GetDouble()));
    TestOverloadFunction(ivec2(GetInt() + GetInt(), GetInt() * GetInt()));
    TestOverloadFunction(ivec4(GetInt(), GetInt() - GetInt(), GetInt() * GetInt(), GetInt() / GetInt()));

    // Constructor calls combining function results
    TestOverloadFunction(vec2(GetVec2().x, GetVec2().y));
    TestOverloadFunction(vec3(GetVec3().x, GetVec3().y, GetVec3().z));
    TestOverloadFunction(vec4(GetVec4().x, GetVec4().y, GetVec4().z, GetVec4().w));
    TestOverloadFunction(dvec2(GetDVec2().x, GetDVec2().y));
    TestOverloadFunction(ivec3(GetIVec3().x, GetIVec3().y, GetIVec3().z));

    // Nested constructor + array access + arithmetic
    TestOverloadFunction(vec2[](vec2(1.0f, 2.0f), vec2(3.0f, 4.0f))[0] + vec2[](vec2(5.0f, 6.0f), vec2(7.0f, 8.0f))[1]);
    TestOverloadFunction(vec3[](vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f))[1] - vec3[](vec3(7.0f, 8.0f, 9.0f), vec3(10.0f, 11.0f, 12.0f))[0]);
    TestOverloadFunction(vec4[](vec4(1.0f), vec4(2.0f))[0] * vec4[](vec4(3.0f), vec4(4.0f))[1]);

    // Matrix operations with multiple levels of calls/access
    TestOverloadFunction(GetMat2() * GetMat2());
    TestOverloadFunction(GetMat3() + GetMat3());
    TestOverloadFunction(GetMat4() - GetMat4());
    TestOverloadFunction(GetMat2() * GetVec2());
    TestOverloadFunction(GetVec3() * GetMat3());
    TestOverloadFunction(GetMat4() * GetVec4());

    // Non-square matrix operations
    TestOverloadFunction(GetMat2x3() + GetMat2x3());
    TestOverloadFunction(GetMat3x2() * GetMat2x3());
    TestOverloadFunction(GetMat2x4() * GetVec2());
    TestOverloadFunction(GetVec4() * GetMat2x4());
    TestOverloadFunction(GetMat3x4() * GetMat4x3());

    // Complex chained expressions with multiple operations
    TestOverloadFunction(GetVec2() + GetVec2() * GetFloat() - GetVec2() / GetFloat());
    TestOverloadFunction(GetVec3() * GetFloat() + GetVec3() / GetFloat() - GetVec3() * GetFloat());
    TestOverloadFunction(GetVec4() - GetVec4() * GetFloat() + GetVec4() / GetFloat() * GetFloat());
    TestOverloadFunction(GetDVec2() / GetDVec2() * GetDouble() + GetDVec2() - GetDVec2() * GetDouble());
    TestOverloadFunction(GetIVec3() + GetIVec3() * GetInt() - GetIVec3() / GetInt() + GetIVec3());

    // Array access + function call + swizzle + arithmetic in chains
    TestOverloadFunction((v2Array[0] + GetVec2()).xy);
    TestOverloadFunction((v3Array[1] * GetVec3()).zyx);
    TestOverloadFunction((v4Array[0] - GetVec4()).xyzw);
    TestOverloadFunction((dv2Array[0] / GetDVec2()).yx);
    TestOverloadFunction((iv3Array[1] + GetIVec3()).xyz);

    // Constructor with array element and arithmetic
    TestOverloadFunction(vec2(v2Array[0].x + v2Array[1].x, v2Array[0].y * v2Array[1].y));
    TestOverloadFunction(vec3(v3Array[0].x + GetFloat(), v3Array[1].y * GetFloat(), v3Array[0].z / GetFloat()));
    TestOverloadFunction(ivec2(iv3Array[0].x * GetInt(), iv3Array[1].y + GetInt()));

    // Matrix array access with arithmetic
    TestOverloadFunction(m3Array[0] * m3Array[1]);
    TestOverloadFunction(m3Array[0] + m3Array[1] * GetFloat());
    TestOverloadFunction(m3Array[1] - m3Array[0] / GetFloat());
    TestOverloadFunction(m3Array[0] * GetVec3());
    TestOverloadFunction(GetVec3() * m3Array[1]);

    // Deeply nested expressions
    TestOverloadFunction((GetVec2() + (GetVec2() * GetFloat())) / (GetVec2() - GetFloat()));
    TestOverloadFunction((GetVec3() * GetFloat() + GetVec3()) - (GetVec3() / GetFloat() * GetVec3()));
    TestOverloadFunction((GetVec4() + GetVec4()) * (GetFloat() + GetFloat()) - (GetVec4() - GetVec4()));
    TestOverloadFunction((GetDVec2() * GetDouble() + GetDVec2()) / (GetDVec2() + GetDouble()));
    TestOverloadFunction((GetIVec3() + GetIVec3() * GetInt()) - (GetIVec3() - GetInt()));

    // Expressions with member access chains
    TestOverloadFunction(vec2(v2Array[0].x, v2Array[1].x));
    TestOverloadFunction(vec3(v3Array[0].x, v3Array[0].y, v3Array[1].z));
    TestOverloadFunction(vec4(v4Array[0].x, v4Array[0].y, v4Array[1].z, v4Array[1].w));
    TestOverloadFunction(dvec2(dv2Array[0].x, dv2Array[1].y));
    TestOverloadFunction(ivec3(iv3Array[0].x, iv3Array[1].y, iv3Array[0].z));

    // Constructor with mixed function calls and array access
    TestOverloadFunction(vec2(v2Array[0] + GetVec2()));
    TestOverloadFunction(vec3(v3Array[1] * GetVec3()));
    TestOverloadFunction(vec4(v4Array[0] - GetVec4()));
    TestOverloadFunction(dvec2(dv2Array[0] / GetDVec2()));
    TestOverloadFunction(ivec3(iv3Array[1] + GetIVec3()));

    // Swizzle on constructor results
    TestOverloadFunction((vec2(GetFloat() + GetFloat(), GetFloat() * GetFloat())).xy);
    TestOverloadFunction((vec3(GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat())).zyx);
    TestOverloadFunction((vec4(GetFloat() + GetFloat(), GetFloat() - GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat())).xyzw);

    // Matrix constructor with nested expressions
    TestOverloadFunction(mat2(vec2(GetFloat() + GetFloat(), GetFloat() * GetFloat()), vec2(GetFloat() - GetFloat(), GetFloat() / GetFloat())));
    TestOverloadFunction(mat3(GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat(), GetFloat() + GetFloat(), GetFloat() - GetFloat(), GetFloat() * GetFloat(), GetFloat(), GetFloat(), GetFloat()));
    TestOverloadFunction(mat4(vec4(GetFloat()), vec4(GetFloat() * GetFloat()), vec4(GetFloat() / GetFloat()), vec4(GetFloat() + GetFloat())));

    // Arithmetic on matrix constructor results
    TestOverloadFunction(mat2(vec2(GetFloat()), vec2(GetFloat())) + mat2(vec2(GetFloat()), vec2(GetFloat())));
    TestOverloadFunction(mat3(GetFloat()) * mat3(GetFloat() * GetFloat()));
    TestOverloadFunction(mat4(vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat())) - mat4(vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat())));

    // Array of constructors with arithmetic
    TestOverloadFunction(vec2[](vec2(GetFloat()), vec2(GetFloat() * GetFloat()))[0] + vec2[](vec2(GetFloat() / GetFloat()), vec2(GetFloat() + GetFloat()))[1]);
    TestOverloadFunction(vec3[](vec3(GetFloat()), vec3(GetFloat() * GetFloat()))[1] - vec3[](vec3(GetFloat()), vec3(GetFloat()))[0]);
    TestOverloadFunction(vec4[](vec4(GetFloat() + GetFloat()), vec4(GetFloat() - GetFloat()))[0] * vec4[](vec4(GetFloat()), vec4(GetFloat()))[1]);

    // Super nested: array[function().member operation] + constructor
    TestOverloadFunction(vec2(v2Array[0].x + GetVec2().x, v2Array[1].y * GetVec2().y));
    TestOverloadFunction(vec3(v3Array[0].x + GetVec3().x, v3Array[1].y - GetVec3().y, v3Array[0].z * GetVec3().z));
    TestOverloadFunction(dvec2(dv2Array[0].x * GetDVec2().x, dv2Array[1].y / GetDVec2().y));

    // Chain of swizzles and operations
    TestOverloadFunction(((GetVec4().xy + GetVec2()) * GetFloat()).xy);
    TestOverloadFunction(((GetVec3().zyx - GetVec3()) / GetFloat()).xyz);
    TestOverloadFunction(((GetVec2().yx + GetVec2().xy) * GetFloat()).xy);

    // --- Test array types with different dimensions and element types ---

    // 1D arrays
    int    intArray1D_2[2]  = int[2](1, 2);
    int    intArray1D_3[3]  = int[3](1, 2, 3);
    float  floatArray1D[5]  = float[5](1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
    double doubleArray1D[4] = double[4](1.0lf, 2.0lf, 3.0lf, 4.0lf);

    TestOverloadFunction(floatArray1D); // Test array parameter overload
    TestOverloadFunction(intArray1D_2); // Test array parameter overload (size 2)
    TestOverloadFunction(intArray1D_3); // Test array parameter overload (size 3)

    // 1D arrays of scalar types with different precision
    int8_t    int8Array1D[3]    = int8_t[3](int8_t(1), int8_t(2), int8_t(3));
    uint16_t  uint16Array1D[3]  = uint16_t[3](uint16_t(1), uint16_t(2), uint16_t(3));
    float32_t float32Array1D[3] = float32_t[3](1.0f, 2.0f, 3.0f);
    float64_t float64Array1D[3] = float64_t[3](1.0lf, 2.0lf, 3.0lf);

    TestOverloadFunction(int8Array1D[0]);
    TestOverloadFunction(uint16Array1D[1]);

    // 1D arrays of vectors
    vec2 vec2Array1D[4] = vec2[4](vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f), vec2(7.0f, 8.0f));
    vec3 vec3Array1D[3] = vec3[3](vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f));

    TestOverloadFunction(vec2Array1D); // Test array parameter overload

    // 2D arrays
    int intArray2D[2][3] = int[2][3](
        int[3](1, 2, 3),
        int[3](4, 5, 6)
    );

    float floatArray2D[3][4] = float[3][4](
        float[4](1.0f, 2.0f, 3.0f, 4.0f),
        float[4](5.0f, 6.0f, 7.0f, 8.0f),
        float[4](9.0f, 10.0f, 11.0f, 12.0f)
    );

    TestOverloadFunction(intArray2D); // Test array parameter overload

    vec3 vec3Array2D[2][2] = vec3[2][2](
        vec3[2](vec3(1.0f), vec3(2.0f)),
        vec3[2](vec3(3.0f), vec3(4.0f))
    );

    mat2 mat2Array2D[2][2] = mat2[2][2](
        mat2[2](mat2(1.0f), mat2(2.0f)),
        mat2[2](mat2(3.0f), mat2(4.0f))
    );

    TestOverloadFunction(vec3Array2D); // Test array parameter overload
    TestOverloadFunction(mat2Array2D); // Test array parameter overload

    // 3D arrays
    float floatArray3D[2][2][3] = float[2][2][3](
        float[2][3](
            float[3](1.0f, 2.0f, 3.0f),
            float[3](4.0f, 5.0f, 6.0f)
        ),
        float[2][3](
            float[3](7.0f, 8.0f, 9.0f),
            float[3](10.0f, 11.0f, 12.0f)
        )
    );

    vec2 vec2Array3D[2][2][2] = vec2[2][2][2](
        vec2[2][2](
            vec2[2](vec2(1.0f), vec2(2.0f)),
            vec2[2](vec2(3.0f), vec2(4.0f))
        ),
        vec2[2][2](
            vec2[2](vec2(5.0f), vec2(6.0f)),
            vec2[2](vec2(7.0f), vec2(8.0f))
        )
    );

    vec2 vec2Array2D[2][2] = vec2[2][2](
        vec2[2](vec2(1.0f), vec2(2.0f)),
        vec2[2](vec2(3.0f), vec2(4.0f))
    );

    vec3 vec3Array3D[2][2][2] = vec3[2][2][2](
        vec3[2][2](
            vec3[2](vec3(1.0f), vec3(2.0f)),
            vec3[2](vec3(3.0f), vec3(4.0f))
        ),
        vec3[2][2](
            vec3[2](vec3(5.0f), vec3(6.0f)),
            vec3[2](vec3(7.0f), vec3(8.0f))
        )
    );

    // Test access and operations
    TestOverloadFunction(floatArray3D[0][1][2]);
    TestOverloadFunction(vec2Array3D[1][0][0]);

    TestOverloadFunction(vec2Array3D[0][0][0]);
    TestOverloadFunction(vec2Array3D[1][0][0]);

    // 3D arrays with arithmetic operations
    TestOverloadFunction(vec2Array3D[0][0][0] + vec2Array3D[1][1][1]);
    TestOverloadFunction(vec3Array3D[0][1][0] * vec3Array3D[1][0][1]);

    // 3D arrays with complex indexing and operations
    TestOverloadFunction((vec2Array3D[0][1][1] + vec2Array3D[1][0][0]).xy);
    TestOverloadFunction((vec3Array3D[0][0][0] - vec3Array3D[1][1][1]).xyz);

    // Mixed operations: cross-dimensional access
    TestOverloadFunction(floatArray1D[0] + floatArray2D[0][0] + floatArray3D[0][0][0]);
    TestOverloadFunction(vec2Array1D[0] + vec2Array2D[0][0] + vec2Array3D[0][0][0]);

    // Array elements with constructor calls
    TestOverloadFunction(vec2(floatArray2D[0][0], floatArray2D[1][1]));
    TestOverloadFunction(vec3(floatArray3D[0][0][0], floatArray3D[1][0][1], floatArray3D[0][1][0]));
    TestOverloadFunction(ivec2(intArray2D[0][0], intArray2D[1][1]));

    // Array elements in complex expressions
    TestOverloadFunction(floatArray2D[0][0] * floatArray2D[1][2] + floatArray3D[1][1][2]);
    TestOverloadFunction(vec2Array2D[0][0] * vec2Array3D[1][0][1] + vec2Array1D[2]);

    // --- 1. 整数类型 (int / uint / uint16_t) ---

    // int[2]
    int i2[2] = int[2](1, 2);
    TestOverloadFunction(i2);                           // 变量
    TestOverloadFunction(int[2](3, 4));                 // 构造函数
    TestOverloadFunction(int[](5, 6));                  // 无维度构造

    // int[3]
    int i3[3] = int[3](1, 2, 3);
    TestOverloadFunction(i3);
    TestOverloadFunction(int[3](4, 5, 6));
    TestOverloadFunction(int[](7, 8, 9));

    // int[2][3] (多维数组)
    int i2x3[2][3] = int[2][3](int[3](1, 2, 3), int[3](4, 5, 6));
    TestOverloadFunction(i2x3);
    TestOverloadFunction(int[2][3](int[3](1, 1, 1), int[3](2, 2, 2)));
    TestOverloadFunction(int[][3](int[3](0, 0, 0), int[3](0, 0, 0)));

    // uint[2] / uint[4]
    uint u4[4] = uint[4](1u, 2u, 3u, 4u);
    TestOverloadFunction(u4);
    TestOverloadFunction(uint[2](10u, 20u));
    TestOverloadFunction(uint[](100u, 200u, 300u, 400u));

    // uint16_t[3]
    uint16_t u16_3[3] = uint16_t[3](1us, 2us, 3us);
    TestOverloadFunction(u16_3);
    TestOverloadFunction(uint16_t[3](4us, 5us, 6us));
    TestOverloadFunction(uint16_t[](7us, 8us, 9us));

    // int[2] 与 int[3]
    int arrI2[2] = {10, 20};
    TestOverloadFunction(arrI2);
    TestOverloadFunction({30, 40});        // 直接传入 int[2]
    TestOverloadFunction({1, 2, 3});       // 直接传入 int[3]

    // int[2][3] (多维数组嵌套)
    TestOverloadFunction({{1, 2, 3}, {4, 5, 6}}); 

    // uint[2] 与 uint[4]
    uint arrU4[4] = {1u, 2u, 3u, 4u};
    TestOverloadFunction(arrU4);
    TestOverloadFunction({10u, 20u});      // 对应 uint[2]

    // uint16_t[3]
    TestOverloadFunction({10us, 20us, 30us});

    // --- 2. 浮点类型 (float / double) ---

    // float[5]
    float f5[5] = float[5](1.0, 2.0, 3.0, 4.0, 5.0);
    TestOverloadFunction(f5);
    TestOverloadFunction(float[5](0.1, 0.2, 0.3, 0.4, 0.5));
    TestOverloadFunction(float[](1.1, 2.2, 3.3, 4.4, 5.5));

    // float[2][2][3] (三维数组)
    float f3D[2][2][3]; // 初始化略
    TestOverloadFunction(f3D);
    TestOverloadFunction(float[2][2][3](
        float[2][3](float[3](1, 1, 1), float[3](1, 1, 1)),
        float[2][3](float[3](2, 2, 2), float[3](2, 2, 2))
    ));
    TestOverloadFunction(float[][][](
        float[][](float[](0, 0, 0), float[](0, 0, 0)),
        float[][](float[](0, 0, 0), float[](0, 0, 0))
    ));

    // double[4]
    double d4[4] = double[4](1.0LF, 2.0LF, 3.0LF, 4.0LF);
    TestOverloadFunction(d4);
    TestOverloadFunction(double[4](0.0LF, 0.0LF, 0.0LF, 0.0LF));
    TestOverloadFunction(double[](9.0LF, 8.0LF, 7.0LF, 6.0LF));

    // float[5]
    TestOverloadFunction({1.1, 2.2, 3.3, 4.4, 5.5});

    // float[2][3] 与 float[3][4]
    float arrF2x3[2][3] = {{1., 2., 3.}, {4., 5., 6.}};
    TestOverloadFunction(arrF2x3);
    TestOverloadFunction({{0.,0.,0.,0.}, {1.,1.,1.,1.}, {2.,2.,2.,2.}}); // 对应 float[3][4]

    // float[2][2][3] (三维)
    TestOverloadFunction({
        {{1.,1.,1.}, {2.,2.,2.}}, 
        {{3.,3.,3.}, {4.,4.,4.}}
    });

    // double[4]
    TestOverloadFunction({1.0LF, 2.0LF, 3.0LF, 4.0LF});

    // --- 3. 向量类型 (vec2, ivec3, vec3, vec4, dvec2) ---

    // vec2[3]
    vec2 v2_3[3] = vec2[3](vec2(0), vec2(1), vec2(2));
    TestOverloadFunction(v2_3);
    TestOverloadFunction(vec2[3](vec2(1.1), vec2(2.2), vec2(3.3)));
    TestOverloadFunction(vec2[](vec2(0.5), vec2(0.6), vec2(0.7)));

    // vec2[2][2][2] (三维向量数组)
    vec2 v2_3D[2][2][2];
    TestOverloadFunction(v2_3D);
    TestOverloadFunction(vec2[2][2][2](
        vec2[2][2](vec2[2](vec2(1), vec2(1)), vec2[2](vec2(1), vec2(1))),
        vec2[2][2](vec2[2](vec2(1), vec2(1)), vec2[2](vec2(1), vec2(1)))
    ));

    // ivec3[2] / vec3[2] / vec4[2]
    vec3 v3_2[2] = vec3[2](vec3(1, 0, 0), vec3(0, 1, 0));
    TestOverloadFunction(v3_2);
    TestOverloadFunction(ivec3[2](ivec3(1), ivec3(2)));
    TestOverloadFunction(vec4[](vec4(1), vec4(0)));

    // dvec2[2]
    dvec2 dv2_2[2] = dvec2[2](dvec2(1.0LF), dvec2(2.0LF));
    TestOverloadFunction(dv2_2);
    TestOverloadFunction(dvec2[2](dvec2(0), dvec2(1)));
    TestOverloadFunction(dvec2[](dvec2(5), dvec2(6)));

    // vec2[3] 与 vec2[4]
    TestOverloadFunction({vec2(0), vec2(1), vec2(2)});
    vec2 arrV2_4[4] = {vec2(0), vec2(0.5), vec2(0.8), vec2(1.0)};
    TestOverloadFunction(arrV2_4);

    // vec2[2][2][2] (三维向量数组)
    TestOverloadFunction({
        {{vec2(0), vec2(0)}, {vec2(1), vec2(1)}},
        {{vec2(2), vec2(2)}, {vec2(3), vec2(3)}}
    });

    // ivec3[2], vec3[2], vec4[2], dvec2[2]
    TestOverloadFunction({ivec3(1), ivec3(2)});
    TestOverloadFunction({vec3(0.1), vec3(0.2)});
    TestOverloadFunction({vec4(0), vec4(1)});
    TestOverloadFunction({dvec2(1.0LF), dvec2(2.0LF)});

    // vec3[2][2]
    TestOverloadFunction({{vec3(0), vec3(0)}, {vec3(1), vec3(1)}});

    // --- 4. 矩阵类型 (mat2, mat3, mat4) ---

    // mat2[2][2] (多维矩阵数组)
    mat2 m2x2x2[2][2] = mat2[2][2](
        mat2[2](mat2(1.0), mat2(1.0)),
        mat2[2](mat2(1.0), mat2(1.0))
    );
    TestOverloadFunction(m2x2x2);
    TestOverloadFunction(mat2[2][2](mat2[2](mat2(0), mat2(0)), mat2[2](mat2(0), mat2(0))));
    TestOverloadFunction(mat2[][](mat2[2](mat2(1), mat2(1)), mat2[2](mat2(1), mat2(1))));

    // mat3[2] / mat4[2]
    mat4 m4_2[2] = mat4[2](mat4(1.0), mat4(1.0));
    TestOverloadFunction(m4_2);
    TestOverloadFunction(mat3[2](mat3(1), mat3(1)));
    TestOverloadFunction(mat4[](mat4(2), mat4(2)));

    // mat3[2] 与 mat4[2]
    TestOverloadFunction({mat3(1.0), mat3(1.0)});
    mat4 arrM4[2] = {mat4(0), mat4(1)};
    TestOverloadFunction(arrM4);

    // mat2[2][2] (多维矩阵数组)
    TestOverloadFunction({
        {mat2(1), mat2(1)},
        {mat2(0), mat2(0)}
    });

    // 链式调用与复杂数据结构测试
    InnerData localInner;
    localInner.float_field          = 1.0f;
    localInner.int_field            = 42;
    localInner.vec3_field           = vec3(1.0, 2.0, 3.0);
    localInner.mat2_field[0]        = mat2(1.0);
    localInner.intArray2D_2x3[0][0] = 7;

    MiddleData localMiddle;
    localMiddle.inner      = localInner;
    localMiddle.v4Array[0] = vec4(1.0);
    localMiddle.mat4_field = mat4(1.0);

    OuterData localOuter;
    localOuter.middle[0]   = localMiddle;
    localOuter.middle[1]   = localMiddle;
    localOuter.dvec2_field = dvec2(0.5, 0.5);

    // 各种重载匹配测试
    TestOverloadFunction(localInner);                                      // 匹配 InnerData
    TestOverloadFunction(localMiddle);                                     // 匹配 MiddleData
    TestOverloadFunction(localOuter);                                      // 匹配 OuterData

    TestOverloadFunction(localOuter.middle[1]);                            // 匹配 MiddleData
    TestOverloadFunction(localOuter.middle[0].inner);                      // 匹配 InnerData
    TestOverloadFunction(localOuter.middle[0].inner.vec3_field);           // 匹配 vec3
    TestOverloadFunction(localOuter.middle[0].inner.mat2_field[1]);        // 匹配 mat2
    TestOverloadFunction(localOuter.middle[0].v4Array[2]);                 // 匹配 vec4
    TestOverloadFunction(localOuter.middle[0].v4Array[2].xyz);             // 匹配 vec3
    TestOverloadFunction(localOuter.middle[1].mat4_field[0]);              // 匹配 vec4 (mat4 的列是 vec4)
    TestOverloadFunction(localOuter.middle[1].mat4_field[0].x);            // 匹配 float32_t

    // 数组重载匹配
    TestOverloadFunction(localOuter.middle[0].inner.mat2_field);           // 匹配 mat2[2]
    TestOverloadFunction(localOuter.middle[0].inner.intArray2D_2x3);       // 匹配 int[2][3]
    TestOverloadFunction(localOuter.middle[0].inner.intArray2D_2x3[0]);    // 匹配 int[3]
    TestOverloadFunction(localOuter.middle[0].inner.intArray2D_2x3[1][2]); // 匹配 int32_t

    // 运算 + 链式调用
    TestOverloadFunction(localOuter.middle[0].inner.vec3_field + vec3(1.0));                            // 匹配 vec3
    TestOverloadFunction(localOuter.middle[1].mat4_field * localOuter.middle[0].inner.vec3_field.xyzz); // 匹配 vec4
    TestOverloadFunction(dot(localOuter.middle[0].inner.vec3_field, vec3(0.5)));                        // 匹配 float32_t

    struct {
        int int_field;
        vec4 vec4_field;
        mat4 mat4_field;
        OuterData data;
    } MyStruct;

    TestOverloadFunction(MyStruct.int_field);
    TestOverloadFunction(MyStruct.data);

    TestOverloadFunction(MyStruct.data.dvec2_field.xyz);
    TestOverloadFunction(MyStruct.data.dvec2_field.xxyy);

    TestOverloadFunction(MyStruct.data.middle[0]);
    TestOverloadFunction(MyStruct.data.middle[1].inner);

    TestOverloadFunction(MyStruct.data.middle[0].inner.mat2_field[0][0].xxxx);
    TestOverloadFunction(mat3x4(MyStruct.data.middle[1].v4Array));
    TestOverloadFunction(MyStruct.data.middle[1].inner.intArray2D_2x3[2][1] * MyStruct.data.dvec2_field.zz);
    TestOverloadFunction(MyStruct.data.middle[0].inner.vec3_field * MyStruct.data.middle[1].inner.float_field);

    MySpirvType0 type0;
    MySpirvType1 type1;
    MySpirvType2 type2;
    MySpirvType3 type3;
    MySpirvType4 type4;
    MySpirvType5 type5;
    MySpirvType6 type6;
    MySpirvType7 type7;
    MySpirvType8 type8;

    TestOverloadFunction(type0);
    TestOverloadFunction(type1);
    TestOverloadFunction(type2);
    TestOverloadFunction(type3);
    TestOverloadFunction(type4);
    TestOverloadFunction(type5);
    TestOverloadFunction(type6);
    TestOverloadFunction(type7);
    TestOverloadFunction(type8);
    TestOverloadFunction(type0, type1);
    TestOverloadFunction(type1, type0);
}

void TestOverloadFunction() {}
