#define GL_AMD_shader_fragment_mask 1

uint fragmentMaskFetchAMD(sampler2DMS _Sampler, ivec2 _Pos);
uint fragmentMaskFetchAMD(isampler2DMS _Sampler, ivec2 _Pos);
uint fragmentMaskFetchAMD(usampler2DMS _Sampler, ivec2 _Pos);
uint fragmentMaskFetchAMD(sampler2DMSArray _Sampler, ivec3 _Pos);
uint fragmentMaskFetchAMD(isampler2DMSArray _Sampler, ivec3 _Pos);
uint fragmentMaskFetchAMD(usampler2DMSArray _Sampler, ivec3 _Pos);
uint fragmentMaskFetchAMD(subpassInputMS _Subpass);
uint fragmentMaskFetchAMD(isubpassInputMS _Subpass);
uint fragmentMaskFetchAMD(usubpassInputMS _Subpass);

vec4  fragmentFetchAMD(sampler2DMS _Sampler, ivec2 _Pos, uint _FragIndex);
ivec4 fragmentFetchAMD(isampler2DMS _Sampler, ivec2 _Pos, uint _FragIndex);
uvec4 fragmentFetchAMD(usampler2DMS _Sampler, ivec2 _Pos, uint _FragIndex);
vec4  fragmentFetchAMD(sampler2DMSArray _Sampler, ivec3 _Pos, uint _FragIndex);
ivec4 fragmentFetchAMD(isampler2DMSArray _Sampler, ivec3 _Pos, uint _FragIndex);
uvec4 fragmentFetchAMD(usampler2DMSArray _Sampler, ivec3 _Pos, uint _FragIndex);
vec4  fragmentFetchAMD(subpassInputMS _Subpass, uint _FragIndex);
ivec4 fragmentFetchAMD(isubpassInputMS _Subpass, uint _FragIndex);
uvec4 fragmentFetchAMD(usubpassInputMS _Subpass, uint _FragIndex);
