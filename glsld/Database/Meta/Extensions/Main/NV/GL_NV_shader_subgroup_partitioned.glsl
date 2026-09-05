#define GL_NV_shader_subgroup_partitioned 1

#include "../KHR/GL_KHR_shader_subgroup.glsl"

float subgroupPartitionedAddNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedAddNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedAddNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedAddNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedMulNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedMulNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedMulNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedMulNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedMinNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedMinNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedMinNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedMinNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedMaxNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedMaxNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedMaxNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedMaxNV(double _Value, uvec4 _Ballot);

int subgroupPartitionedAndNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedAndNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedAndNV(bool _Value, uvec4 _Ballot);

int subgroupPartitionedOrNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedOrNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedOrNV(bool _Value, uvec4 _Ballot);

int subgroupPartitionedXorNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedXorNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedXorNV(bool _Value, uvec4 _Ballot);

float subgroupPartitionedInclusiveAddNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedInclusiveAddNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveAddNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedInclusiveAddNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedInclusiveMulNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedInclusiveMulNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveMulNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedInclusiveMulNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedInclusiveMinNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedInclusiveMinNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveMinNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedInclusiveMinNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedInclusiveMaxNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedInclusiveMaxNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveMaxNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedInclusiveMaxNV(double _Value, uvec4 _Ballot);

int subgroupPartitionedInclusiveAndNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveAndNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedInclusiveAndNV(bool _Value, uvec4 _Ballot);

int subgroupPartitionedInclusiveOrNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveOrNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedInclusiveOrNV(bool _Value, uvec4 _Ballot);

int subgroupPartitionedInclusiveXorNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedInclusiveXorNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedInclusiveXorNV(bool _Value, uvec4 _Ballot);

float subgroupPartitionedExclusiveAddNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedExclusiveAddNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveAddNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedExclusiveAddNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedExclusiveMulNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedExclusiveMulNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveMulNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedExclusiveMulNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedExclusiveMinNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedExclusiveMinNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveMinNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedExclusiveMinNV(double _Value, uvec4 _Ballot);

float subgroupPartitionedExclusiveMaxNV(float _Value, uvec4 _Ballot);
int subgroupPartitionedExclusiveMaxNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveMaxNV(uint _Value, uvec4 _Ballot);
double subgroupPartitionedExclusiveMaxNV(double _Value, uvec4 _Ballot);

int subgroupPartitionedExclusiveAndNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveAndNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedExclusiveAndNV(bool _Value, uvec4 _Ballot);

int subgroupPartitionedExclusiveOrNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveOrNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedExclusiveOrNV(bool _Value, uvec4 _Ballot);

int subgroupPartitionedExclusiveXorNV(int _Value, uvec4 _Ballot);
uint subgroupPartitionedExclusiveXorNV(uint _Value, uvec4 _Ballot);
bool subgroupPartitionedExclusiveXorNV(bool _Value, uvec4 _Ballot);

uvec4 subgroupPartitionNV(float _Value);
uvec4 subgroupPartitionNV(int _Value);
uvec4 subgroupPartitionNV(uint _Value);
uvec4 subgroupPartitionNV(bool _Value);
uvec4 subgroupPartitionNV(double _Value);
