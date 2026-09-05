#define GL_KHR_shader_subgroup_quad 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

float subgroupQuadBroadcast(float _Value, uint _Id);
int subgroupQuadBroadcast(int _Value, uint _Id);
uint subgroupQuadBroadcast(uint _Value, uint _Id);
bool subgroupQuadBroadcast(bool _Value, uint _Id);
double subgroupQuadBroadcast(double _Value, uint _Id);

float subgroupQuadSwapHorizontal(float _Value);
int subgroupQuadSwapHorizontal(int _Value);
uint subgroupQuadSwapHorizontal(uint _Value);
bool subgroupQuadSwapHorizontal(bool _Value);
double subgroupQuadSwapHorizontal(double _Value);

float subgroupQuadSwapVertical(float _Value);
int subgroupQuadSwapVertical(int _Value);
uint subgroupQuadSwapVertical(uint _Value);
bool subgroupQuadSwapVertical(bool _Value);
double subgroupQuadSwapVertical(double _Value);

float subgroupQuadSwapDiagonal(float _Value);
int subgroupQuadSwapDiagonal(int _Value);
uint subgroupQuadSwapDiagonal(uint _Value);
bool subgroupQuadSwapDiagonal(bool _Value);
double subgroupQuadSwapDiagonal(double _Value);
