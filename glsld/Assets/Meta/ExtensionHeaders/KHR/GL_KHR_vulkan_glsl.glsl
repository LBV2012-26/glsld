in int gl_VertexIndex;
in int gl_InstanceIndex;

const int gl_MaxInputAttachments = 1;

vec4 subpassLoad(subpassInput _Subpass);
ivec4 subpassLoad(isubpassInput _Subpass);
uvec4 subpassLoad(usubpassInput _Subpass);

vec4 subpassLoad(subpassInputMS _Subpass, int _Sample);
ivec4 subpassLoad(isubpassInputMS _Subpass, int _Sample);
uvec4 subpassLoad(usubpassInputMS _Subpass, int _Sample);
