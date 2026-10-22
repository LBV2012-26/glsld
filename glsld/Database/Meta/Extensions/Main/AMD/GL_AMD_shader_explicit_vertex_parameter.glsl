#if defined(GL_FRAGMENT_SHADER)
#define __GLSLD_EXTENSION_ENABLED_GL_AMD_shader_explicit_vertex_parameter 1

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
float interpolateAtVertexAMD(float _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
vec2 interpolateAtVertexAMD(vec2 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
vec3 interpolateAtVertexAMD(vec3 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
vec4 interpolateAtVertexAMD(vec4 _Interpolant, uint _VertexIndex);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_AMD_gpu_shader_half_float) || defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_float16)
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
float16_t interpolateAtVertexAMD(float16_t _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16vec2 interpolateAtVertexAMD(f16vec2 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16vec3 interpolateAtVertexAMD(f16vec3 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
f16vec4 interpolateAtVertexAMD(f16vec4 _Interpolant, uint _VertexIndex);
#endif

spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uint interpolateAtVertexAMD(uint _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uvec2 interpolateAtVertexAMD(uvec2 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uvec3 interpolateAtVertexAMD(uvec3 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uvec4 interpolateAtVertexAMD(uvec4 _Interpolant, uint _VertexIndex);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16)
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
uint16_t interpolateAtVertexAMD(uint16_t _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16vec2 interpolateAtVertexAMD(u16vec2 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16vec3 interpolateAtVertexAMD(u16vec3 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
u16vec4 interpolateAtVertexAMD(u16vec4 _Interpolant, uint _VertexIndex);
#endif

spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
int interpolateAtVertexAMD(int _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
ivec2 interpolateAtVertexAMD(ivec2 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
ivec3 interpolateAtVertexAMD(ivec3 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
ivec4 interpolateAtVertexAMD(ivec4 _Interpolant, uint _VertexIndex);

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_shader_explicit_arithmetic_types_int16)
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
int16_t interpolateAtVertexAMD(int16_t _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16vec2 interpolateAtVertexAMD(i16vec2 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16vec3 interpolateAtVertexAMD(i16vec3 _Interpolant, uint _VertexIndex);
spirv_instruction(extensions = ["SPV_AMD_shader_explicit_vertex_parameter"], set = "SPV_AMD_shader_explicit_vertex_parameter", id = 1)
i16vec4 interpolateAtVertexAMD(i16vec4 _Interpolant, uint _VertexIndex);
#endif

#endif
