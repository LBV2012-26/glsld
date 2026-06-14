uint dotEXT(uvec2 _A, uvec2 _B);
int dotEXT(ivec2 _A, ivec2 _B);
int dotEXT(ivec2 _A, uvec2 _B);
int dotEXT(uvec2 _A, ivec2 _B);

uint dotEXT(uvec3 _A, uvec3 _B);
int dotEXT(ivec3 _A, ivec3 _B);
int dotEXT(ivec3 _A, uvec3 _B);
int dotEXT(uvec3 _A, ivec3 _B);

uint dotEXT(uvec4 _A, uvec4 _B);
int dotEXT(ivec4 _A, ivec4 _B);
int dotEXT(ivec4 _A, uvec4 _B);
int dotEXT(uvec4 _A, ivec4 _B);

uint dotPacked4x8EXT(uint _A, uint _B);
int dotPacked4x8EXT(int _A, uint _B);
int dotPacked4x8EXT(uint _A, int _B);
int dotPacked4x8EXT(int _A, int _B);

uint dotAccSatEXT(uvec2 _A, uvec2 _B, uint _C);
int dotAccSatEXT(ivec2 _A, uvec2 _B, int _C);
int dotAccSatEXT(uvec2 _A, ivec2 _B, int _C);
int dotAccSatEXT(ivec2 _A, ivec2 _B, int _C);

uint dotAccSatEXT(uvec3 _A, uvec3 _B, uint _C);
int dotAccSatEXT(ivec3 _A, uvec3 _B, int _C);
int dotAccSatEXT(uvec3 _A, ivec3 _B, int _C);
int dotAccSatEXT(ivec3 _A, ivec3 _B, int _C);

uint dotAccSatEXT(uvec4 _A, uvec4 _B, uint _C);
int dotAccSatEXT(ivec4 _A, uvec4 _B, int _C);
int dotAccSatEXT(uvec4 _A, ivec4 _B, int _C);
int dotAccSatEXT(ivec4 _A, ivec4 _B, int _C);

uint dotPacked4x8AccSatEXT(uint _A, uint _B, uint _C);
int dotPacked4x8AccSatEXT(int _A, uint _B, int _C);
int dotPacked4x8AccSatEXT(uint _A, int _B, int _C);
int dotPacked4x8AccSatEXT(int _A, int _B, int _C);
