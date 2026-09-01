#version 460 core
#pragma shader_stage(compute)

#extension GL_EXT_shader_explicit_arithmetic_types : require
#extension GL_EXT_buffer_reference                 : require
#extension GL_EXT_buffer_reference_uvec2           : require
#extension GL_KHR_cooperative_matrix               : require
#extension GL_KHR_memory_scope_semantics           : require
#extension GL_NV_explicit_typecast                 : require

void OverloadFunction(int16_t Int16, int32_t Int32, int64_t Int64, float32_t Float32) {}
void OverloadFunction(uint16_t UInt16, uint32_t UInt32, uint64_t UInt64, float16_t Float16) {}

void OverloadFunction(); // Test declare and implement toggle
void OverloadFunction(bool Bool) {}

void OverloadFunction(vec2 float2) {}
void OverloadFunction(vec3 float3) {}
void OverloadFunction(vec4 float4) {}

void OverloadFunction(dvec2 double2) {}
void OverloadFunction(dvec3 double3) {}
void OverloadFunction(dvec4 double4) {}

void OverloadFunction(ivec2 int2) {}
void OverloadFunction(ivec3 int3) {}
void OverloadFunction(ivec4 int4) {}

void OverloadFunction(bvec2 bool2) {}
void OverloadFunction(bvec3 bool3) {}
void OverloadFunction(bvec4 bool4) {}

void OverloadFunction(mat2x2 float2x2) {}
void OverloadFunction(mat2x3 float2x3) {}
void OverloadFunction(mat2x4 float2x4) {}
void OverloadFunction(mat3x2 float3x2) {}
void OverloadFunction(mat3x3 float3x3) {}
void OverloadFunction(mat3x4 float3x4) {}
void OverloadFunction(mat4x2 float4x2) {}
void OverloadFunction(mat4x3 float4x3) {}
void OverloadFunction(mat4x4 float4x4) {}

void OverloadFunction(int8_t Int8) {}
void OverloadFunction(uint8_t UInt8) {}
void OverloadFunction(int16_t Int16) {}
void OverloadFunction(uint16_t UInt16) {}
void OverloadFunction(int32_t Int32) {}
void OverloadFunction(uint32_t UInt32) {}
void OverloadFunction(int64_t Int64) {}
void OverloadFunction(uint64_t UInt64) {}

void OverloadFunction(float16_t Float16) {}
void OverloadFunction(float32_t Float32) {}
void OverloadFunction(float64_t Float64) {}

void OverloadFunction(int intArray1D_2[2]) {}
void OverloadFunction(int intArray1D_3[3]) {}
void OverloadFunction(int intArray2D_2x3[2][3]) {}
void OverloadFunction(uint uintArray1D_2[2]) {}
void OverloadFunction(uint uintArray1D_4[4]) {}
void OverloadFunction(uint16_t uint16Array1D_3[3]) {}
void OverloadFunction(float floatArray1D_5[5]) {}
void OverloadFunction(float floatArray2D_2x3[2][3]) {}
void OverloadFunction(float floatArray2D_3x4[3][4]) {}
void OverloadFunction(float floatArray3D_2x2x3[2][2][3]) {}
void OverloadFunction(double doubleArray1D_4[4]) {}
void OverloadFunction(vec2 vec2Array1D_3[3]) {}
void OverloadFunction(vec2 vec2Array1D_4[4]) {}
void OverloadFunction(vec2 vec2Array2D_2x2[2][2]) {}
void OverloadFunction(vec2 vec2Array3D_2x2x2[2][2][2]) {}
void OverloadFunction(dvec2 dvec2Array1D_2[2]) {}
void OverloadFunction(ivec3 ivec3Array1D_2[2]) {}
void OverloadFunction(vec3 vec3Array1D_2[2]) {}
void OverloadFunction(vec3 vec3Array2D_2x2[2][2]) {}
void OverloadFunction(vec4 vec4Array1D_2[2]) {}
void OverloadFunction(mat2 mat2Array1D_2[2]) {}
void OverloadFunction(mat2 mat2Array2D_2x2[2][2]) {}
void OverloadFunction(mat3 mat3Array1D_2[2]) {}
void OverloadFunction(mat4 mat4Array1D_2[2]) {}

void OverloadFunction(int intArray[]);
void OverloadFunction(uint uintArray[]);

int GetInt() { return 1; }
float GetFloat() { return 1.0f; }
double GetDouble() { return 1.0lf; }
vec2 GetVec2() { return vec2(1.0f, 2.0f); }
vec3 GetVec3() { return vec3(1.0f, 2.0f, 3.0f); }
vec4 GetVec4() { return vec4(1.0f, 2.0f, 3.0f, 4.0f); }
dvec2 GetDVec2() { return dvec2(1.0lf, 2.0lf); }
dvec3 GetDVec3() { return dvec3(1.0lf, 2.0lf, 3.0lf); }
dvec4 GetDVec4() { return dvec4(1.0lf, 2.0lf, 3.0lf, 4.0lf); }
ivec2 GetIVec2() { return ivec2(1, 2); }
ivec3 GetIVec3() { return ivec3(1, 2, 3); }
ivec4 GetIVec4() { return ivec4(1, 2, 3, 4); }
mat2x2 GetMat2x2() { return mat2x2(1.0f); }
mat2x3 GetMat2x3() { return mat2x3(1.0f); }
mat2x4 GetMat2x4() { return mat2x4(1.0f); }
mat3x2 GetMat3x2() { return mat3x2(1.0f); }
mat3x3 GetMat3x3() { return mat3x3(1.0f); }
mat3x4 GetMat3x4() { return mat3x4(1.0f); }
mat4x2 GetMat4x2() { return mat4x2(1.0f); }
mat4x3 GetMat4x3() { return mat4x3(1.0f); }
mat4x4 GetMat4x4() { return mat4x4(1.0f); }

#define MySpirvTypeX spirv_type(set = "GLSL.std.450", capabilities = [1145], extensions = ["SPV_KHR_ray_tracing"], id = 4514) // same as T1
#define MySpirvType0 spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [5353], set = "GLSL.std.450", id = 5341)
#define MySpirvType1 spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [1145], set = "GLSL.std.450", id = 4514)
#define MySpirvType2 spirv_type(id = 32, spirv_id 64, 2)
#define MySpirvType3 spirv_type(id = 32, spirv_id 64)
#define MySpirvType4 spirv_type(id = 32, spirv_id 32)
#define MySpirvType5 spirv_type(id = 21, 64, 1)
#define MySpirvType6 spirv_type(id = 32, 32, 1)
#define MySpirvType7 spirv_type(id = 32, 64)
#define MySpirvType8 spirv_type(id = 32, 32)

void OverloadFunction(MySpirvType0 Param0);
void OverloadFunction(MySpirvType1 Param1);
void OverloadFunction(MySpirvType2 Param2);
void OverloadFunction(MySpirvType3 Param3);
void OverloadFunction(MySpirvType4 Param4);
void OverloadFunction(MySpirvType5 Param5);
void OverloadFunction(MySpirvType6 Param6);
void OverloadFunction(MySpirvType7 Param7);
void OverloadFunction(MySpirvType8 Param8);
void OverloadFunction(MySpirvType0 Param0, MySpirvType1 Param1);
void OverloadFunction(MySpirvType1 Param1, MySpirvType0 Param0);

#define COOPMAT_TYPE_16 coopmat<float16_t, gl_ScopeSubgroup, 16, 16, gl_MatrixUseA>
#define COOPMAT_TYPE_32 coopmat<float16_t, gl_ScopeSubgroup, 32, 32, gl_MatrixUseB>

void OverloadFunction(COOPMAT_TYPE_16 CoopMat16);
void OverloadFunction(COOPMAT_TYPE_32 CoopMat32);
void OverloadFunction(coopmat Matrix, int Int);
void OverloadFunction(coopmat Matrix, uint Uint);

void OverloadFunction(_Func Func);

struct InnerData {
    float Float;
    int   Int;
    vec3  float3;
    mat2  mat2Array1D_2[2];
    int   intArray2D_2x3[2][3];
};

struct MiddleData {
    InnerData Inner;
    vec4      vec4Array2D_2[3];
    mat4      float4x4;
};

struct OuterData {
    MiddleData Middle[2];
    dvec2      double2;
};

void OverloadFunction(InnerData InnerData) {}
void OverloadFunction(MiddleData MiddleData) {}
void OverloadFunction(OuterData OuterData) {}
void OverloadFunction(InnerData InnerData, MiddleData MiddleData, OuterData OuterData) {}
void OverloadFunction(OuterData OuterData, MiddleData MiddleData, InnerData InnerData) {}
void OverloadFunction(InnerData InnerData, OuterData OuterData, MiddleData MiddleData) {}
void OverloadFunction(OuterData OuterData, InnerData InnerData, MiddleData MiddleData) {}
void OverloadFunction(MiddleData MiddleData, InnerData InnerData) {}
void OverloadFunction(OuterData OuterData, MiddleData MiddleData) {}

layout(buffer_reference, scalar) buffer _Buffer {
    uint data[];
};

layout(buffer_reference, scalar) buffer _OtherBuffer {
    uint data[];
};

void main() {
    int8_t    Int8;
    int16_t   Int16;
    int32_t   Int32;
    int64_t   Int64;

    uint8_t   UInt8;
    uint16_t  UInt16;
    uint32_t  UInt32;
    uint64_t  UInt64;

    float16_t Float16;
    float32_t Float32;
    float64_t Float64;

#define CALL_TEST_OVERLOAD_FUNC OverloadFunction(UInt16, UInt32, UInt64, Float16)

    // test literal overloads
    OverloadFunction(true);
    OverloadFunction(false);
    OverloadFunction(1);
    OverloadFunction(1u);
    OverloadFunction(1.0);
    OverloadFunction(1.0hf);
    OverloadFunction(1.0f);
    OverloadFunction(1.0lf);

    // int8 -> int
    OverloadFunction(Int8 + Int8);           // int8 + int8 -> int8
    OverloadFunction(Int8 * Int8);           // int8 * int8 -> int8
    OverloadFunction(Int8 - Int8);           // int8 - int8 -> int8
    OverloadFunction(Int8 / Int8);           // int8 / int8 -> int8

    // uint8 -> uint
    OverloadFunction(UInt8 + UInt8);         // uint8 + uint8 -> uint8
    OverloadFunction(UInt8 * UInt8);         // uint8 * uint8 -> uint8
    OverloadFunction(UInt8 - UInt8);         // uint8 - uint8 -> uint8
    OverloadFunction(UInt8 / UInt8);         // uint8 / uint8 -> uint8

    // int16 -> int
    OverloadFunction(Int16 + Int16);         // int16 + int16 -> int16
    OverloadFunction(Int16 * Int16);         // int16 * int16 -> int16
    OverloadFunction(Int16 - Int16);         // int16 - int16 -> int16
    OverloadFunction(Int16 / Int16);         // int16 / int16 -> int16

    // uint16 -> uint
    OverloadFunction(UInt16 + UInt16);       // uint16 + uint16 -> uint16
    OverloadFunction(UInt16 * UInt16);       // uint16 * uint16 -> uint16
    OverloadFunction(UInt16 - UInt16);       // uint16 - uint16 -> uint16
    OverloadFunction(UInt16 / UInt16);       // uint16 / uint16 -> uint16

    // int32 -> int
    OverloadFunction(Int32 + Int32);         // int32 + int32 -> int32
    OverloadFunction(Int32 * Int32);         // int32 * int32 -> int32
    OverloadFunction(Int32 - Int32);         // int32 - int32 -> int32
    OverloadFunction(Int32 / Int32);         // int32 / int32 -> int32

    // uint32 -> uint
    OverloadFunction(UInt32 + UInt32);       // uint32 + uint32 -> uint32
    OverloadFunction(UInt32 * UInt32);       // uint32 * uint32 -> uint32
    OverloadFunction(UInt32 - UInt32);       // uint32 - uint32 -> uint32
    OverloadFunction(UInt32 / UInt32);       // uint32 / uint32 -> uint32

    // int64 -> int64
    OverloadFunction(Int64 + Int64);         // int64 + int64 -> int64
    OverloadFunction(Int64 * Int64);         // int64 * int64 -> int64
    OverloadFunction(Int64 - Int64);         // int64 - int64 -> int64
    OverloadFunction(Int64 / Int64);         // int64 / int64 -> int64

    // uint64 -> uint64
    OverloadFunction(UInt64 + UInt64);       // uint64 + uint64 -> uint64
    OverloadFunction(UInt64 * UInt64);       // uint64 * uint64 -> uint64
    OverloadFunction(UInt64 - UInt64);       // uint64 - uint64 -> uint64
    OverloadFunction(UInt64 / UInt64);       // uint64 / uint64 -> uint64

    // float16 -> float
    OverloadFunction(Float16 + Float16);     // float16 + float16 -> float16
    OverloadFunction(Float16 * Float16);     // float16 * float16 -> float16
    OverloadFunction(Float16 - Float16);     // float16 - float16 -> float16
    OverloadFunction(Float16 / Float16);     // float16 / float16 -> float16

    // float32 -> float
    OverloadFunction(Float32 + Float32);     // float32 + float32 -> float32
    OverloadFunction(Float32 * Float32);     // float32 * float32 -> float32
    OverloadFunction(Float32 - Float32);     // float32 - float32 -> float32
    OverloadFunction(Float32 / Float32);     // float32 / float32 -> float32

    // float64 -> double
    OverloadFunction(Float64 + Float64);     // float64 + float64 -> double
    OverloadFunction(Float64 * Float64);     // float64 * float64 -> double
    OverloadFunction(Float64 - Float64);     // float64 - float64 -> double
    OverloadFunction(Float64 / Float64);     // float64 / float64 -> double

    // mixed scalar type promotion
    OverloadFunction(Int8 + Int16);          // int8 + int16 -> int
    OverloadFunction(Int8 + Int32);          // int8 + int32 -> int
    OverloadFunction(Int16 + Int32);         // int16 + int32 -> int
    OverloadFunction(UInt8 + UInt16);        // uint8 + uint16 -> uint
    OverloadFunction(UInt8 + UInt32);        // uint8 + uint32 -> uint
    OverloadFunction(UInt16 + UInt32);       // uint16 + uint32 -> uint
    OverloadFunction(Float16 + Float32);     // float16 + float32 -> float
    OverloadFunction(Float16 + Float64);     // float16 + float64 -> double
    OverloadFunction(Float32 + Float64);     // float32 + float64 -> double

    // vector type promotion
    ivec2 int2 = ivec2(1, 2);
    ivec3 int3 = ivec3(1, 2, 3);
    ivec4 int4 = ivec4(1, 2, 3, 4);

    OverloadFunction(int2 + int2);           // ivec2 + ivec2 -> ivec2
    OverloadFunction(int2 * int2);           // ivec2 * ivec2 -> ivec2
    OverloadFunction(int3 + int3);           // ivec3 + ivec3 -> ivec3
    OverloadFunction(int3 * int3);           // ivec3 * ivec3 -> ivec3
    OverloadFunction(int4 + int4);           // ivec4 + ivec4 -> ivec4
    OverloadFunction(int4 * int4);           // ivec4 * ivec4 -> ivec4

    // float vector type promotion
    vec2 float2 = vec2(1.0f, 2.0f);
    vec3 float3 = vec3(1.0f, 2.0f, 3.0f);
    vec4 float4 = vec4(1.0f, 2.0f, 3.0f, 4.0f);

    OverloadFunction(float2 + float2);       // vec2 + vec2 -> vec2
    OverloadFunction(float2 * float2);       // vec2 * vec2 -> vec2
    OverloadFunction(float3 + float3);       // vec3 + vec3 -> vec3
    OverloadFunction(float3 * float3);       // vec3 * vec3 -> vec3
    OverloadFunction(float4 + float4);       // vec4 + vec4 -> vec4
    OverloadFunction(float4 * float4);       // vec4 * vec4 -> vec4

    // double vector type promotion
    dvec2 double2 = dvec2(1.0lf, 2.0lf);
    dvec3 double3 = dvec3(1.0lf, 2.0lf, 3.0lf);
    dvec4 double4 = dvec4(1.0lf, 2.0lf, 3.0lf, 4.0lf);

    OverloadFunction(double2 + double2);     // dvec2 + dvec2 -> dvec2
    OverloadFunction(double2 * double2);     // dvec2 * dvec2 -> dvec2
    OverloadFunction(double3 + double3);     // dvec3 + dvec3 -> dvec3
    OverloadFunction(double3 * double3);     // dvec3 * dvec3 -> dvec3
    OverloadFunction(double4 + double4);     // dvec4 + dvec4 -> dvec4
    OverloadFunction(double4 * double4);     // dvec4 * dvec4 -> dvec4

    // vector component scalar type promotion
    OverloadFunction((int2 + int2).x);       // ivec2 + ivec2 -> ivec2, .x -> int
    OverloadFunction((float2 + float2).x);   // vec2 + vec2 -> vec2, .x -> float
    OverloadFunction((double2 + double2).x); // dvec2 + dvec2 -> dvec2, .x -> double

    // test expression overloads
    OverloadFunction(1.0f + 1 * 1.0lf);
    OverloadFunction(-1.0f);
    OverloadFunction(float(1 + 2) * 3);
    OverloadFunction((double)(1 + 2));

    // test scalar expression overloads
    float  Scalar1 = 1.0f;
    double Scalar2 = 1.0lf;
    int    Scalar3 = 1;

    OverloadFunction(Scalar1 + Scalar1);     // float + float -> float
    OverloadFunction(Scalar1 - Scalar1);     // float - float -> float
    OverloadFunction(Scalar1 * Scalar1);     // float * float -> float
    OverloadFunction(Scalar1 / Scalar1);     // float / float -> float
    OverloadFunction(Scalar2 + Scalar2);     // double + double -> double
    OverloadFunction(Scalar2 - Scalar2);     // double - double -> double
    OverloadFunction(Scalar2 * Scalar2);     // double * double -> double
    OverloadFunction(Scalar2 / Scalar2);     // double / double -> double
    OverloadFunction(Scalar3 + Scalar3);     // int + int -> int
    OverloadFunction(Scalar3 - Scalar3);     // int - int -> int
    OverloadFunction(Scalar3 * Scalar3);     // int * int -> int
    OverloadFunction(Scalar3 / Scalar3);     // int / int -> int

    // test vec2 expression overloads
    OverloadFunction(float2 + float2);       // vec2 + vec2 -> vec2
    OverloadFunction(float2 - float2);       // vec2 - vec2 -> vec2
    OverloadFunction(float2 * float2);       // vec2 * vec2 (component-wise) -> vec2
    OverloadFunction(float2 / float2);       // vec2 / vec2 (component-wise) -> vec2
    OverloadFunction(float2 * Scalar1);      // vec2 * scalar -> vec2
    OverloadFunction(Scalar1 * float2);      // scalar * vec2 -> vec2
    OverloadFunction(float2 / Scalar1);      // vec2 / scalar -> vec2
    OverloadFunction(float2 + Scalar1);      // vec2 + scalar -> vec2
    OverloadFunction(float2 - Scalar1);      // vec2 - scalar -> vec2

    // test vec3 expression overloads
    OverloadFunction(float3 + float3);       // vec3 + vec3 -> vec3
    OverloadFunction(float3 - float3);       // vec3 - vec3 -> vec3
    OverloadFunction(float3 * float3);       // vec3 * vec3 (component-wise) -> vec3
    OverloadFunction(float3 / float3);       // vec3 / vec3 (component-wise) -> vec3
    OverloadFunction(float3 * Scalar1);      // vec3 * scalar -> vec3
    OverloadFunction(Scalar1 * float3);      // scalar * vec3 -> vec3
    OverloadFunction(float3 / Scalar1);      // vec3 / scalar -> vec3
    OverloadFunction(float3 + Scalar1);      // vec3 + scalar -> vec3
    OverloadFunction(float3 - Scalar1);      // vec3 - scalar -> vec3

    // test vec4 expression overloads
    OverloadFunction(float4 + float4);       // vec4 + vec4 -> vec4
    OverloadFunction(float4 - float4);       // vec4 - vec4 -> vec4
    OverloadFunction(float4 * float4);       // vec4 * vec4 (component-wise) -> vec4
    OverloadFunction(float4 / float4);       // vec4 / vec4 (component-wise) -> vec4
    OverloadFunction(float4 * Scalar1);      // vec4 * scalar -> vec4
    OverloadFunction(Scalar1 * float4);      // scalar * vec4 -> vec4
    OverloadFunction(float4 / Scalar1);      // vec4 / scalar -> vec4
    OverloadFunction(float4 + Scalar1);      // vec4 + scalar -> vec4
    OverloadFunction(float4 - Scalar1);      // vec4 - scalar -> vec4

    // test dvec expression overloads
    OverloadFunction(double2 + double2);     // dvec2 + dvec2 -> dvec2
    OverloadFunction(double2 * Scalar2);     // dvec2 * double -> dvec2
    OverloadFunction(double3 - double3);     // dvec3 - dvec3 -> dvec3
    OverloadFunction(double3 * Scalar2);     // dvec3 * double -> dvec3
    OverloadFunction(double4 / double4);     // dvec4 / dvec4 -> dvec4
    OverloadFunction(double4 * Scalar2);     // dvec4 * double -> dvec4

    // test ivec expression overloads
    OverloadFunction(int2 + int2);           // ivec2 + ivec2 -> ivec2
    OverloadFunction(int2 * Scalar3);        // ivec2 * int -> ivec2
    OverloadFunction(int3 - int3);           // ivec3 - ivec3 -> ivec3
    OverloadFunction(int3 * Scalar3);        // ivec3 * int -> ivec3
    OverloadFunction(int4 + int4);           // ivec4 + ivec4 -> ivec4
    OverloadFunction(int4 / Scalar3);        // ivec4 / int -> ivec4

    // test square matrix expression overloads
    mat2 float2x2 = mat2(1.0f);
    mat3 float3x3 = mat3(1.0f);
    mat4 float4x4 = mat4(1.0f);

    OverloadFunction(float2x2 + float2x2);    // mat2 + mat2 -> mat2
    OverloadFunction(float2x2 - float2x2);    // mat2 - mat2 -> mat2
    OverloadFunction(float2x2 * float2x2);    // mat2 * mat2 -> mat2
    OverloadFunction(float2x2 * Scalar1);     // mat2 * scalar -> mat2
    OverloadFunction(Scalar1 * float2x2);     // scalar * mat2 -> mat2
    OverloadFunction(float2x2 * float2);      // mat2 * vec2 -> vec2
    OverloadFunction(float2 * float2x2);      // vec2 * mat2 -> vec2

    OverloadFunction(float3x3 + float3x3);    // mat3 + mat3 -> mat3
    OverloadFunction(float3x3 - float3x3);    // mat3 - mat3 -> mat3
    OverloadFunction(float3x3 * float3x3);    // mat3 * mat3 -> mat3
    OverloadFunction(float3x3 * Scalar1);     // mat3 * scalar -> mat3
    OverloadFunction(Scalar1 * float3x3);     // scalar * mat3 -> mat3
    OverloadFunction(float3x3 * float3);      // mat3 * vec3 -> vec3
    OverloadFunction(float3 * float3x3);      // vec3 * mat3 -> vec3

    OverloadFunction(float4x4 + float4x4);    // mat4 + mat4 -> mat4
    OverloadFunction(float4x4 - float4x4);    // mat4 - mat4 -> mat4
    OverloadFunction(float4x4 * float4x4);    // mat4 * mat4 -> mat4
    OverloadFunction(float4x4 * Scalar1);     // mat4 * scalar -> mat4
    OverloadFunction(Scalar1 * float4x4);     // scalar * mat4 -> mat4
    OverloadFunction(float4x4 * float4);      // mat4 * vec4 -> vec4
    OverloadFunction(float4 * float4x4);      // vec4 * mat4 -> vec4

    // test non-square matrix expression overloads
    mat2x3 float2x3 = mat2x3(1.0f);           // 2 columns, 3 rows
    mat3x2 float3x2 = mat3x2(1.0f);           // 3 columns, 2 rows
    mat2x4 float2x4 = mat2x4(1.0f);           // 2 columns, 4 rows
    mat4x2 float4x2 = mat4x2(1.0f);           // 4 columns, 2 rows
    mat3x4 float3x4 = mat3x4(1.0f);           // 3 columns, 4 rows
    mat4x3 float4x3 = mat4x3(1.0f);           // 4 columns, 3 rows

    OverloadFunction(float2x3 + float2x3);    // mat2x3 + mat2x3 -> mat2x3
    OverloadFunction(float2x3 - float2x3);    // mat2x3 - mat2x3 -> mat2x3
    OverloadFunction(float2x3 * Scalar1);     // mat2x3 * scalar -> mat2x3
    OverloadFunction(Scalar1 * float2x3);     // scalar * mat2x3 -> mat2x3
    OverloadFunction(float2x3 * float2);      // mat2x3 * vec2 -> vec3
    OverloadFunction(float3 * float2x3);      // vec3 * mat2x3 -> vec2

    OverloadFunction(float3x2 + float3x2);    // mat3x2 + mat3x2 -> mat3x2
    OverloadFunction(float3x2 - float3x2);    // mat3x2 - mat3x2 -> mat3x2
    OverloadFunction(float3x2 * Scalar1);     // mat3x2 * scalar -> mat3x2
    OverloadFunction(Scalar1 * float3x2);     // scalar * mat3x2 -> mat3x2
    OverloadFunction(float3x2 * float3);      // mat3x2 * vec3 -> vec2
    OverloadFunction(float2 * float3x2);      // vec2 * mat3x2 -> vec3

    OverloadFunction(float2x4 + float2x4);    // mat2x4 + mat2x4 -> mat2x4
    OverloadFunction(float2x4 - float2x4);    // mat2x4 - mat2x4 -> mat2x4
    OverloadFunction(float2x4 * Scalar1);     // mat2x4 * scalar -> mat2x4
    OverloadFunction(Scalar1 * float2x4);     // scalar * mat2x4 -> mat2x4
    OverloadFunction(float2x4 * float2);      // mat2x4 * vec2 -> vec4
    OverloadFunction(float4 * float2x4);      // vec4 * mat2x4 -> vec2

    OverloadFunction(float4x2 + float4x2);    // mat4x2 + mat4x2 -> mat4x2
    OverloadFunction(float4x2 - float4x2);    // mat4x2 - mat4x2 -> mat4x2
    OverloadFunction(float4x2 * Scalar1);     // mat4x2 * scalar -> mat4x2
    OverloadFunction(Scalar1 * float4x2);     // scalar * mat4x2 -> mat4x2
    OverloadFunction(float4x2 * float4);      // mat4x2 * vec4 -> vec2
    OverloadFunction(float2 * float4x2);      // vec2 * mat4x2 -> vec4

    OverloadFunction(float3x4 + float3x4);    // mat3x4 + mat3x4 -> mat3x4
    OverloadFunction(float3x4 - float3x4);    // mat3x4 - mat3x4 -> mat3x4
    OverloadFunction(float3x4 * Scalar1);     // mat3x4 * scalar -> mat3x4
    OverloadFunction(Scalar1 * float3x4);     // scalar * mat3x4 -> mat3x4
    OverloadFunction(float3x4 * float3);      // mat3x4 * vec3 -> vec4
    OverloadFunction(float4 * float3x4);      // vec4 * mat3x4 -> vec3

    OverloadFunction(float4x3 + float4x3);    // mat4x3 + mat4x3 -> mat4x3
    OverloadFunction(float4x3 - float4x3);    // mat4x3 - mat4x3 -> mat4x3
    OverloadFunction(float4x3 * Scalar1);     // mat4x3 * scalar -> mat4x3
    OverloadFunction(Scalar1 * float4x3);     // scalar * mat4x3 -> mat4x3
    OverloadFunction(float4x3 * float4);      // mat4x3 * vec4 -> vec3
    OverloadFunction(float3 * float4x3);      // vec3 * mat4x3 -> vec4

    // test non-square matrix * matrix overloads
    OverloadFunction(float2x3 * float3x2);     // mat2x3(2col,3row) * mat3x2(3col,2row) -> mat3x3 = mat3
    OverloadFunction(float3x2 * float2x3);     // mat3x2(3col,2row) * mat2x3(2col,3row) -> mat2x2 = mat2
    OverloadFunction(float2x4 * float4x2);     // mat2x4(2col,4row) * mat4x2(4col,2row) -> mat4x4 = mat4
    OverloadFunction(float4x2 * float2x4);     // mat4x2(4col,2row) * mat2x4(2col,4row) -> mat2x2 = mat2
    OverloadFunction(float3x4 * float4x3);     // mat3x4(3col,4row) * mat4x3(4col,3row) -> mat4x4 = mat4
    OverloadFunction(float4x3 * float3x4);     // mat4x3(4col,3row) * mat3x4(3col,4row) -> mat3x3 = mat3
    OverloadFunction(float3x4 * float2x3);     // mat3x4(3col,4row) * mat2x3(2col,3row) -> mat2x4
    OverloadFunction(float3x2 * float4x3);     // mat3x2(3col,2row) * mat4x3(4col,3row) -> mat4x2
    OverloadFunction(float4x2 * float3x4);     // mat4x2(4col,2row) * mat3x4(3col,4row) -> mat3x2
    OverloadFunction(float4x3 * float2x4);     // mat4x3(4col,3row) * mat2x4(2col,4row) -> mat2x3

    // test compound matrix/vector/scalar expression chains
    OverloadFunction((float2 + float2) * Scalar1 - float2 / Scalar1);   // vec2
    OverloadFunction((float3 - float3) * Scalar1 + float3 / Scalar1);   // vec3
    OverloadFunction((float4 * Scalar1) - (float4 / Scalar1) + float4); // vec4
    OverloadFunction(float4x4 * (float4 + float4));                     // mat4 * vec4 -> vec4
    OverloadFunction((float3x3 * float3x3) * float3);                   // (mat3 * mat3) * vec3 -> vec3
    OverloadFunction(float2x3 * (float2 + float2));                     // mat2x3 * (vec2 + vec2) -> vec3
    OverloadFunction((float3x4 * float4x3) * float4);                   // mat4x4 * vec4 -> vec4

    // test non-literal overloads
    OverloadFunction(Int16, Int32, Int64, Float32);
    OverloadFunction(UInt16, UInt32, UInt64, Float16);

    // test bits upgrade
    OverloadFunction(Int8);
    OverloadFunction(UInt8);

    // test ambiguous overloads
    OverloadFunction(Int16, Int8, Int8, Float16);
    OverloadFunction(UInt16, UInt8, UInt8, Float32);

    // test swizzle overloads
    mat4 Matrix = mat4(
        vec4(1.0f,  2.0f,  3.0f,  4.0f),
        vec4(5.0f,  6.0f,  7.0f,  8.0f),
        vec4(9.0f,  10.0f, 11.0f, 12.0f),
        vec4(13.0f, 14.0f, 15.0f, 16.0f)
    );

    vec4 Vector = Matrixv[0].xyzw;
    float Component1 = Vector.x;
    float Component2 = Matrix[1].xyzw.x;

    OverloadFunction(Component1);
    OverloadFunction(Component2);

    // --- Super complex compound expressions ---

    // Function call + arithmetic operations + swizzle
    OverloadFunction(GetFloat() + GetFloat() * GetFloat());
    OverloadFunction(GetDouble() - GetDouble() / GetDouble());
    OverloadFunction(GetInt() * GetInt() + GetInt());

    // Function call + vector swizzle + arithmetic
    OverloadFunction((GetVec2() + GetVec2()).xy);
    OverloadFunction((GetVec3() * GetVec3()).xyz);
    OverloadFunction((GetVec4() - GetVec4()).xyzw);
    OverloadFunction((GetDVec2() / GetDVec2()).yx);
    OverloadFunction((GetIVec3() + GetIVec3()).zyx);
    OverloadFunction((GetIVec4() * GetIVec4()).wwzz);

    // Nested function calls with arithmetic
    OverloadFunction(GetVec2() + GetVec2() * GetFloat());
    OverloadFunction(GetVec3() * GetFloat() - GetVec3() / GetFloat());
    OverloadFunction(GetFloat() * GetVec4() + GetVec4() * GetFloat());
    OverloadFunction(GetDVec3() + GetDouble() * GetDVec3());
    OverloadFunction(GetIVec2() * GetInt() + GetIVec2() / GetInt());

    // Array-like access + swizzle + arithmetic
    uint  uintArray1D[2]    = uint[2](1u, 2u);
    uint  uintArray1D_4[4]  = uint[4](1u, 2u, 3u, 4u);
    vec2  vec2Array1D_3[3]  = vec2[](vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f));
    vec3  vec3Array1D_2[2]  = vec3[](vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f));
    vec4  vec4Array2D_2[2]  = vec4[](vec4(1.0f, 2.0f, 3.0f, 4.0f), vec4(5.0f, 6.0f, 7.0f, 8.0f));
    dvec2 devc2Array1D_2[2] = dvec2[](dvec2(1.0lf, 2.0lf), dvec2(3.0lf, 4.0lf));
    ivec3 ivec3Array1D_2[2] = ivec3[](ivec3(1, 2, 3), ivec3(4, 5, 6));
    mat3  mat3Array1D_2[2]  = mat3[](mat3(1.0f), mat3(2.0f));
    mat4  mat4Array1D_2[2]  = mat4[](mat4(1.0f), mat4(2.0f));

    // GL_NV_explicit_typecast: scalar -> scalar
    OverloadFunction((bool)Scalar1);
    OverloadFunction((int)Scalar1);
    OverloadFunction((uint)Scalar3);
    OverloadFunction((float)Scalar2);
    OverloadFunction((double)Scalar3);
    OverloadFunction((int8_t)Scalar2);
    OverloadFunction((uint16_t)Scalar1);
    OverloadFunction((float64_t)Scalar3);

    // GL_NV_explicit_typecast: scalar -> vector
    OverloadFunction((vec2)Scalar1);
    OverloadFunction((vec3)Scalar2);
    OverloadFunction((vec4)Scalar3);
    OverloadFunction((dvec4)Scalar1);
    OverloadFunction((ivec3)Scalar2);
    OverloadFunction((uvec2)Scalar3);
    OverloadFunction((bvec4)Scalar1);

    // GL_NV_explicit_typecast: scalar -> square/non-square matrix
    OverloadFunction((mat2)Scalar1);
    OverloadFunction((mat3)Scalar2);
    OverloadFunction((mat4)Scalar3);
    OverloadFunction((mat2x3)Scalar1);
    OverloadFunction((mat3x4)Scalar2);
    OverloadFunction((mat4x2)Scalar3);

    // GL_NV_explicit_typecast: vector -> vector, component count may shrink
    OverloadFunction((vec2)float2);
    OverloadFunction((vec2)float3);
    OverloadFunction((vec2)float4);
    OverloadFunction((vec3)float3);
    OverloadFunction((vec3)float4);
    OverloadFunction((ivec2)float4);
    OverloadFunction((dvec3)int4);
    OverloadFunction((bvec2)double4);

    // GL_NV_explicit_typecast: vec4 -> mat2 is the only vector -> matrix case
    OverloadFunction((mat2)float4);
    OverloadFunction((mat2)int4);
    OverloadFunction((mat2)double4);

    // GL_NV_explicit_typecast: matrix -> any matrix
    OverloadFunction((mat2)float4x4);
    OverloadFunction((mat3)float2x2);
    OverloadFunction((mat4)float3x3);
    OverloadFunction((mat2x3)float4x2);
    OverloadFunction((mat3x4)float2x3);
    OverloadFunction((mat4x2)float3x4);

    // Nested casts, right associativity and operator precedence
    OverloadFunction((int)(float)(double)Scalar1);
    OverloadFunction((vec2)(vec3)(vec4)Scalar1);
    OverloadFunction((int)-Scalar1);
    OverloadFunction((int)+Scalar1);
    OverloadFunction(-(int)Scalar1);
    OverloadFunction(+(int)Scalar1);
    OverloadFunction((int)Scalar1 * Scalar3);
    OverloadFunction((int)(Scalar1 * Scalar2));
    OverloadFunction((float)float4.x);
    OverloadFunction((vec2)GetVec4().xy);
    OverloadFunction(((vec4)Scalar1).xyz);
    OverloadFunction(((mat2)float4)[0]);

    // The cast has exactly one operand. These commas form a sequence expression;
    // the final scalar value is cast to the target vector/matrix.
    OverloadFunction((vec3)(1.0f, 2.0f, 3.0f));
    OverloadFunction((mat2)(Scalar1, Scalar2, Scalar3));

    // Buffer reference conversions from GL_EXT_buffer_reference(_uvec2)
    _Buffer      BufferFromUInt64 = (_Buffer)UInt64;
    _Buffer      BufferFromUVec2  = (_Buffer)uvec2(0u, 0u);
    uint64_t     UInt64FromBuffer = (uint64_t)BufferFromUInt64;
    uvec2        UVec2FromBuffer  = (uvec2)BufferFromUInt64;
    _OtherBuffer OtherFromBuffer  = (_OtherBuffer)BufferFromUInt64;

    // Invalid casts required by the negative side of the extension rules.
    // TypeResolver should leave these expressions as unknown/error types.
    vec4 InvalidVectorGrowth   = (vec4)float2;          // vector component count increases
    int  InvalidVectorScalar   = (int)float4;           // vector -> scalar
    mat2 InvalidVec3Matrix     = (mat2)float3;          // only four-component vector -> mat2
    vec4 InvalidMatrixVector   = (vec4)float4x4;        // matrix -> vector
    float InvalidMatrixScalar  = (float)float2x2;       // matrix -> scalar
    int  InvalidArrayScalar    = (int)uintArray1D;      // array casts are unsupported

    InnerData InnerValue;
    OuterData OuterValue;
    InnerData InvalidStructSource = (InnerData)OuterValue;
    InnerData InvalidStructTarget = (InnerData)Scalar3;

    // also test overloads that accept whole arrays
    OverloadFunction(vec2Array1D_3);
    OverloadFunction(vec3Array1D_2);
    OverloadFunction(vec4Array2D_2);
    OverloadFunction(devc2Array1D_2);
    OverloadFunction(ivec3Array1D_2);
    OverloadFunction(mat3Array1D_2);
    OverloadFunction(uintArray1D);
    OverloadFunction(uintArray1D_4);
    OverloadFunction(mat4Array1D_2);

    OverloadFunction(vec2Array1D_3[0] + vec2Array1D_3[1]);
    OverloadFunction(vec3Array1D_2[0] - vec3Array1D_2[1]);
    OverloadFunction(vec4Array2D_2[0] * vec4Array2D_2[1]);
    OverloadFunction(devc2Array1D_2[0] / devc2Array1D_2[1]);
    OverloadFunction(ivec3Array1D_2[0] + ivec3Array1D_2[1]);
    OverloadFunction(mat3Array1D_2[0] * mat3Array1D_2[1]);

    // Array access with swizzle
    OverloadFunction((vec2Array1D_3[0] + vec2Array1D_3[2]).xy);
    OverloadFunction((vec3Array1D_2[1] * vec3Array1D_2[0]).zyx);
    OverloadFunction((vec4Array2D_2[0] - vec4Array2D_2[1]).xyzw);
    OverloadFunction((devc2Array1D_2[0] + devc2Array1D_2[1]).yx);
    OverloadFunction((ivec3Array1D_2[1] / ivec3Array1D_2[0]).xyz);

    // Constructor calls with nested expressions
    OverloadFunction(vec2(GetFloat() + GetFloat(), GetFloat() * GetFloat()));
    OverloadFunction(vec3(GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat()));
    OverloadFunction(vec4(GetFloat() + GetFloat(), GetFloat() - GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat()));
    OverloadFunction(dvec2(GetDouble() + GetDouble(), GetDouble() * GetDouble()));
    OverloadFunction(dvec3(GetDouble(), GetDouble() / GetDouble(), GetDouble() - GetDouble()));
    OverloadFunction(ivec2(GetInt() + GetInt(), GetInt() * GetInt()));
    OverloadFunction(ivec4(GetInt(), GetInt() - GetInt(), GetInt() * GetInt(), GetInt() / GetInt()));

    // Constructor calls combining function results
    OverloadFunction(vec2(GetVec2().x, GetVec2().y));
    OverloadFunction(vec3(GetVec3().x, GetVec3().y, GetVec3().z));
    OverloadFunction(vec4(GetVec4().x, GetVec4().y, GetVec4().z, GetVec4().w));
    OverloadFunction(dvec2(GetDVec2().x, GetDVec2().y));
    OverloadFunction(ivec3(GetIVec3().x, GetIVec3().y, GetIVec3().z));

    // Nested constructor + array access + arithmetic
    OverloadFunction(vec2[](vec2(1.0f, 2.0f), vec2(3.0f, 4.0f))[0] + vec2[](vec2(5.0f, 6.0f), vec2(7.0f, 8.0f))[1]);
    OverloadFunction(vec3[](vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f))[1] - vec3[](vec3(7.0f, 8.0f, 9.0f), vec3(10.0f, 11.0f, 12.0f))[0]);
    OverloadFunction(vec4[](vec4(1.0f), vec4(2.0f))[0] * vec4[](vec4(3.0f), vec4(4.0f))[1]);

    // Matrix operations with multiple levels of calls/access
    OverloadFunction(GetMat2x2() * GetMat2x2());
    OverloadFunction(GetMat3x3() + GetMat3x3());
    OverloadFunction(GetMat4x4() - GetMat4x4());
    OverloadFunction(GetMat2x2() * GetVec2());
    OverloadFunction(GetVec3() * GetMat3x3());
    OverloadFunction(GetMat4x4() * GetVec4());

    // Non-square matrix operations
    OverloadFunction(GetMat2x3() + GetMat2x3());
    OverloadFunction(GetMat3x2() * GetMat2x3());
    OverloadFunction(GetMat2x4() * GetVec2());
    OverloadFunction(GetVec4() * GetMat2x4());
    OverloadFunction(GetMat3x4() * GetMat4x3());

    // Complex chained expressions with multiple operations
    OverloadFunction(GetVec2() + GetVec2() * GetFloat() - GetVec2() / GetFloat());
    OverloadFunction(GetVec3() * GetFloat() + GetVec3() / GetFloat() - GetVec3() * GetFloat());
    OverloadFunction(GetVec4() - GetVec4() * GetFloat() + GetVec4() / GetFloat() * GetFloat());
    OverloadFunction(GetDVec2() / GetDVec2() * GetDouble() + GetDVec2() - GetDVec2() * GetDouble());
    OverloadFunction(GetIVec3() + GetIVec3() * GetInt() - GetIVec3() / GetInt() + GetIVec3());

    // Array access + function call + swizzle + arithmetic in chains
    OverloadFunction((vec2Array1D_3[0] + GetVec2()).xy);
    OverloadFunction((vec3Array1D_2[1] * GetVec3()).zyx);
    OverloadFunction((vec4Array2D_2[0] - GetVec4()).xyzw);
    OverloadFunction((devc2Array1D_2[0] / GetDVec2()).yx);
    OverloadFunction((ivec3Array1D_2[1] + GetIVec3()).xyz);

    // Constructor with array element and arithmetic
    OverloadFunction(vec2(vec2Array1D_3[0].x + vec2Array1D_3[1].x, vec2Array1D_3[0].y * vec2Array1D_3[1].y));
    OverloadFunction(vec3(vec3Array1D_2[0].x + GetFloat(), vec3Array1D_2[1].y * GetFloat(), vec3Array1D_2[0].z / GetFloat()));
    OverloadFunction(ivec2(ivec3Array1D_2[0].x * GetInt(), ivec3Array1D_2[1].y + GetInt()));

    // Matrix array access with arithmetic
    OverloadFunction(mat3Array1D_2[0] * mat3Array1D_2[1]);
    OverloadFunction(mat3Array1D_2[0] + mat3Array1D_2[1] * GetFloat());
    OverloadFunction(mat3Array1D_2[1] - mat3Array1D_2[0] / GetFloat());
    OverloadFunction(mat3Array1D_2[0] * GetVec3());
    OverloadFunction(GetVec3() * mat3Array1D_2[1]);

    // Deeply nested expressions
    OverloadFunction((GetVec2() + (GetVec2() * GetFloat())) / (GetVec2() - GetFloat()));
    OverloadFunction((GetVec3() * GetFloat() + GetVec3()) - (GetVec3() / GetFloat() * GetVec3()));
    OverloadFunction((GetVec4() + GetVec4()) * (GetFloat() + GetFloat()) - (GetVec4() - GetVec4()));
    OverloadFunction((GetDVec2() * GetDouble() + GetDVec2()) / (GetDVec2() + GetDouble()));
    OverloadFunction((GetIVec3() + GetIVec3() * GetInt()) - (GetIVec3() - GetInt()));

    // Expressions with member access chains
    OverloadFunction(vec2(vec2Array1D_3[0].x, vec2Array1D_3[1].x));
    OverloadFunction(vec3(vec3Array1D_2[0].x, vec3Array1D_2[0].y, vec3Array1D_2[1].z));
    OverloadFunction(vec4(vec4Array2D_2[0].x, vec4Array2D_2[0].y, vec4Array2D_2[1].z, vec4Array2D_2[1].w));
    OverloadFunction(dvec2(devc2Array1D_2[0].x, devc2Array1D_2[1].y));
    OverloadFunction(ivec3(ivec3Array1D_2[0].x, ivec3Array1D_2[1].y, ivec3Array1D_2[0].z));

    // Constructor with mixed function calls and array access
    OverloadFunction(vec2(vec2Array1D_3[0] + GetVec2()));
    OverloadFunction(vec3(vec3Array1D_2[1] * GetVec3()));
    OverloadFunction(vec4(vec4Array2D_2[0] - GetVec4()));
    OverloadFunction(dvec2(devc2Array1D_2[0] / GetDVec2()));
    OverloadFunction(ivec3(ivec3Array1D_2[1] + GetIVec3()));

    // Swizzle on constructor results
    OverloadFunction((vec2(GetFloat() + GetFloat(), GetFloat() * GetFloat())).xy);
    OverloadFunction((vec3(GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat())).zyx);
    OverloadFunction((vec4(GetFloat() + GetFloat(), GetFloat() - GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat())).xyzw);

    // Matrix constructor with nested expressions
    OverloadFunction(mat2(vec2(GetFloat() + GetFloat(), GetFloat() * GetFloat()), vec2(GetFloat() - GetFloat(), GetFloat() / GetFloat())));
    OverloadFunction(mat3(GetFloat(), GetFloat() * GetFloat(), GetFloat() / GetFloat(), GetFloat() + GetFloat(), GetFloat() - GetFloat(), GetFloat() * GetFloat(), GetFloat(), GetFloat(), GetFloat()));
    OverloadFunction(mat4(vec4(GetFloat()), vec4(GetFloat() * GetFloat()), vec4(GetFloat() / GetFloat()), vec4(GetFloat() + GetFloat())));

    // Arithmetic on matrix constructor results
    OverloadFunction(mat2(vec2(GetFloat()), vec2(GetFloat())) + mat2(vec2(GetFloat()), vec2(GetFloat())));
    OverloadFunction(mat3(GetFloat()) * mat3(GetFloat() * GetFloat()));
    OverloadFunction(mat4(vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat())) - mat4(vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat()), vec4(GetFloat())));

    // Array of constructors with arithmetic
    OverloadFunction(vec2[](vec2(GetFloat()), vec2(GetFloat() * GetFloat()))[0] + vec2[](vec2(GetFloat() / GetFloat()), vec2(GetFloat() + GetFloat()))[1]);
    OverloadFunction(vec3[](vec3(GetFloat()), vec3(GetFloat() * GetFloat()))[1] - vec3[](vec3(GetFloat()), vec3(GetFloat()))[0]);
    OverloadFunction(vec4[](vec4(GetFloat() + GetFloat()), vec4(GetFloat() - GetFloat()))[0] * vec4[](vec4(GetFloat()), vec4(GetFloat()))[1]);

    // Super nested: array[function().member operation] + constructor
    OverloadFunction(vec2(vec2Array1D_3[0].x + GetVec2().x, vec2Array1D_3[1].y * GetVec2().y));
    OverloadFunction(vec3(vec3Array1D_2[0].x + GetVec3().x, vec3Array1D_2[1].y - GetVec3().y, vec3Array1D_2[0].z * GetVec3().z));
    OverloadFunction(dvec2(devc2Array1D_2[0].x * GetDVec2().x, devc2Array1D_2[1].y / GetDVec2().y));

    // Chain of swizzles and operations
    OverloadFunction(((GetVec4().xy + GetVec2()) * GetFloat()).xy);
    OverloadFunction(((GetVec3().zyx - GetVec3()) / GetFloat()).xyz);
    OverloadFunction(((GetVec2().yx + GetVec2().xy) * GetFloat()).xy);

    // --- Test array types with different dimensions and element types ---

    // 1D arrays
    int    intArray1D_2[2]    = int[2](1, 2);
    int    intArray1D_3[3]    = int[3](1, 2, 3);
    float  floatArray1D_5[5]  = float[5](1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
    double doubleArray1D_4[4] = double[4](1.0lf, 2.0lf, 3.0lf, 4.0lf);

    OverloadFunction(floatArray1D_5); // Test array parameter overload
    OverloadFunction(intArray1D_2); // Test array parameter overload (size 2)
    OverloadFunction(intArray1D_3); // Test array parameter overload (size 3)

    // 1D arrays of scalar types with different precision
    int8_t    int8Array1D_3[3]    = int8_t[3](int8_t(1), int8_t(2), int8_t(3));
    uint16_t  uint16Array1D_3[3]  = uint16_t[3](uint16_t(1), uint16_t(2), uint16_t(3));
    float32_t float32Array1D_3[3] = float32_t[3](1.0f, 2.0f, 3.0f);
    float64_t float64Array1D_3[3] = float64_t[3](1.0lf, 2.0lf, 3.0lf);

    OverloadFunction(int8Array1D_3[0]);
    OverloadFunction(uint16Array1D_3[1]);

    // 1D arrays of vectors
    vec2 vec2Array1D_4[4] = vec2[4](vec2(1.0f, 2.0f), vec2(3.0f, 4.0f), vec2(5.0f, 6.0f), vec2(7.0f, 8.0f));
    vec3 vec3Array1D_3[3] = vec3[3](vec3(1.0f, 2.0f, 3.0f), vec3(4.0f, 5.0f, 6.0f), vec3(7.0f, 8.0f, 9.0f));

    OverloadFunction(vec2Array1D_4); // Test array parameter overload

    // 2D arrays
    int intArray2D_2x3[2][3] = int[2][3](
        int[3](1, 2, 3),
        int[3](4, 5, 6)
    );

    float floatArray2D_3x4[3][4] = float[3][4](
        float[4](1.0f, 2.0f, 3.0f, 4.0f),
        float[4](5.0f, 6.0f, 7.0f, 8.0f),
        float[4](9.0f, 10.0f, 11.0f, 12.0f)
    );

    OverloadFunction(intArray2D_2x3); // Test array parameter overload

    vec2 vec2Array2D_2x2[2][2] = vec2[2][2](
        vec2[2](vec2(1.0f), vec2(2.0f)),
        vec2[2](vec2(3.0f), vec2(4.0f))
    );

    vec3 vec3Array2D_2x2[2][2] = vec3[2][2](
        vec3[2](vec3(1.0f), vec3(2.0f)),
        vec3[2](vec3(3.0f), vec3(4.0f))
    );

    mat2 mat2Array2D_2x2[2][2] = mat2[2][2](
        mat2[2](mat2(1.0f), mat2(2.0f)),
        mat2[2](mat2(3.0f), mat2(4.0f))
    );

    OverloadFunction(vec2Array2D_2x2);
    OverloadFunction(vec3Array2D_2x2); // Test array parameter overload
    OverloadFunction(mat2Array2D_2x2); // Test array parameter overload

    // 3D arrays
    float floatArray3D_2x2x3[2][2][3] = float[2][2][3](
        float[2][3](
            float[3](1.0f, 2.0f, 3.0f),
            float[3](4.0f, 5.0f, 6.0f)
        ),
        float[2][3](
            float[3](7.0f, 8.0f, 9.0f),
            float[3](10.0f, 11.0f, 12.0f)
        )
    );

    vec2 vec2Array3D_2x2x2[2][2][2] = vec2[2][2][2](
        vec2[2][2](
            vec2[2](vec2(1.0f), vec2(2.0f)),
            vec2[2](vec2(3.0f), vec2(4.0f))
        ),
        vec2[2][2](
            vec2[2](vec2(5.0f), vec2(6.0f)),
            vec2[2](vec2(7.0f), vec2(8.0f))
        )
    );

    vec3 vec3Array3D_2x2x2[2][2][2] = vec3[2][2][2](
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
    OverloadFunction(floatArray3D_2x2x3[0][1][2]);
    OverloadFunction(vec2Array3D_2x2x2[1][0][0]);

    OverloadFunction(vec2Array3D_2x2x2[0][0][0]);
    OverloadFunction(vec2Array3D_2x2x2[1][0][0]);

    // 3D arrays with arithmetic operations
    OverloadFunction(vec2Array3D_2x2x2[0][0][0] + vec2Array3D_2x2x2[1][1][1]);
    OverloadFunction(vec3Array3D_2x2x2[0][1][0] * vec3Array3D_2x2x2[1][0][1]);

    // 3D arrays with complex indexing and operations
    OverloadFunction((vec2Array3D_2x2x2[0][1][1] + vec2Array3D_2x2x2[1][0][0]).xy);
    OverloadFunction((vec3Array3D_2x2x2[0][0][0] - vec3Array3D_2x2x2[1][1][1]).xyz);

    // Mixed operations: cross-dimensional access
    OverloadFunction(floatArray1D_5[0] + floatArray2D_3x4[0][0] + floatArray3D_2x2x3[0][0][0]);
    OverloadFunction(vec2Array1D_4[0] + vec2Array2D_2x2[0][0] + vec2Array3D_2x2x2[0][0][0]);

    // Array elements with constructor calls
    OverloadFunction(vec2(floatArray2D_3x4[0][0], floatArray2D_3x4[1][1]));
    OverloadFunction(vec3(floatArray3D_2x2x3[0][0][0], floatArray3D_2x2x3[1][0][1], floatArray3D_2x2x3[0][1][0]));
    OverloadFunction(ivec2(intArray2D_2x3[0][0], intArray2D_2x3[1][1]));

    // Array elements in complex expressions
    OverloadFunction(floatArray2D_3x4[0][0] * floatArray2D_3x4[1][2] + floatArray3D_2x2x3[1][1][2]);
    OverloadFunction(vec2Array2D_2x2[0][0] * vec2Array3D_2x2x2[1][0][1] + vec2Array1D_4[2]);

    OverloadFunction(intArray1D_2);                 // 变量
    OverloadFunction(int[2](3, 4));                 // 构造函数
    OverloadFunction(int[](5, 6));                  // 无维度构造

    OverloadFunction(intArray1D_3);
    OverloadFunction(int[3](4, 5, 6));
    OverloadFunction(int[](7, 8, 9));

    OverloadFunction(intArray2D_2x3);
    OverloadFunction(int[2][3](int[3](1, 1, 1), int[3](2, 2, 2)));
    OverloadFunction(int[][3](int[3](0, 0, 0), int[3](0, 0, 0)));

    OverloadFunction(uintArray1D_4);
    OverloadFunction(uint[2](10u, 20u));
    OverloadFunction(uint[](100u, 200u, 300u, 400u));

    uint16_t uint16Array1D_3[3] = uint16_t[3](1us, 2us, 3us);
    OverloadFunction(uint16Array1D_3);
    OverloadFunction(uint16_t[3](4us, 5us, 6us));
    OverloadFunction(uint16_t[](7us, 8us, 9us));

    OverloadFunction(intArray1D_2);
    OverloadFunction({30, 40});
    OverloadFunction({1, 2, 3});

    OverloadFunction({{1, 2, 3}, {4, 5, 6}}); 

    uint uintArray1D_4[4] = {1u, 2u, 3u, 4u};
    OverloadFunction(uintArray1D_4);
    OverloadFunction({10u, 20u});

    OverloadFunction({10us, 20us, 30us});

    OverloadFunction(floatArray1D_5);
    OverloadFunction(float[5](0.1, 0.2, 0.3, 0.4, 0.5));
    OverloadFunction(float[](1.1, 2.2, 3.3, 4.4, 5.5));

    float floatArray3D_2x2x3[2][2][3];
    OverloadFunction(floatArray3D_2x2x3);
    OverloadFunction(float[2][2][3](
        float[2][3](float[3](1, 1, 1), float[3](1, 1, 1)),
        float[2][3](float[3](2, 2, 2), float[3](2, 2, 2))
    ));
    OverloadFunction(float[][][](
        float[][](float[](0, 0, 0), float[](0, 0, 0)),
        float[][](float[](0, 0, 0), float[](0, 0, 0))
    ));

    // double[4]
    double doubleArray1D_4[4] = double[4](1.0LF, 2.0LF, 3.0LF, 4.0LF);
    OverloadFunction(doubleArray1D_4);
    OverloadFunction(double[4](0.0LF, 0.0LF, 0.0LF, 0.0LF));
    OverloadFunction(double[](9.0LF, 8.0LF, 7.0LF, 6.0LF));

    // float[5]
    OverloadFunction({1.1, 2.2, 3.3, 4.4, 5.5});

    // float[2][3] 与 float[3][4]
    float floatArray2D_2x3[2][3] = {{1., 2., 3.}, {4., 5., 6.}};
    OverloadFunction(floatArray2D_2x3);
    OverloadFunction({{0.,0.,0.,0.}, {1.,1.,1.,1.}, {2.,2.,2.,2.}}); // 对应 float[3][4]

    // float[2][2][3] (三维)
    OverloadFunction({
        {{1.,1.,1.}, {2.,2.,2.}}, 
        {{3.,3.,3.}, {4.,4.,4.}}
    });

    // double[4]
    OverloadFunction({1.0LF, 2.0LF, 3.0LF, 4.0LF});

    // vec2[3]
    OverloadFunction(vec2Array1D_3);
    OverloadFunction(vec2[3](vec2(1.1), vec2(2.2), vec2(3.3)));
    OverloadFunction(vec2[](vec2(0.5), vec2(0.6), vec2(0.7)));

    // vec2[2][2][2] (三维向量数组)
    OverloadFunction(vec2Array3D_2x2x2);
    OverloadFunction(vec2[2][2][2](
        vec2[2][2](vec2[2](vec2(1), vec2(1)), vec2[2](vec2(1), vec2(1))),
        vec2[2][2](vec2[2](vec2(1), vec2(1)), vec2[2](vec2(1), vec2(1)))
    ));

    // ivec3[2] / vec3[2] / vec4[2]
    OverloadFunction(vec3Array1D_2);
    OverloadFunction(ivec3[2](ivec3(1), ivec3(2)));
    OverloadFunction(vec4[](vec4(1), vec4(0)));

    // dvec2[2]
    dvec2 dvec2Array1D_2[2] = dvec2[2](dvec2(1.0LF), dvec2(2.0LF));
    OverloadFunction(dvec2Array1D_2);
    OverloadFunction(dvec2[2](dvec2(0), dvec2(1)));
    OverloadFunction(dvec2[](dvec2(5), dvec2(6)));

    // vec2[3] 与 vec2[4]
    OverloadFunction({vec2(0), vec2(1), vec2(2)});
    OverloadFunction(vec2Array1D_4);

    // vec2[2][2][2] (三维向量数组)
    OverloadFunction({
        {{vec2(0), vec2(0)}, {vec2(1), vec2(1)}},
        {{vec2(2), vec2(2)}, {vec2(3), vec2(3)}}
    });

    // ivec3[2], vec3[2], vec4[2], dvec2[2]
    OverloadFunction({ivec3(1), ivec3(2)});
    OverloadFunction({vec3(0.1), vec3(0.2)});
    OverloadFunction({vec4(0), vec4(1)});
    OverloadFunction({dvec2(1.0LF), dvec2(2.0LF)});

    // vec3[2][2]
    OverloadFunction({{vec3(0), vec3(0)}, {vec3(1), vec3(1)}});

    OverloadFunction(mat2Array2D_2x2);
    OverloadFunction(mat2[2][2](mat2[2](mat2(0), mat2(0)), mat2[2](mat2(0), mat2(0))));
    OverloadFunction(mat2[][](mat2[2](mat2(1), mat2(1)), mat2[2](mat2(1), mat2(1))));

    // mat3[2] / mat4[2]
    OverloadFunction(mat4Array1D_2);
    OverloadFunction(mat3[2](mat3(1), mat3(1)));
    OverloadFunction(mat4[](mat4(2), mat4(2)));

    // mat3[2] 与 mat4[2]
    OverloadFunction({mat3(1.0), mat3(1.0)});
    OverloadFunction(mat4Array1D_2);

    // mat2[2][2] (多维矩阵数组)
    OverloadFunction({
        {mat2(1), mat2(1)},
        {mat2(0), mat2(0)}
    });

    InnerData LocalInner;
    MiddleData LocalMiddle;
    OuterData LocalOuter;

    OverloadFunction(LocalInner);                                      // 匹配 InnerData
    OverloadFunction(LocalMiddle);                                     // 匹配 MiddleData
    OverloadFunction(LocalOuter);                                      // 匹配 OuterData

    OverloadFunction(LocalOuter.Middle[1]);                            // 匹配 MiddleData
    OverloadFunction(LocalOuter.Middle[0].Inner);                      // 匹配 InnerData
    OverloadFunction(LocalOuter.Middle[0].Inner.float3);               // 匹配 vec3
    OverloadFunction(LocalOuter.Middle[0].Inner.mat2Array1D_2[1]);     // 匹配 mat2
    OverloadFunction(LocalOuter.Middle[0].vec4Array2D_2[2]);           // 匹配 vec4
    OverloadFunction(LocalOuter.Middle[0].vec4Array2D_2[2].xyz);       // 匹配 vec3
    OverloadFunction(LocalOuter.Middle[1].float4x4[0]);                // 匹配 vec4 (mat4 的列是 vec4)
    OverloadFunction(LocalOuter.Middle[1].float4x4[0].x);              // 匹配 float32_t

    OverloadFunction(LocalOuter.Middle[0].Inner.mat2Array1D_2);        // 匹配 mat2[2]
    OverloadFunction(LocalOuter.Middle[0].Inner.intArray2D_2x3);       // 匹配 int[2][3]
    OverloadFunction(LocalOuter.Middle[0].Inner.intArray2D_2x3[0]);    // 匹配 int[3]
    OverloadFunction(LocalOuter.Middle[0].Inner.intArray2D_2x3[1][2]); // 匹配 int32_t

    // 运算 + 链式调用
    OverloadFunction(LocalOuter.Middle[0].Inner.float3 + vec3(1.0));                          // 匹配 vec3
    OverloadFunction(LocalOuter.Middle[1].float4x4 * LocalOuter.Middle[0].Inner.float3.xyzz); // 匹配 vec4
    OverloadFunction(dot(LocalOuter.Middle[0].Inner.vec3_field, vec3(0.5)));                  // 匹配 float32_t

    OverloadFunction(OuterData(), MiddleData());
    OverloadFunction(InnerData(), MiddleData(), OuterData());
    OverloadFunction(MiddleData(), InnerData());
    OverloadFunction(OuterData(), MiddleData(), InnerData());

    struct {
        int       Int;
        vec4      float4;
        mat4      float4x4;
        OuterData Outer;
    } MyStruct;

    OverloadFunction(MyStruct.Int);
    OverloadFunction(MyStruct.float4);

    OverloadFunction(MyStruct.float4x4[0]);
    OverloadFunction(MyStruct.float4x4[1]);

    OverloadFunction(MyStruct.Outer);
    OverloadFunction(MyStruct.Outer.Middle[0]);
    OverloadFunction(MyStruct.Outer.Middle[1].Inner);

    OverloadFunction(MyStruct.Outer.Middle[0].Inner.mat2Array1D_2[0][0].xxxx);
    OverloadFunction(mat3x4(MyStruct.Outer.Middle[1].Vec4Array2D_2));
    OverloadFunction(MyStruct.Outer.Middle[1].Inner.intArray2D_2x3[2][1] * MyStruct.Outer.double2.xx);
    OverloadFunction(MyStruct.Outer.Middle[0].Inner.float3 * MyStruct.Outer.Middle[1].Inner.float3);

    MySpirvTypeX typex;
    MySpirvType0 type0;
    MySpirvType1 type1;
    MySpirvType2 type2;
    MySpirvType3 type3;
    MySpirvType4 type4;
    MySpirvType5 type5;
    MySpirvType6 type6;
    MySpirvType7 type7;
    MySpirvType8 type8;

    OverloadFunction(typex);
    OverloadFunction(type0);
    OverloadFunction(type1);
    OverloadFunction(type2);
    OverloadFunction(type3);
    OverloadFunction(type4);
    OverloadFunction(type5);
    OverloadFunction(type6);
    OverloadFunction(type7);
    OverloadFunction(type8);
    OverloadFunction(type0, type1);
    OverloadFunction(type1, type0);

    COOPMAT_TYPE_16 CoopMat16;
    COOPMAT_TYPE_32 CoopMat32;

    OverloadFunction(CoopMat16);
    OverloadFunction(CoopMat32);

    OverloadFunction(CoopMat16, Int32);
    OverloadFunction(CoopMat32, UInt32);

    OverloadFunction(OverloadFunction);

    OverloadFunction(intArray1D_2.length());

    int  intArray[];
    uint uintArray[];
    OverloadFunction(intArray);
    OverloadFunction(uintArray);

    _Buffer Buffer;
    OverloadFunction(Buffer.data);
}

void OverloadFunction() {}
