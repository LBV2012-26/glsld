#define GL_EXT_integer_dot_product 1

uint dotEXT(uvec2 a, uvec2 b);
int dotEXT(ivec2 a, ivec2 b);
int dotEXT(ivec2 a, uvec2 b);
int dotEXT(uvec2 a, ivec2 b);

uint dotEXT(uvec3 a, uvec3 b);
int dotEXT(ivec3 a, ivec3 b);
int dotEXT(ivec3 a, uvec3 b);
int dotEXT(uvec3 a, ivec3 b);

uint dotEXT(uvec4 a, uvec4 b);
int dotEXT(ivec4 a, ivec4 b);
int dotEXT(ivec4 a, uvec4 b);
int dotEXT(uvec4 a, ivec4 b);

uint dotPacked4x8EXT(uint a, uint b);
int dotPacked4x8EXT(int a, uint b);
int dotPacked4x8EXT(uint a, int b);
int dotPacked4x8EXT(int a, int b);

uint dotAccSatEXT(uvec2 a, uvec2 b, uint c);
int dotAccSatEXT(ivec2 a, uvec2 b, int c);
int dotAccSatEXT(uvec2 a, ivec2 b, int c);
int dotAccSatEXT(ivec2 a, ivec2 b, int c);

uint dotAccSatEXT(uvec3 a, uvec3 b, uint c);
int dotAccSatEXT(ivec3 a, uvec3 b, int c);
int dotAccSatEXT(uvec3 a, ivec3 b, int c);
int dotAccSatEXT(ivec3 a, ivec3 b, int c);

uint dotAccSatEXT(uvec4 a, uvec4 b, uint c);
int dotAccSatEXT(ivec4 a, uvec4 b, int c);
int dotAccSatEXT(uvec4 a, ivec4 b, int c);
int dotAccSatEXT(ivec4 a, ivec4 b, int c);

uint dotPacked4x8AccSatEXT(uint a, uint b, uint c);
int dotPacked4x8AccSatEXT(int a, uint b, int c);
int dotPacked4x8AccSatEXT(uint a, int b, int c);
int dotPacked4x8AccSatEXT(int a, int b, int c);
