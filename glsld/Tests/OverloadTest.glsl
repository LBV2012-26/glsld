#version 460 core
#extension GL_EXT_shader_explicit_arithmetic_types_int8    : require
#extension GL_EXT_shader_explicit_arithmetic_types_int16   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int32   : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64   : require
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : require
#extension GL_EXT_shader_explicit_arithmetic_types_float64 : require

void TestOverloadFunction(int16_t int16arg, int32_t int32arg, int64_t int64arg, float32_t float32arg) {}
void TestOverloadFunction(uint16_t uint16arg, uint32_t uint32arg, uint64_t uint64arg, float16_t float16arg) {}

void TestOverloadFunction();
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
    TestOverloadFunction((m3a * m3b) * v3a);               // (mat3*mat3)*vec3 -> vec3
    TestOverloadFunction(m2x3 * (v2a + v2b));              // mat2x3*(vec2+vec2) -> vec3
    TestOverloadFunction((m3x4 * m4x3) * v4a);             // mat4x4 * vec4 -> vec4

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

    #define CALL_TEST_OVERLOAD_FUNC TestOverloadFunction(int16arg, int32arg, int64arg, float32arg)
}

void TestOverloadFunction() {}
