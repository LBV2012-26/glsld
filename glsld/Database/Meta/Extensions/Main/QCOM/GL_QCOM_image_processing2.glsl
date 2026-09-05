#define GL_QCOM_image_processing2 1

#include "../KHR/GL_KHR_vulkan_glsl.glsl"
#include "GL_QCOM_image_processing.glsl"

vec4 textureBlockMatchWindowSSDQCOM(sampler2D _Target, uvec2 _TargetCoord, sampler2D _Reference, uvec2 _RefCoord, uvec2 _BlockSize);
vec4 textureBlockMatchWindowSADQCOM(sampler2D _Target, uvec2 _TargetCoord, sampler2D _Reference, uvec2 _RefCoord, uvec2 _BlockSize);
vec4 textureBlockMatchGatherSSDQCOM(sampler2D _Target, uvec2 _TargetCoord, sampler2D _Reference, uvec2 _RefCoord, uvec2 _BlockSize);
vec4 textureBlockMatchGatherSADQCOM(sampler2D _Target, uvec2 _TargetCoord, sampler2D _Reference, uvec2 _RefCoord, uvec2 _BlockSize);
