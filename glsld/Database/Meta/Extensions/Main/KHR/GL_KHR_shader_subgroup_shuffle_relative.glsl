#define GL_KHR_shader_subgroup_shuffle_relative 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

float subgroupShuffleUp(float _Value, uint _Delta);
int subgroupShuffleUp(int _Value, uint _Delta);
uint subgroupShuffleUp(uint _Value, uint _Delta);
bool subgroupShuffleUp(bool _Value, uint _Delta);
double subgroupShuffleUp(double _Value, uint _Delta);

float subgroupShuffleDown(float _Value, uint _Delta);
int subgroupShuffleDown(int _Value, uint _Delta);
uint subgroupShuffleDown(uint _Value, uint _Delta);
bool subgroupShuffleDown(bool _Value, uint _Delta);
double subgroupShuffleDown(double _Value, uint _Delta);
