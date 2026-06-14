#ifndef RAY_TRACING_COMMON_GLSL_
#define RAY_TRACING_COMMON_GLSL_

#if defined(GL_EXT_ray_query) && !defined(GL_EXT_ray_tracing)
#define accelerationStructureEXT spirv_type(extensions = ["SPV_KHR_ray_query"], capabilities = [4472], id = 5341)
#elif !defined(GL_EXT_ray_query) && defined(GL_EXT_ray_tracing)
#define accelerationStructureEXT spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [5353], id = 5341)
#elif (defined(GL_EXT_ray_query) && defined(GL_EXT_ray_tracing)) || defined(GL_EXT_shader_invocation_reorder)
// capabilities 5353 implicit include 4472
#define accelerationStructureEXT spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [5353], id = 5341)
#endif

const uint gl_RayFlagsNoneEXT                     = 0U;
const uint gl_RayFlagsOpaqueEXT                   = 1U;
const uint gl_RayFlagsNoOpaqueEXT                 = 2U;
const uint gl_RayFlagsTerminateOnFirstHitEXT      = 4U;
const uint gl_RayFlagsSkipClosestHitShaderEXT     = 8U;
const uint gl_RayFlagsCullBackFacingTrianglesEXT  = 16U;
const uint gl_RayFlagsCullFrontFacingTrianglesEXT = 32U;
const uint gl_RayFlagsCullOpaqueEXT               = 64U;
const uint gl_RayFlagsCullNoOpaqueEXT             = 128U;

#endif
