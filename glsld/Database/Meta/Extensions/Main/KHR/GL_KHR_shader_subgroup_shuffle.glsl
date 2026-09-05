#define GL_KHR_shader_subgroup_shuffle 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

float subgroupShuffle(float _Value, uint _Id);
int subgroupShuffle(int _Value, uint _Id);
uint subgroupShuffle(uint _Value, uint _Id);
bool subgroupShuffle(bool _Value, uint _Id);
double subgroupShuffle(double _Value, uint _Id);

float subgroupShuffleXor(float _Value, uint _Mask);
int subgroupShuffleXor(int _Value, uint _Mask);
uint subgroupShuffleXor(uint _Value, uint _Mask);
bool subgroupShuffleXor(bool _Value, uint _Mask);
double subgroupShuffleXor(double _Value, uint _Mask);
