#define GL_QCOM_image_processing 1

#include "../KHR/GL_KHR_vulkan_glsl.glsl"

vec4 textureWeightedQCOM(sampler2D _Texture, vec2 _Pos, sampler2DArray _Weight);
vec4 textureBoxFilterQCOM(sampler2D _Texture, vec2 _Pos, vec2 _BoxSize);
vec4 textureBlockMatchSADQCOM(sampler2D _Target, uvec2 _TargetCoord, sampler2D _Reference, uvec2 _RefCoord, uvec2 _BlockSize);
vec4 textureBlockMatchSSDQCOM(sampler2D _Target, uvec2 _TargetCoord, sampler2D _Reference, uvec2 _RefCoord, uvec2 _BlockSize);
