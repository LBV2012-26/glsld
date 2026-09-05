#define GL_KHR_shader_subgroup_arithmetic 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

float subgroupAdd(float _Value);
int subgroupAdd(int _Value);
uint subgroupAdd(uint _Value);
double subgroupAdd(double _Value);

float subgroupMul(float _Value);
int subgroupMul(int _Value);
uint subgroupMul(uint _Value);
double subgroupMul(double _Value);

float subgroupMin(float _Value);
int subgroupMin(int _Value);
uint subgroupMin(uint _Value);
double subgroupMin(double _Value);

float subgroupMax(float _Value);
int subgroupMax(int _Value);
uint subgroupMax(uint _Value);
double subgroupMax(double _Value);

int subgroupAnd(int _Value);
uint subgroupAnd(uint _Value);
bool subgroupAnd(bool _Value);

int subgroupOr(int _Value);
uint subgroupOr(uint _Value);
bool subgroupOr(bool _Value);

int subgroupXor(int _Value);
uint subgroupXor(uint _Value);
bool subgroupXor(bool _Value);

float subgroupInclusiveAdd(float _Value);
int subgroupInclusiveAdd(int _Value);
uint subgroupInclusiveAdd(uint _Value);
double subgroupInclusiveAdd(double _Value);

float subgroupInclusiveMul(float _Value);
int subgroupInclusiveMul(int _Value);
uint subgroupInclusiveMul(uint _Value);
double subgroupInclusiveMul(double _Value);

float subgroupInclusiveMin(float _Value);
int subgroupInclusiveMin(int _Value);
uint subgroupInclusiveMin(uint _Value);
double subgroupInclusiveMin(double _Value);

float subgroupInclusiveMax(float _Value);
int subgroupInclusiveMax(int _Value);
uint subgroupInclusiveMax(uint _Value);
double subgroupInclusiveMax(double _Value);

int subgroupInclusiveAnd(int _Value);
uint subgroupInclusiveAnd(uint _Value);
bool subgroupInclusiveAnd(bool _Value);

int subgroupInclusiveOr(int _Value);
uint subgroupInclusiveOr(uint _Value);
bool subgroupInclusiveOr(bool _Value);

int subgroupInclusiveXor(int _Value);
uint subgroupInclusiveXor(uint _Value);
bool subgroupInclusiveXor(bool _Value);

float subgroupExclusiveAdd(float _Value);
int subgroupExclusiveAdd(int _Value);
uint subgroupExclusiveAdd(uint _Value);
double subgroupExclusiveAdd(double _Value);

float subgroupExclusiveMul(float _Value);
int subgroupExclusiveMul(int _Value);
uint subgroupExclusiveMul(uint _Value);
double subgroupExclusiveMul(double _Value);

float subgroupExclusiveMin(float _Value);
int subgroupExclusiveMin(int _Value);
uint subgroupExclusiveMin(uint _Value);
double subgroupExclusiveMin(double _Value);

float subgroupExclusiveMax(float _Value);
int subgroupExclusiveMax(int _Value);
uint subgroupExclusiveMax(uint _Value);
double subgroupExclusiveMax(double _Value);

int subgroupExclusiveAnd(int _Value);
uint subgroupExclusiveAnd(uint _Value);
bool subgroupExclusiveAnd(bool _Value);

int subgroupExclusiveOr(int _Value);
uint subgroupExclusiveOr(uint _Value);
bool subgroupExclusiveOr(bool _Value);

int subgroupExclusiveXor(int _Value);
uint subgroupExclusiveXor(uint _Value);
bool subgroupExclusiveXor(bool _Value);
