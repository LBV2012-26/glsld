#define GL_EXT_shader_subgroup_extended_types_int8 1

// GL_EXT_shader_subgroup_extended_types_int8 (I8/U8)
// GLSL_EXT_shader_subgroup_extended_types: genI8/U8 overloads of subgroup built-ins,
// expanded over the scalar + vec2/3/4 forms of each gen type.
// Note: Clustered rows follow this spec revision verbatim (single parameter; later revisions add the cluster size argument).

bool subgroupAllEqual(int8_t _Value);
bool subgroupAllEqual(i8vec2 _Value);
bool subgroupAllEqual(i8vec3 _Value);
bool subgroupAllEqual(i8vec4 _Value);
bool subgroupAllEqual(uint8_t _Value);
bool subgroupAllEqual(u8vec2 _Value);
bool subgroupAllEqual(u8vec3 _Value);
bool subgroupAllEqual(u8vec4 _Value);

int8_t subgroupBroadcast(int8_t _Value, uint _Id);
i8vec2 subgroupBroadcast(i8vec2 _Value, uint _Id);
i8vec3 subgroupBroadcast(i8vec3 _Value, uint _Id);
i8vec4 subgroupBroadcast(i8vec4 _Value, uint _Id);
uint8_t subgroupBroadcast(uint8_t _Value, uint _Id);
u8vec2 subgroupBroadcast(u8vec2 _Value, uint _Id);
u8vec3 subgroupBroadcast(u8vec3 _Value, uint _Id);
u8vec4 subgroupBroadcast(u8vec4 _Value, uint _Id);

int8_t subgroupBroadcastFirst(int8_t _Value);
i8vec2 subgroupBroadcastFirst(i8vec2 _Value);
i8vec3 subgroupBroadcastFirst(i8vec3 _Value);
i8vec4 subgroupBroadcastFirst(i8vec4 _Value);
uint8_t subgroupBroadcastFirst(uint8_t _Value);
u8vec2 subgroupBroadcastFirst(u8vec2 _Value);
u8vec3 subgroupBroadcastFirst(u8vec3 _Value);
u8vec4 subgroupBroadcastFirst(u8vec4 _Value);

int8_t subgroupShuffle(int8_t _Value, uint _Id);
i8vec2 subgroupShuffle(i8vec2 _Value, uint _Id);
i8vec3 subgroupShuffle(i8vec3 _Value, uint _Id);
i8vec4 subgroupShuffle(i8vec4 _Value, uint _Id);
uint8_t subgroupShuffle(uint8_t _Value, uint _Id);
u8vec2 subgroupShuffle(u8vec2 _Value, uint _Id);
u8vec3 subgroupShuffle(u8vec3 _Value, uint _Id);
u8vec4 subgroupShuffle(u8vec4 _Value, uint _Id);

int8_t subgroupShuffleXor(int8_t _Value, uint _Mask);
i8vec2 subgroupShuffleXor(i8vec2 _Value, uint _Mask);
i8vec3 subgroupShuffleXor(i8vec3 _Value, uint _Mask);
i8vec4 subgroupShuffleXor(i8vec4 _Value, uint _Mask);
uint8_t subgroupShuffleXor(uint8_t _Value, uint _Mask);
u8vec2 subgroupShuffleXor(u8vec2 _Value, uint _Mask);
u8vec3 subgroupShuffleXor(u8vec3 _Value, uint _Mask);
u8vec4 subgroupShuffleXor(u8vec4 _Value, uint _Mask);

int8_t subgroupShuffleUp(int8_t _Value, uint _Delta);
i8vec2 subgroupShuffleUp(i8vec2 _Value, uint _Delta);
i8vec3 subgroupShuffleUp(i8vec3 _Value, uint _Delta);
i8vec4 subgroupShuffleUp(i8vec4 _Value, uint _Delta);
uint8_t subgroupShuffleUp(uint8_t _Value, uint _Delta);
u8vec2 subgroupShuffleUp(u8vec2 _Value, uint _Delta);
u8vec3 subgroupShuffleUp(u8vec3 _Value, uint _Delta);
u8vec4 subgroupShuffleUp(u8vec4 _Value, uint _Delta);

int8_t subgroupShuffleDown(int8_t _Value, uint _Delta);
i8vec2 subgroupShuffleDown(i8vec2 _Value, uint _Delta);
i8vec3 subgroupShuffleDown(i8vec3 _Value, uint _Delta);
i8vec4 subgroupShuffleDown(i8vec4 _Value, uint _Delta);
uint8_t subgroupShuffleDown(uint8_t _Value, uint _Delta);
u8vec2 subgroupShuffleDown(u8vec2 _Value, uint _Delta);
u8vec3 subgroupShuffleDown(u8vec3 _Value, uint _Delta);
u8vec4 subgroupShuffleDown(u8vec4 _Value, uint _Delta);

int8_t subgroupAdd(int8_t _Value);
i8vec2 subgroupAdd(i8vec2 _Value);
i8vec3 subgroupAdd(i8vec3 _Value);
i8vec4 subgroupAdd(i8vec4 _Value);
uint8_t subgroupAdd(uint8_t _Value);
u8vec2 subgroupAdd(u8vec2 _Value);
u8vec3 subgroupAdd(u8vec3 _Value);
u8vec4 subgroupAdd(u8vec4 _Value);

int8_t subgroupMul(int8_t _Value);
i8vec2 subgroupMul(i8vec2 _Value);
i8vec3 subgroupMul(i8vec3 _Value);
i8vec4 subgroupMul(i8vec4 _Value);
uint8_t subgroupMul(uint8_t _Value);
u8vec2 subgroupMul(u8vec2 _Value);
u8vec3 subgroupMul(u8vec3 _Value);
u8vec4 subgroupMul(u8vec4 _Value);

int8_t subgroupMin(int8_t _Value);
i8vec2 subgroupMin(i8vec2 _Value);
i8vec3 subgroupMin(i8vec3 _Value);
i8vec4 subgroupMin(i8vec4 _Value);
uint8_t subgroupMin(uint8_t _Value);
u8vec2 subgroupMin(u8vec2 _Value);
u8vec3 subgroupMin(u8vec3 _Value);
u8vec4 subgroupMin(u8vec4 _Value);

int8_t subgroupMax(int8_t _Value);
i8vec2 subgroupMax(i8vec2 _Value);
i8vec3 subgroupMax(i8vec3 _Value);
i8vec4 subgroupMax(i8vec4 _Value);
uint8_t subgroupMax(uint8_t _Value);
u8vec2 subgroupMax(u8vec2 _Value);
u8vec3 subgroupMax(u8vec3 _Value);
u8vec4 subgroupMax(u8vec4 _Value);

int8_t subgroupAnd(int8_t _Value);
i8vec2 subgroupAnd(i8vec2 _Value);
i8vec3 subgroupAnd(i8vec3 _Value);
i8vec4 subgroupAnd(i8vec4 _Value);
uint8_t subgroupAnd(uint8_t _Value);
u8vec2 subgroupAnd(u8vec2 _Value);
u8vec3 subgroupAnd(u8vec3 _Value);
u8vec4 subgroupAnd(u8vec4 _Value);

int8_t subgroupOr(int8_t _Value);
i8vec2 subgroupOr(i8vec2 _Value);
i8vec3 subgroupOr(i8vec3 _Value);
i8vec4 subgroupOr(i8vec4 _Value);
uint8_t subgroupOr(uint8_t _Value);
u8vec2 subgroupOr(u8vec2 _Value);
u8vec3 subgroupOr(u8vec3 _Value);
u8vec4 subgroupOr(u8vec4 _Value);

int8_t subgroupXor(int8_t _Value);
i8vec2 subgroupXor(i8vec2 _Value);
i8vec3 subgroupXor(i8vec3 _Value);
i8vec4 subgroupXor(i8vec4 _Value);
uint8_t subgroupXor(uint8_t _Value);
u8vec2 subgroupXor(u8vec2 _Value);
u8vec3 subgroupXor(u8vec3 _Value);
u8vec4 subgroupXor(u8vec4 _Value);

int8_t subgroupInclusiveAdd(int8_t _Value);
i8vec2 subgroupInclusiveAdd(i8vec2 _Value);
i8vec3 subgroupInclusiveAdd(i8vec3 _Value);
i8vec4 subgroupInclusiveAdd(i8vec4 _Value);
uint8_t subgroupInclusiveAdd(uint8_t _Value);
u8vec2 subgroupInclusiveAdd(u8vec2 _Value);
u8vec3 subgroupInclusiveAdd(u8vec3 _Value);
u8vec4 subgroupInclusiveAdd(u8vec4 _Value);

int8_t subgroupInclusiveMul(int8_t _Value);
i8vec2 subgroupInclusiveMul(i8vec2 _Value);
i8vec3 subgroupInclusiveMul(i8vec3 _Value);
i8vec4 subgroupInclusiveMul(i8vec4 _Value);
uint8_t subgroupInclusiveMul(uint8_t _Value);
u8vec2 subgroupInclusiveMul(u8vec2 _Value);
u8vec3 subgroupInclusiveMul(u8vec3 _Value);
u8vec4 subgroupInclusiveMul(u8vec4 _Value);

int8_t subgroupInclusiveMin(int8_t _Value);
i8vec2 subgroupInclusiveMin(i8vec2 _Value);
i8vec3 subgroupInclusiveMin(i8vec3 _Value);
i8vec4 subgroupInclusiveMin(i8vec4 _Value);
uint8_t subgroupInclusiveMin(uint8_t _Value);
u8vec2 subgroupInclusiveMin(u8vec2 _Value);
u8vec3 subgroupInclusiveMin(u8vec3 _Value);
u8vec4 subgroupInclusiveMin(u8vec4 _Value);

int8_t subgroupInclusiveMax(int8_t _Value);
i8vec2 subgroupInclusiveMax(i8vec2 _Value);
i8vec3 subgroupInclusiveMax(i8vec3 _Value);
i8vec4 subgroupInclusiveMax(i8vec4 _Value);
uint8_t subgroupInclusiveMax(uint8_t _Value);
u8vec2 subgroupInclusiveMax(u8vec2 _Value);
u8vec3 subgroupInclusiveMax(u8vec3 _Value);
u8vec4 subgroupInclusiveMax(u8vec4 _Value);

int8_t subgroupInclusiveAnd(int8_t _Value);
i8vec2 subgroupInclusiveAnd(i8vec2 _Value);
i8vec3 subgroupInclusiveAnd(i8vec3 _Value);
i8vec4 subgroupInclusiveAnd(i8vec4 _Value);
uint8_t subgroupInclusiveAnd(uint8_t _Value);
u8vec2 subgroupInclusiveAnd(u8vec2 _Value);
u8vec3 subgroupInclusiveAnd(u8vec3 _Value);
u8vec4 subgroupInclusiveAnd(u8vec4 _Value);

int8_t subgroupInclusiveOr(int8_t _Value);
i8vec2 subgroupInclusiveOr(i8vec2 _Value);
i8vec3 subgroupInclusiveOr(i8vec3 _Value);
i8vec4 subgroupInclusiveOr(i8vec4 _Value);
uint8_t subgroupInclusiveOr(uint8_t _Value);
u8vec2 subgroupInclusiveOr(u8vec2 _Value);
u8vec3 subgroupInclusiveOr(u8vec3 _Value);
u8vec4 subgroupInclusiveOr(u8vec4 _Value);

int8_t subgroupInclusiveXor(int8_t _Value);
i8vec2 subgroupInclusiveXor(i8vec2 _Value);
i8vec3 subgroupInclusiveXor(i8vec3 _Value);
i8vec4 subgroupInclusiveXor(i8vec4 _Value);
uint8_t subgroupInclusiveXor(uint8_t _Value);
u8vec2 subgroupInclusiveXor(u8vec2 _Value);
u8vec3 subgroupInclusiveXor(u8vec3 _Value);
u8vec4 subgroupInclusiveXor(u8vec4 _Value);

int8_t subgroupExclusiveAdd(int8_t _Value);
i8vec2 subgroupExclusiveAdd(i8vec2 _Value);
i8vec3 subgroupExclusiveAdd(i8vec3 _Value);
i8vec4 subgroupExclusiveAdd(i8vec4 _Value);
uint8_t subgroupExclusiveAdd(uint8_t _Value);
u8vec2 subgroupExclusiveAdd(u8vec2 _Value);
u8vec3 subgroupExclusiveAdd(u8vec3 _Value);
u8vec4 subgroupExclusiveAdd(u8vec4 _Value);

int8_t subgroupExclusiveMul(int8_t _Value);
i8vec2 subgroupExclusiveMul(i8vec2 _Value);
i8vec3 subgroupExclusiveMul(i8vec3 _Value);
i8vec4 subgroupExclusiveMul(i8vec4 _Value);
uint8_t subgroupExclusiveMul(uint8_t _Value);
u8vec2 subgroupExclusiveMul(u8vec2 _Value);
u8vec3 subgroupExclusiveMul(u8vec3 _Value);
u8vec4 subgroupExclusiveMul(u8vec4 _Value);

int8_t subgroupExclusiveMin(int8_t _Value);
i8vec2 subgroupExclusiveMin(i8vec2 _Value);
i8vec3 subgroupExclusiveMin(i8vec3 _Value);
i8vec4 subgroupExclusiveMin(i8vec4 _Value);
uint8_t subgroupExclusiveMin(uint8_t _Value);
u8vec2 subgroupExclusiveMin(u8vec2 _Value);
u8vec3 subgroupExclusiveMin(u8vec3 _Value);
u8vec4 subgroupExclusiveMin(u8vec4 _Value);

int8_t subgroupExclusiveMax(int8_t _Value);
i8vec2 subgroupExclusiveMax(i8vec2 _Value);
i8vec3 subgroupExclusiveMax(i8vec3 _Value);
i8vec4 subgroupExclusiveMax(i8vec4 _Value);
uint8_t subgroupExclusiveMax(uint8_t _Value);
u8vec2 subgroupExclusiveMax(u8vec2 _Value);
u8vec3 subgroupExclusiveMax(u8vec3 _Value);
u8vec4 subgroupExclusiveMax(u8vec4 _Value);

int8_t subgroupExclusiveAnd(int8_t _Value);
i8vec2 subgroupExclusiveAnd(i8vec2 _Value);
i8vec3 subgroupExclusiveAnd(i8vec3 _Value);
i8vec4 subgroupExclusiveAnd(i8vec4 _Value);
uint8_t subgroupExclusiveAnd(uint8_t _Value);
u8vec2 subgroupExclusiveAnd(u8vec2 _Value);
u8vec3 subgroupExclusiveAnd(u8vec3 _Value);
u8vec4 subgroupExclusiveAnd(u8vec4 _Value);

int8_t subgroupExclusiveOr(int8_t _Value);
i8vec2 subgroupExclusiveOr(i8vec2 _Value);
i8vec3 subgroupExclusiveOr(i8vec3 _Value);
i8vec4 subgroupExclusiveOr(i8vec4 _Value);
uint8_t subgroupExclusiveOr(uint8_t _Value);
u8vec2 subgroupExclusiveOr(u8vec2 _Value);
u8vec3 subgroupExclusiveOr(u8vec3 _Value);
u8vec4 subgroupExclusiveOr(u8vec4 _Value);

int8_t subgroupExclusiveXor(int8_t _Value);
i8vec2 subgroupExclusiveXor(i8vec2 _Value);
i8vec3 subgroupExclusiveXor(i8vec3 _Value);
i8vec4 subgroupExclusiveXor(i8vec4 _Value);
uint8_t subgroupExclusiveXor(uint8_t _Value);
u8vec2 subgroupExclusiveXor(u8vec2 _Value);
u8vec3 subgroupExclusiveXor(u8vec3 _Value);
u8vec4 subgroupExclusiveXor(u8vec4 _Value);

int8_t subgroupClusteredAdd(int8_t _Value);
i8vec2 subgroupClusteredAdd(i8vec2 _Value);
i8vec3 subgroupClusteredAdd(i8vec3 _Value);
i8vec4 subgroupClusteredAdd(i8vec4 _Value);
uint8_t subgroupClusteredAdd(uint8_t _Value);
u8vec2 subgroupClusteredAdd(u8vec2 _Value);
u8vec3 subgroupClusteredAdd(u8vec3 _Value);
u8vec4 subgroupClusteredAdd(u8vec4 _Value);

int8_t subgroupClusteredMul(int8_t _Value);
i8vec2 subgroupClusteredMul(i8vec2 _Value);
i8vec3 subgroupClusteredMul(i8vec3 _Value);
i8vec4 subgroupClusteredMul(i8vec4 _Value);
uint8_t subgroupClusteredMul(uint8_t _Value);
u8vec2 subgroupClusteredMul(u8vec2 _Value);
u8vec3 subgroupClusteredMul(u8vec3 _Value);
u8vec4 subgroupClusteredMul(u8vec4 _Value);

int8_t subgroupClusteredMin(int8_t _Value);
i8vec2 subgroupClusteredMin(i8vec2 _Value);
i8vec3 subgroupClusteredMin(i8vec3 _Value);
i8vec4 subgroupClusteredMin(i8vec4 _Value);
uint8_t subgroupClusteredMin(uint8_t _Value);
u8vec2 subgroupClusteredMin(u8vec2 _Value);
u8vec3 subgroupClusteredMin(u8vec3 _Value);
u8vec4 subgroupClusteredMin(u8vec4 _Value);

int8_t subgroupClusteredMax(int8_t _Value);
i8vec2 subgroupClusteredMax(i8vec2 _Value);
i8vec3 subgroupClusteredMax(i8vec3 _Value);
i8vec4 subgroupClusteredMax(i8vec4 _Value);
uint8_t subgroupClusteredMax(uint8_t _Value);
u8vec2 subgroupClusteredMax(u8vec2 _Value);
u8vec3 subgroupClusteredMax(u8vec3 _Value);
u8vec4 subgroupClusteredMax(u8vec4 _Value);

int8_t subgroupClusteredAnd(int8_t _Value);
i8vec2 subgroupClusteredAnd(i8vec2 _Value);
i8vec3 subgroupClusteredAnd(i8vec3 _Value);
i8vec4 subgroupClusteredAnd(i8vec4 _Value);
uint8_t subgroupClusteredAnd(uint8_t _Value);
u8vec2 subgroupClusteredAnd(u8vec2 _Value);
u8vec3 subgroupClusteredAnd(u8vec3 _Value);
u8vec4 subgroupClusteredAnd(u8vec4 _Value);

int8_t subgroupClusteredOr(int8_t _Value);
i8vec2 subgroupClusteredOr(i8vec2 _Value);
i8vec3 subgroupClusteredOr(i8vec3 _Value);
i8vec4 subgroupClusteredOr(i8vec4 _Value);
uint8_t subgroupClusteredOr(uint8_t _Value);
u8vec2 subgroupClusteredOr(u8vec2 _Value);
u8vec3 subgroupClusteredOr(u8vec3 _Value);
u8vec4 subgroupClusteredOr(u8vec4 _Value);

int8_t subgroupClusteredXor(int8_t _Value);
i8vec2 subgroupClusteredXor(i8vec2 _Value);
i8vec3 subgroupClusteredXor(i8vec3 _Value);
i8vec4 subgroupClusteredXor(i8vec4 _Value);
uint8_t subgroupClusteredXor(uint8_t _Value);
u8vec2 subgroupClusteredXor(u8vec2 _Value);
u8vec3 subgroupClusteredXor(u8vec3 _Value);
u8vec4 subgroupClusteredXor(u8vec4 _Value);

int8_t subgroupQuadBroadcast(int8_t _Value, uint _Id);
i8vec2 subgroupQuadBroadcast(i8vec2 _Value, uint _Id);
i8vec3 subgroupQuadBroadcast(i8vec3 _Value, uint _Id);
i8vec4 subgroupQuadBroadcast(i8vec4 _Value, uint _Id);
uint8_t subgroupQuadBroadcast(uint8_t _Value, uint _Id);
u8vec2 subgroupQuadBroadcast(u8vec2 _Value, uint _Id);
u8vec3 subgroupQuadBroadcast(u8vec3 _Value, uint _Id);
u8vec4 subgroupQuadBroadcast(u8vec4 _Value, uint _Id);

int8_t subgroupQuadSwapHorizontal(int8_t _Value);
i8vec2 subgroupQuadSwapHorizontal(i8vec2 _Value);
i8vec3 subgroupQuadSwapHorizontal(i8vec3 _Value);
i8vec4 subgroupQuadSwapHorizontal(i8vec4 _Value);
uint8_t subgroupQuadSwapHorizontal(uint8_t _Value);
u8vec2 subgroupQuadSwapHorizontal(u8vec2 _Value);
u8vec3 subgroupQuadSwapHorizontal(u8vec3 _Value);
u8vec4 subgroupQuadSwapHorizontal(u8vec4 _Value);

int8_t subgroupQuadSwapVertical(int8_t _Value);
i8vec2 subgroupQuadSwapVertical(i8vec2 _Value);
i8vec3 subgroupQuadSwapVertical(i8vec3 _Value);
i8vec4 subgroupQuadSwapVertical(i8vec4 _Value);
uint8_t subgroupQuadSwapVertical(uint8_t _Value);
u8vec2 subgroupQuadSwapVertical(u8vec2 _Value);
u8vec3 subgroupQuadSwapVertical(u8vec3 _Value);
u8vec4 subgroupQuadSwapVertical(u8vec4 _Value);

int8_t subgroupQuadSwapDiagonal(int8_t _Value);
i8vec2 subgroupQuadSwapDiagonal(i8vec2 _Value);
i8vec3 subgroupQuadSwapDiagonal(i8vec3 _Value);
i8vec4 subgroupQuadSwapDiagonal(i8vec4 _Value);
uint8_t subgroupQuadSwapDiagonal(uint8_t _Value);
u8vec2 subgroupQuadSwapDiagonal(u8vec2 _Value);
u8vec3 subgroupQuadSwapDiagonal(u8vec3 _Value);
u8vec4 subgroupQuadSwapDiagonal(u8vec4 _Value);

int8_t subgroupPartitionedAddNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedAddNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedAddNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedAddNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedAddNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedAddNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedAddNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedAddNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedMulNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedMulNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedMulNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedMulNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedMulNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedMulNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedMulNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedMulNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedMinNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedMinNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedMinNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedMinNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedMinNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedMinNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedMinNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedMinNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedMaxNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedMaxNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedMaxNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedMaxNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedMaxNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedMaxNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedMaxNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedMaxNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedAndNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedAndNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedAndNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedAndNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedAndNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedAndNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedAndNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedAndNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedOrNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedOrNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedOrNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedOrNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedOrNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedOrNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedOrNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedOrNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedXorNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedXorNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedXorNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedXorNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedXorNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedXorNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedXorNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedXorNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveAddNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveAddNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveAddNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveAddNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveAddNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveAddNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveAddNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveAddNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveMulNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveMulNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveMulNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveMulNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveMulNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveMulNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveMulNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveMulNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveMinNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveMinNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveMinNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveMinNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveMinNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveMinNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveMinNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveMinNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveMaxNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveMaxNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveMaxNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveMaxNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveMaxNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveMaxNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveMaxNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveMaxNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveAndNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveAndNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveAndNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveAndNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveAndNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveAndNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveAndNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveAndNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveOrNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveOrNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveOrNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveOrNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveOrNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveOrNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveOrNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveOrNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedInclusiveXorNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedInclusiveXorNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedInclusiveXorNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedInclusiveXorNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedInclusiveXorNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedInclusiveXorNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedInclusiveXorNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedInclusiveXorNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveAddNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveAddNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveAddNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveAddNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveAddNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveAddNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveAddNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveAddNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveMulNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveMulNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveMulNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveMulNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveMulNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveMulNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveMulNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveMulNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveMinNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveMinNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveMinNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveMinNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveMinNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveMinNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveMinNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveMinNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveMaxNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveMaxNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveMaxNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveMaxNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveMaxNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveMaxNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveMaxNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveMaxNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveAndNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveAndNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveAndNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveAndNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveAndNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveAndNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveAndNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveAndNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveOrNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveOrNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveOrNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveOrNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveOrNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveOrNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveOrNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveOrNV(u8vec4 _Value, uvec4 _Ballot);

int8_t subgroupPartitionedExclusiveXorNV(int8_t _Value, uvec4 _Ballot);
i8vec2 subgroupPartitionedExclusiveXorNV(i8vec2 _Value, uvec4 _Ballot);
i8vec3 subgroupPartitionedExclusiveXorNV(i8vec3 _Value, uvec4 _Ballot);
i8vec4 subgroupPartitionedExclusiveXorNV(i8vec4 _Value, uvec4 _Ballot);
uint8_t subgroupPartitionedExclusiveXorNV(uint8_t _Value, uvec4 _Ballot);
u8vec2 subgroupPartitionedExclusiveXorNV(u8vec2 _Value, uvec4 _Ballot);
u8vec3 subgroupPartitionedExclusiveXorNV(u8vec3 _Value, uvec4 _Ballot);
u8vec4 subgroupPartitionedExclusiveXorNV(u8vec4 _Value, uvec4 _Ballot);

uvec4 subgroupPartitionNV(int8_t _Value);
uvec4 subgroupPartitionNV(i8vec2 _Value);
uvec4 subgroupPartitionNV(i8vec3 _Value);
uvec4 subgroupPartitionNV(i8vec4 _Value);
uvec4 subgroupPartitionNV(uint8_t _Value);
uvec4 subgroupPartitionNV(u8vec2 _Value);
uvec4 subgroupPartitionNV(u8vec3 _Value);
uvec4 subgroupPartitionNV(u8vec4 _Value);
