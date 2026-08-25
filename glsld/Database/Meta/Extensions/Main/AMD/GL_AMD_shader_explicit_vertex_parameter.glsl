#if defined(GL_FRAGMENT_SHADER)
#define GL_AMD_shader_explicit_vertex_parameter 1

spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4992)
in vec2 gl_BaryCoordNoPerspAMD;
spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4993)
in vec2 gl_BaryCoordNoPerspCentroidAMD;
spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4994)
in vec2 gl_BaryCoordNoPerspSampleAMD;
spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4995)
in vec2 gl_BaryCoordSmoothAMD;
spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4996)
in vec2 gl_BaryCoordSmoothCentroidAMD;
spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4997)
in vec2 gl_BaryCoordSmoothSampleAMD;
spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 11, 4998)
in vec3 gl_BaryCoordPullModelAMD;

#define __explicitInterpAMD spirv_decorate(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], 4999)

spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
float interpolateAtVertexAMD(float _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
vec2 interpolateAtVertexAMD(vec2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
vec3 interpolateAtVertexAMD(vec3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
vec4 interpolateAtVertexAMD(vec4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat2x2 interpolateAtVertexAMD(mat2x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat2x3 interpolateAtVertexAMD(mat2x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat2x4 interpolateAtVertexAMD(mat2x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat3x2 interpolateAtVertexAMD(mat3x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat3x3 interpolateAtVertexAMD(mat3x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat3x4 interpolateAtVertexAMD(mat3x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat4x2 interpolateAtVertexAMD(mat4x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat4x3 interpolateAtVertexAMD(mat4x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
mat4x4 interpolateAtVertexAMD(mat4x4 _Interpolant, uint _VertexIdx);

#if defined(GL_AMD_gpu_shader_half_float) || defined(GL_EXT_shader_explicit_arithmetic_types_float16)
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
float16_t interpolateAtVertexAMD(float16_t _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16vec2 interpolateAtVertexAMD(f16vec2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16vec3 interpolateAtVertexAMD(f16vec3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16vec4 interpolateAtVertexAMD(f16vec4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat2x2 interpolateAtVertexAMD(f16mat2x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat2x3 interpolateAtVertexAMD(f16mat2x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat2x4 interpolateAtVertexAMD(f16mat2x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat3x2 interpolateAtVertexAMD(f16mat3x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat3x3 interpolateAtVertexAMD(f16mat3x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat3x4 interpolateAtVertexAMD(f16mat3x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat4x2 interpolateAtVertexAMD(f16mat4x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat4x3 interpolateAtVertexAMD(f16mat4x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16mat4x4 interpolateAtVertexAMD(f16mat4x4 _Interpolant, uint _VertexIdx);
#endif

spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uint interpolateAtVertexAMD(uint _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uvec2 interpolateAtVertexAMD(uvec2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uvec3 interpolateAtVertexAMD(uvec3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uvec4 interpolateAtVertexAMD(uvec4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat2x2 interpolateAtVertexAMD(umat2x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat2x3 interpolateAtVertexAMD(umat2x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat2x4 interpolateAtVertexAMD(umat2x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat3x2 interpolateAtVertexAMD(umat3x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat3x3 interpolateAtVertexAMD(umat3x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat3x4 interpolateAtVertexAMD(umat3x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat4x2 interpolateAtVertexAMD(umat4x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat4x3 interpolateAtVertexAMD(umat4x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
umat4x4 interpolateAtVertexAMD(umat4x4 _Interpolant, uint _VertexIdx);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uint16_t interpolateAtVertexAMD(uint16_t _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16vec2 interpolateAtVertexAMD(u16vec2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16vec3 interpolateAtVertexAMD(u16vec3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16vec4 interpolateAtVertexAMD(u16vec4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat2x2 interpolateAtVertexAMD(u16mat2x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat2x3 interpolateAtVertexAMD(u16mat2x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat2x4 interpolateAtVertexAMD(u16mat2x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat3x2 interpolateAtVertexAMD(u16mat3x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat3x3 interpolateAtVertexAMD(u16mat3x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat3x4 interpolateAtVertexAMD(u16mat3x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat4x2 interpolateAtVertexAMD(u16mat4x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat4x3 interpolateAtVertexAMD(u16mat4x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16mat4x4 interpolateAtVertexAMD(u16mat4x4 _Interpolant, uint _VertexIdx);
#endif

spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
int interpolateAtVertexAMD(int _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
ivec2 interpolateAtVertexAMD(ivec2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
ivec3 interpolateAtVertexAMD(ivec3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
ivec4 interpolateAtVertexAMD(ivec4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat2x2 interpolateAtVertexAMD(imat2x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat2x3 interpolateAtVertexAMD(imat2x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat2x4 interpolateAtVertexAMD(imat2x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat3x2 interpolateAtVertexAMD(imat3x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat3x3 interpolateAtVertexAMD(imat3x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat3x4 interpolateAtVertexAMD(imat3x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat4x2 interpolateAtVertexAMD(imat4x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat4x3 interpolateAtVertexAMD(imat4x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
imat4x4 interpolateAtVertexAMD(imat4x4 _Interpolant, uint _VertexIdx);

#if defined(GL_EXT_shader_explicit_arithmetic_types_int16)
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
int16_t interpolateAtVertexAMD(int16_t _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16vec2 interpolateAtVertexAMD(i16vec2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16vec3 interpolateAtVertexAMD(i16vec3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16vec4 interpolateAtVertexAMD(i16vec4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat2x2 interpolateAtVertexAMD(i16mat2x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat2x3 interpolateAtVertexAMD(i16mat2x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat2x4 interpolateAtVertexAMD(i16mat2x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat3x2 interpolateAtVertexAMD(i16mat3x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat3x3 interpolateAtVertexAMD(i16mat3x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat3x4 interpolateAtVertexAMD(i16mat3x4 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat4x2 interpolateAtVertexAMD(i16mat4x2 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat4x3 interpolateAtVertexAMD(i16mat4x3 _Interpolant, uint _VertexIdx);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16mat4x4 interpolateAtVertexAMD(i16mat4x4 _Interpolant, uint _VertexIdx);
#endif

#endif
