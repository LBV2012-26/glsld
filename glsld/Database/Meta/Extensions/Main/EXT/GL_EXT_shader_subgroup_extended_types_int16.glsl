#define GL_EXT_shader_subgroup_extended_types_int16 1

// GL_EXT_shader_subgroup_extended_types_int16 (I16/U16)
// GLSL_EXT_shader_subgroup_extended_types: genI16/U16 overloads of subgroup built-ins,
// expanded over the scalar + vec2/3/4 forms of each gen type.
// Note: Clustered rows follow this spec revision verbatim (single parameter; later revisions add the cluster size argument).

bool subgroupAllEqual(int16_t _Value);
bool subgroupAllEqual(i16vec2 _Value);
bool subgroupAllEqual(i16vec3 _Value);
bool subgroupAllEqual(i16vec4 _Value);
bool subgroupAllEqual(uint16_t _Value);
bool subgroupAllEqual(u16vec2 _Value);
bool subgroupAllEqual(u16vec3 _Value);
bool subgroupAllEqual(u16vec4 _Value);

int16_t subgroupBroadcast(int16_t _Value, uint _Id);
i16vec2 subgroupBroadcast(i16vec2 _Value, uint _Id);
i16vec3 subgroupBroadcast(i16vec3 _Value, uint _Id);
i16vec4 subgroupBroadcast(i16vec4 _Value, uint _Id);
uint16_t subgroupBroadcast(uint16_t _Value, uint _Id);
u16vec2 subgroupBroadcast(u16vec2 _Value, uint _Id);
u16vec3 subgroupBroadcast(u16vec3 _Value, uint _Id);
u16vec4 subgroupBroadcast(u16vec4 _Value, uint _Id);

int16_t subgroupBroadcastFirst(int16_t _Value);
i16vec2 subgroupBroadcastFirst(i16vec2 _Value);
i16vec3 subgroupBroadcastFirst(i16vec3 _Value);
i16vec4 subgroupBroadcastFirst(i16vec4 _Value);
uint16_t subgroupBroadcastFirst(uint16_t _Value);
u16vec2 subgroupBroadcastFirst(u16vec2 _Value);
u16vec3 subgroupBroadcastFirst(u16vec3 _Value);
u16vec4 subgroupBroadcastFirst(u16vec4 _Value);

int16_t subgroupShuffle(int16_t _Value, uint _Id);
i16vec2 subgroupShuffle(i16vec2 _Value, uint _Id);
i16vec3 subgroupShuffle(i16vec3 _Value, uint _Id);
i16vec4 subgroupShuffle(i16vec4 _Value, uint _Id);
uint16_t subgroupShuffle(uint16_t _Value, uint _Id);
u16vec2 subgroupShuffle(u16vec2 _Value, uint _Id);
u16vec3 subgroupShuffle(u16vec3 _Value, uint _Id);
u16vec4 subgroupShuffle(u16vec4 _Value, uint _Id);

int16_t subgroupShuffleXor(int16_t _Value, uint _Mask);
i16vec2 subgroupShuffleXor(i16vec2 _Value, uint _Mask);
i16vec3 subgroupShuffleXor(i16vec3 _Value, uint _Mask);
i16vec4 subgroupShuffleXor(i16vec4 _Value, uint _Mask);
uint16_t subgroupShuffleXor(uint16_t _Value, uint _Mask);
u16vec2 subgroupShuffleXor(u16vec2 _Value, uint _Mask);
u16vec3 subgroupShuffleXor(u16vec3 _Value, uint _Mask);
u16vec4 subgroupShuffleXor(u16vec4 _Value, uint _Mask);

int16_t subgroupShuffleUp(int16_t _Value, uint _Delta);
i16vec2 subgroupShuffleUp(i16vec2 _Value, uint _Delta);
i16vec3 subgroupShuffleUp(i16vec3 _Value, uint _Delta);
i16vec4 subgroupShuffleUp(i16vec4 _Value, uint _Delta);
uint16_t subgroupShuffleUp(uint16_t _Value, uint _Delta);
u16vec2 subgroupShuffleUp(u16vec2 _Value, uint _Delta);
u16vec3 subgroupShuffleUp(u16vec3 _Value, uint _Delta);
u16vec4 subgroupShuffleUp(u16vec4 _Value, uint _Delta);

int16_t subgroupShuffleDown(int16_t _Value, uint _Delta);
i16vec2 subgroupShuffleDown(i16vec2 _Value, uint _Delta);
i16vec3 subgroupShuffleDown(i16vec3 _Value, uint _Delta);
i16vec4 subgroupShuffleDown(i16vec4 _Value, uint _Delta);
uint16_t subgroupShuffleDown(uint16_t _Value, uint _Delta);
u16vec2 subgroupShuffleDown(u16vec2 _Value, uint _Delta);
u16vec3 subgroupShuffleDown(u16vec3 _Value, uint _Delta);
u16vec4 subgroupShuffleDown(u16vec4 _Value, uint _Delta);

int16_t subgroupAdd(int16_t _Value);
i16vec2 subgroupAdd(i16vec2 _Value);
i16vec3 subgroupAdd(i16vec3 _Value);
i16vec4 subgroupAdd(i16vec4 _Value);
uint16_t subgroupAdd(uint16_t _Value);
u16vec2 subgroupAdd(u16vec2 _Value);
u16vec3 subgroupAdd(u16vec3 _Value);
u16vec4 subgroupAdd(u16vec4 _Value);

int16_t subgroupMul(int16_t _Value);
i16vec2 subgroupMul(i16vec2 _Value);
i16vec3 subgroupMul(i16vec3 _Value);
i16vec4 subgroupMul(i16vec4 _Value);
uint16_t subgroupMul(uint16_t _Value);
u16vec2 subgroupMul(u16vec2 _Value);
u16vec3 subgroupMul(u16vec3 _Value);
u16vec4 subgroupMul(u16vec4 _Value);

int16_t subgroupMin(int16_t _Value);
i16vec2 subgroupMin(i16vec2 _Value);
i16vec3 subgroupMin(i16vec3 _Value);
i16vec4 subgroupMin(i16vec4 _Value);
uint16_t subgroupMin(uint16_t _Value);
u16vec2 subgroupMin(u16vec2 _Value);
u16vec3 subgroupMin(u16vec3 _Value);
u16vec4 subgroupMin(u16vec4 _Value);

int16_t subgroupMax(int16_t _Value);
i16vec2 subgroupMax(i16vec2 _Value);
i16vec3 subgroupMax(i16vec3 _Value);
i16vec4 subgroupMax(i16vec4 _Value);
uint16_t subgroupMax(uint16_t _Value);
u16vec2 subgroupMax(u16vec2 _Value);
u16vec3 subgroupMax(u16vec3 _Value);
u16vec4 subgroupMax(u16vec4 _Value);

int16_t subgroupAnd(int16_t _Value);
i16vec2 subgroupAnd(i16vec2 _Value);
i16vec3 subgroupAnd(i16vec3 _Value);
i16vec4 subgroupAnd(i16vec4 _Value);
uint16_t subgroupAnd(uint16_t _Value);
u16vec2 subgroupAnd(u16vec2 _Value);
u16vec3 subgroupAnd(u16vec3 _Value);
u16vec4 subgroupAnd(u16vec4 _Value);

int16_t subgroupOr(int16_t _Value);
i16vec2 subgroupOr(i16vec2 _Value);
i16vec3 subgroupOr(i16vec3 _Value);
i16vec4 subgroupOr(i16vec4 _Value);
uint16_t subgroupOr(uint16_t _Value);
u16vec2 subgroupOr(u16vec2 _Value);
u16vec3 subgroupOr(u16vec3 _Value);
u16vec4 subgroupOr(u16vec4 _Value);

int16_t subgroupXor(int16_t _Value);
i16vec2 subgroupXor(i16vec2 _Value);
i16vec3 subgroupXor(i16vec3 _Value);
i16vec4 subgroupXor(i16vec4 _Value);
uint16_t subgroupXor(uint16_t _Value);
u16vec2 subgroupXor(u16vec2 _Value);
u16vec3 subgroupXor(u16vec3 _Value);
u16vec4 subgroupXor(u16vec4 _Value);

int16_t subgroupInclusiveAdd(int16_t _Value);
i16vec2 subgroupInclusiveAdd(i16vec2 _Value);
i16vec3 subgroupInclusiveAdd(i16vec3 _Value);
i16vec4 subgroupInclusiveAdd(i16vec4 _Value);
uint16_t subgroupInclusiveAdd(uint16_t _Value);
u16vec2 subgroupInclusiveAdd(u16vec2 _Value);
u16vec3 subgroupInclusiveAdd(u16vec3 _Value);
u16vec4 subgroupInclusiveAdd(u16vec4 _Value);

int16_t subgroupInclusiveMul(int16_t _Value);
i16vec2 subgroupInclusiveMul(i16vec2 _Value);
i16vec3 subgroupInclusiveMul(i16vec3 _Value);
i16vec4 subgroupInclusiveMul(i16vec4 _Value);
uint16_t subgroupInclusiveMul(uint16_t _Value);
u16vec2 subgroupInclusiveMul(u16vec2 _Value);
u16vec3 subgroupInclusiveMul(u16vec3 _Value);
u16vec4 subgroupInclusiveMul(u16vec4 _Value);

int16_t subgroupInclusiveMin(int16_t _Value);
i16vec2 subgroupInclusiveMin(i16vec2 _Value);
i16vec3 subgroupInclusiveMin(i16vec3 _Value);
i16vec4 subgroupInclusiveMin(i16vec4 _Value);
uint16_t subgroupInclusiveMin(uint16_t _Value);
u16vec2 subgroupInclusiveMin(u16vec2 _Value);
u16vec3 subgroupInclusiveMin(u16vec3 _Value);
u16vec4 subgroupInclusiveMin(u16vec4 _Value);

int16_t subgroupInclusiveMax(int16_t _Value);
i16vec2 subgroupInclusiveMax(i16vec2 _Value);
i16vec3 subgroupInclusiveMax(i16vec3 _Value);
i16vec4 subgroupInclusiveMax(i16vec4 _Value);
uint16_t subgroupInclusiveMax(uint16_t _Value);
u16vec2 subgroupInclusiveMax(u16vec2 _Value);
u16vec3 subgroupInclusiveMax(u16vec3 _Value);
u16vec4 subgroupInclusiveMax(u16vec4 _Value);

int16_t subgroupInclusiveAnd(int16_t _Value);
i16vec2 subgroupInclusiveAnd(i16vec2 _Value);
i16vec3 subgroupInclusiveAnd(i16vec3 _Value);
i16vec4 subgroupInclusiveAnd(i16vec4 _Value);
uint16_t subgroupInclusiveAnd(uint16_t _Value);
u16vec2 subgroupInclusiveAnd(u16vec2 _Value);
u16vec3 subgroupInclusiveAnd(u16vec3 _Value);
u16vec4 subgroupInclusiveAnd(u16vec4 _Value);

int16_t subgroupInclusiveOr(int16_t _Value);
i16vec2 subgroupInclusiveOr(i16vec2 _Value);
i16vec3 subgroupInclusiveOr(i16vec3 _Value);
i16vec4 subgroupInclusiveOr(i16vec4 _Value);
uint16_t subgroupInclusiveOr(uint16_t _Value);
u16vec2 subgroupInclusiveOr(u16vec2 _Value);
u16vec3 subgroupInclusiveOr(u16vec3 _Value);
u16vec4 subgroupInclusiveOr(u16vec4 _Value);

int16_t subgroupInclusiveXor(int16_t _Value);
i16vec2 subgroupInclusiveXor(i16vec2 _Value);
i16vec3 subgroupInclusiveXor(i16vec3 _Value);
i16vec4 subgroupInclusiveXor(i16vec4 _Value);
uint16_t subgroupInclusiveXor(uint16_t _Value);
u16vec2 subgroupInclusiveXor(u16vec2 _Value);
u16vec3 subgroupInclusiveXor(u16vec3 _Value);
u16vec4 subgroupInclusiveXor(u16vec4 _Value);

int16_t subgroupExclusiveAdd(int16_t _Value);
i16vec2 subgroupExclusiveAdd(i16vec2 _Value);
i16vec3 subgroupExclusiveAdd(i16vec3 _Value);
i16vec4 subgroupExclusiveAdd(i16vec4 _Value);
uint16_t subgroupExclusiveAdd(uint16_t _Value);
u16vec2 subgroupExclusiveAdd(u16vec2 _Value);
u16vec3 subgroupExclusiveAdd(u16vec3 _Value);
u16vec4 subgroupExclusiveAdd(u16vec4 _Value);

int16_t subgroupExclusiveMul(int16_t _Value);
i16vec2 subgroupExclusiveMul(i16vec2 _Value);
i16vec3 subgroupExclusiveMul(i16vec3 _Value);
i16vec4 subgroupExclusiveMul(i16vec4 _Value);
uint16_t subgroupExclusiveMul(uint16_t _Value);
u16vec2 subgroupExclusiveMul(u16vec2 _Value);
u16vec3 subgroupExclusiveMul(u16vec3 _Value);
u16vec4 subgroupExclusiveMul(u16vec4 _Value);

int16_t subgroupExclusiveMin(int16_t _Value);
i16vec2 subgroupExclusiveMin(i16vec2 _Value);
i16vec3 subgroupExclusiveMin(i16vec3 _Value);
i16vec4 subgroupExclusiveMin(i16vec4 _Value);
uint16_t subgroupExclusiveMin(uint16_t _Value);
u16vec2 subgroupExclusiveMin(u16vec2 _Value);
u16vec3 subgroupExclusiveMin(u16vec3 _Value);
u16vec4 subgroupExclusiveMin(u16vec4 _Value);

int16_t subgroupExclusiveMax(int16_t _Value);
i16vec2 subgroupExclusiveMax(i16vec2 _Value);
i16vec3 subgroupExclusiveMax(i16vec3 _Value);
i16vec4 subgroupExclusiveMax(i16vec4 _Value);
uint16_t subgroupExclusiveMax(uint16_t _Value);
u16vec2 subgroupExclusiveMax(u16vec2 _Value);
u16vec3 subgroupExclusiveMax(u16vec3 _Value);
u16vec4 subgroupExclusiveMax(u16vec4 _Value);

int16_t subgroupExclusiveAnd(int16_t _Value);
i16vec2 subgroupExclusiveAnd(i16vec2 _Value);
i16vec3 subgroupExclusiveAnd(i16vec3 _Value);
i16vec4 subgroupExclusiveAnd(i16vec4 _Value);
uint16_t subgroupExclusiveAnd(uint16_t _Value);
u16vec2 subgroupExclusiveAnd(u16vec2 _Value);
u16vec3 subgroupExclusiveAnd(u16vec3 _Value);
u16vec4 subgroupExclusiveAnd(u16vec4 _Value);

int16_t subgroupExclusiveOr(int16_t _Value);
i16vec2 subgroupExclusiveOr(i16vec2 _Value);
i16vec3 subgroupExclusiveOr(i16vec3 _Value);
i16vec4 subgroupExclusiveOr(i16vec4 _Value);
uint16_t subgroupExclusiveOr(uint16_t _Value);
u16vec2 subgroupExclusiveOr(u16vec2 _Value);
u16vec3 subgroupExclusiveOr(u16vec3 _Value);
u16vec4 subgroupExclusiveOr(u16vec4 _Value);

int16_t subgroupExclusiveXor(int16_t _Value);
i16vec2 subgroupExclusiveXor(i16vec2 _Value);
i16vec3 subgroupExclusiveXor(i16vec3 _Value);
i16vec4 subgroupExclusiveXor(i16vec4 _Value);
uint16_t subgroupExclusiveXor(uint16_t _Value);
u16vec2 subgroupExclusiveXor(u16vec2 _Value);
u16vec3 subgroupExclusiveXor(u16vec3 _Value);
u16vec4 subgroupExclusiveXor(u16vec4 _Value);

int16_t subgroupClusteredAdd(int16_t _Value);
i16vec2 subgroupClusteredAdd(i16vec2 _Value);
i16vec3 subgroupClusteredAdd(i16vec3 _Value);
i16vec4 subgroupClusteredAdd(i16vec4 _Value);
uint16_t subgroupClusteredAdd(uint16_t _Value);
u16vec2 subgroupClusteredAdd(u16vec2 _Value);
u16vec3 subgroupClusteredAdd(u16vec3 _Value);
u16vec4 subgroupClusteredAdd(u16vec4 _Value);

int16_t subgroupClusteredMul(int16_t _Value);
i16vec2 subgroupClusteredMul(i16vec2 _Value);
i16vec3 subgroupClusteredMul(i16vec3 _Value);
i16vec4 subgroupClusteredMul(i16vec4 _Value);
uint16_t subgroupClusteredMul(uint16_t _Value);
u16vec2 subgroupClusteredMul(u16vec2 _Value);
u16vec3 subgroupClusteredMul(u16vec3 _Value);
u16vec4 subgroupClusteredMul(u16vec4 _Value);

int16_t subgroupClusteredMin(int16_t _Value);
i16vec2 subgroupClusteredMin(i16vec2 _Value);
i16vec3 subgroupClusteredMin(i16vec3 _Value);
i16vec4 subgroupClusteredMin(i16vec4 _Value);
uint16_t subgroupClusteredMin(uint16_t _Value);
u16vec2 subgroupClusteredMin(u16vec2 _Value);
u16vec3 subgroupClusteredMin(u16vec3 _Value);
u16vec4 subgroupClusteredMin(u16vec4 _Value);

int16_t subgroupClusteredMax(int16_t _Value);
i16vec2 subgroupClusteredMax(i16vec2 _Value);
i16vec3 subgroupClusteredMax(i16vec3 _Value);
i16vec4 subgroupClusteredMax(i16vec4 _Value);
uint16_t subgroupClusteredMax(uint16_t _Value);
u16vec2 subgroupClusteredMax(u16vec2 _Value);
u16vec3 subgroupClusteredMax(u16vec3 _Value);
u16vec4 subgroupClusteredMax(u16vec4 _Value);

int16_t subgroupClusteredAnd(int16_t _Value);
i16vec2 subgroupClusteredAnd(i16vec2 _Value);
i16vec3 subgroupClusteredAnd(i16vec3 _Value);
i16vec4 subgroupClusteredAnd(i16vec4 _Value);
uint16_t subgroupClusteredAnd(uint16_t _Value);
u16vec2 subgroupClusteredAnd(u16vec2 _Value);
u16vec3 subgroupClusteredAnd(u16vec3 _Value);
u16vec4 subgroupClusteredAnd(u16vec4 _Value);

int16_t subgroupClusteredOr(int16_t _Value);
i16vec2 subgroupClusteredOr(i16vec2 _Value);
i16vec3 subgroupClusteredOr(i16vec3 _Value);
i16vec4 subgroupClusteredOr(i16vec4 _Value);
uint16_t subgroupClusteredOr(uint16_t _Value);
u16vec2 subgroupClusteredOr(u16vec2 _Value);
u16vec3 subgroupClusteredOr(u16vec3 _Value);
u16vec4 subgroupClusteredOr(u16vec4 _Value);

int16_t subgroupClusteredXor(int16_t _Value);
i16vec2 subgroupClusteredXor(i16vec2 _Value);
i16vec3 subgroupClusteredXor(i16vec3 _Value);
i16vec4 subgroupClusteredXor(i16vec4 _Value);
uint16_t subgroupClusteredXor(uint16_t _Value);
u16vec2 subgroupClusteredXor(u16vec2 _Value);
u16vec3 subgroupClusteredXor(u16vec3 _Value);
u16vec4 subgroupClusteredXor(u16vec4 _Value);

int16_t subgroupQuadBroadcast(int16_t _Value, uint _Id);
i16vec2 subgroupQuadBroadcast(i16vec2 _Value, uint _Id);
i16vec3 subgroupQuadBroadcast(i16vec3 _Value, uint _Id);
i16vec4 subgroupQuadBroadcast(i16vec4 _Value, uint _Id);
uint16_t subgroupQuadBroadcast(uint16_t _Value, uint _Id);
u16vec2 subgroupQuadBroadcast(u16vec2 _Value, uint _Id);
u16vec3 subgroupQuadBroadcast(u16vec3 _Value, uint _Id);
u16vec4 subgroupQuadBroadcast(u16vec4 _Value, uint _Id);

int16_t subgroupQuadSwapHorizontal(int16_t _Value);
i16vec2 subgroupQuadSwapHorizontal(i16vec2 _Value);
i16vec3 subgroupQuadSwapHorizontal(i16vec3 _Value);
i16vec4 subgroupQuadSwapHorizontal(i16vec4 _Value);
uint16_t subgroupQuadSwapHorizontal(uint16_t _Value);
u16vec2 subgroupQuadSwapHorizontal(u16vec2 _Value);
u16vec3 subgroupQuadSwapHorizontal(u16vec3 _Value);
u16vec4 subgroupQuadSwapHorizontal(u16vec4 _Value);

int16_t subgroupQuadSwapVertical(int16_t _Value);
i16vec2 subgroupQuadSwapVertical(i16vec2 _Value);
i16vec3 subgroupQuadSwapVertical(i16vec3 _Value);
i16vec4 subgroupQuadSwapVertical(i16vec4 _Value);
uint16_t subgroupQuadSwapVertical(uint16_t _Value);
u16vec2 subgroupQuadSwapVertical(u16vec2 _Value);
u16vec3 subgroupQuadSwapVertical(u16vec3 _Value);
u16vec4 subgroupQuadSwapVertical(u16vec4 _Value);

int16_t subgroupQuadSwapDiagonal(int16_t _Value);
i16vec2 subgroupQuadSwapDiagonal(i16vec2 _Value);
i16vec3 subgroupQuadSwapDiagonal(i16vec3 _Value);
i16vec4 subgroupQuadSwapDiagonal(i16vec4 _Value);
uint16_t subgroupQuadSwapDiagonal(uint16_t _Value);
u16vec2 subgroupQuadSwapDiagonal(u16vec2 _Value);
u16vec3 subgroupQuadSwapDiagonal(u16vec3 _Value);
u16vec4 subgroupQuadSwapDiagonal(u16vec4 _Value);

int16_t subgroupPartitionedAddNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedAddNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedAddNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedAddNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedAddNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedAddNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedAddNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedAddNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedMulNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedMulNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedMulNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedMulNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedMulNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedMulNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedMulNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedMulNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedMinNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedMinNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedMinNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedMinNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedMinNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedMinNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedMinNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedMinNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedMaxNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedMaxNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedMaxNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedMaxNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedMaxNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedMaxNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedMaxNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedMaxNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedAndNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedAndNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedAndNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedAndNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedAndNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedAndNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedAndNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedAndNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedOrNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedOrNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedOrNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedOrNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedOrNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedOrNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedOrNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedOrNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedXorNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedXorNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedXorNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedXorNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedXorNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedXorNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedXorNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedXorNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveAddNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveAddNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveAddNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveAddNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveAddNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveAddNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveAddNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveAddNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveMulNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveMulNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveMulNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveMulNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveMulNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveMulNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveMulNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveMulNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveMinNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveMinNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveMinNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveMinNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveMinNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveMinNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveMinNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveMinNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveMaxNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveMaxNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveMaxNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveMaxNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveMaxNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveMaxNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveMaxNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveMaxNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveAndNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveAndNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveAndNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveAndNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveAndNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveAndNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveAndNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveAndNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveOrNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveOrNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveOrNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveOrNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveOrNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveOrNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveOrNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveOrNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedInclusiveXorNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedInclusiveXorNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedInclusiveXorNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedInclusiveXorNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedInclusiveXorNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedInclusiveXorNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedInclusiveXorNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedInclusiveXorNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveAddNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveAddNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveAddNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveAddNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveAddNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveAddNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveAddNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveAddNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveMulNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveMulNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveMulNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveMulNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveMulNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveMulNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveMulNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveMulNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveMinNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveMinNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveMinNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveMinNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveMinNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveMinNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveMinNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveMinNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveMaxNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveMaxNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveMaxNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveMaxNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveMaxNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveMaxNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveMaxNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveMaxNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveAndNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveAndNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveAndNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveAndNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveAndNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveAndNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveAndNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveAndNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveOrNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveOrNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveOrNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveOrNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveOrNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveOrNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveOrNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveOrNV(u16vec4 _Value, uvec4 _Ballot);

int16_t subgroupPartitionedExclusiveXorNV(int16_t _Value, uvec4 _Ballot);
i16vec2 subgroupPartitionedExclusiveXorNV(i16vec2 _Value, uvec4 _Ballot);
i16vec3 subgroupPartitionedExclusiveXorNV(i16vec3 _Value, uvec4 _Ballot);
i16vec4 subgroupPartitionedExclusiveXorNV(i16vec4 _Value, uvec4 _Ballot);
uint16_t subgroupPartitionedExclusiveXorNV(uint16_t _Value, uvec4 _Ballot);
u16vec2 subgroupPartitionedExclusiveXorNV(u16vec2 _Value, uvec4 _Ballot);
u16vec3 subgroupPartitionedExclusiveXorNV(u16vec3 _Value, uvec4 _Ballot);
u16vec4 subgroupPartitionedExclusiveXorNV(u16vec4 _Value, uvec4 _Ballot);

uvec4 subgroupPartitionNV(int16_t _Value);
uvec4 subgroupPartitionNV(i16vec2 _Value);
uvec4 subgroupPartitionNV(i16vec3 _Value);
uvec4 subgroupPartitionNV(i16vec4 _Value);
uvec4 subgroupPartitionNV(uint16_t _Value);
uvec4 subgroupPartitionNV(u16vec2 _Value);
uvec4 subgroupPartitionNV(u16vec3 _Value);
uvec4 subgroupPartitionNV(u16vec4 _Value);
