#define GL_KHR_shader_subgroup_clustered 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

float subgroupClusteredAdd(float _Value, uint _ClusterSize);
int subgroupClusteredAdd(int _Value, uint _ClusterSize);
uint subgroupClusteredAdd(uint _Value, uint _ClusterSize);
double subgroupClusteredAdd(double _Value, uint _ClusterSize);

float subgroupClusteredMul(float _Value, uint _ClusterSize);
int subgroupClusteredMul(int _Value, uint _ClusterSize);
uint subgroupClusteredMul(uint _Value, uint _ClusterSize);
double subgroupClusteredMul(double _Value, uint _ClusterSize);

float subgroupClusteredMin(float _Value, uint _ClusterSize);
int subgroupClusteredMin(int _Value, uint _ClusterSize);
uint subgroupClusteredMin(uint _Value, uint _ClusterSize);
double subgroupClusteredMin(double _Value, uint _ClusterSize);

float subgroupClusteredMax(float _Value, uint _ClusterSize);
int subgroupClusteredMax(int _Value, uint _ClusterSize);
uint subgroupClusteredMax(uint _Value, uint _ClusterSize);
double subgroupClusteredMax(double _Value, uint _ClusterSize);

int subgroupClusteredAnd(int _Value, uint _ClusterSize);
uint subgroupClusteredAnd(uint _Value, uint _ClusterSize);
bool subgroupClusteredAnd(bool _Value, uint _ClusterSize);

int subgroupClusteredOr(int _Value, uint _ClusterSize);
uint subgroupClusteredOr(uint _Value, uint _ClusterSize);
bool subgroupClusteredOr(bool _Value, uint _ClusterSize);

int subgroupClusteredXor(int _Value, uint _ClusterSize);
uint subgroupClusteredXor(uint _Value, uint _ClusterSize);
bool subgroupClusteredXor(bool _Value, uint _ClusterSize);
