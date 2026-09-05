#define GL_EXT_shader_subgroup_extended_types_int64 1

// GL_EXT_shader_subgroup_extended_types_int64 (I64/U64)
// GLSL_EXT_shader_subgroup_extended_types: genI64/U64 overloads of subgroup built-ins,
// expanded over the scalar + vec2/3/4 forms of each gen type.
// Note: Clustered rows follow this spec revision verbatim (single parameter; later revisions add the cluster size argument).

bool subgroupAllEqual(int64_t _Value);
bool subgroupAllEqual(i64vec2 _Value);
bool subgroupAllEqual(i64vec3 _Value);
bool subgroupAllEqual(i64vec4 _Value);
bool subgroupAllEqual(uint64_t _Value);
bool subgroupAllEqual(u64vec2 _Value);
bool subgroupAllEqual(u64vec3 _Value);
bool subgroupAllEqual(u64vec4 _Value);

int64_t subgroupBroadcast(int64_t _Value, uint _Id);
i64vec2 subgroupBroadcast(i64vec2 _Value, uint _Id);
i64vec3 subgroupBroadcast(i64vec3 _Value, uint _Id);
i64vec4 subgroupBroadcast(i64vec4 _Value, uint _Id);
uint64_t subgroupBroadcast(uint64_t _Value, uint _Id);
u64vec2 subgroupBroadcast(u64vec2 _Value, uint _Id);
u64vec3 subgroupBroadcast(u64vec3 _Value, uint _Id);
u64vec4 subgroupBroadcast(u64vec4 _Value, uint _Id);

int64_t subgroupBroadcastFirst(int64_t _Value);
i64vec2 subgroupBroadcastFirst(i64vec2 _Value);
i64vec3 subgroupBroadcastFirst(i64vec3 _Value);
i64vec4 subgroupBroadcastFirst(i64vec4 _Value);
uint64_t subgroupBroadcastFirst(uint64_t _Value);
u64vec2 subgroupBroadcastFirst(u64vec2 _Value);
u64vec3 subgroupBroadcastFirst(u64vec3 _Value);
u64vec4 subgroupBroadcastFirst(u64vec4 _Value);

int64_t subgroupShuffle(int64_t _Value, uint _Id);
i64vec2 subgroupShuffle(i64vec2 _Value, uint _Id);
i64vec3 subgroupShuffle(i64vec3 _Value, uint _Id);
i64vec4 subgroupShuffle(i64vec4 _Value, uint _Id);
uint64_t subgroupShuffle(uint64_t _Value, uint _Id);
u64vec2 subgroupShuffle(u64vec2 _Value, uint _Id);
u64vec3 subgroupShuffle(u64vec3 _Value, uint _Id);
u64vec4 subgroupShuffle(u64vec4 _Value, uint _Id);

int64_t subgroupShuffleXor(int64_t _Value, uint _Mask);
i64vec2 subgroupShuffleXor(i64vec2 _Value, uint _Mask);
i64vec3 subgroupShuffleXor(i64vec3 _Value, uint _Mask);
i64vec4 subgroupShuffleXor(i64vec4 _Value, uint _Mask);
uint64_t subgroupShuffleXor(uint64_t _Value, uint _Mask);
u64vec2 subgroupShuffleXor(u64vec2 _Value, uint _Mask);
u64vec3 subgroupShuffleXor(u64vec3 _Value, uint _Mask);
u64vec4 subgroupShuffleXor(u64vec4 _Value, uint _Mask);

int64_t subgroupShuffleUp(int64_t _Value, uint _Delta);
i64vec2 subgroupShuffleUp(i64vec2 _Value, uint _Delta);
i64vec3 subgroupShuffleUp(i64vec3 _Value, uint _Delta);
i64vec4 subgroupShuffleUp(i64vec4 _Value, uint _Delta);
uint64_t subgroupShuffleUp(uint64_t _Value, uint _Delta);
u64vec2 subgroupShuffleUp(u64vec2 _Value, uint _Delta);
u64vec3 subgroupShuffleUp(u64vec3 _Value, uint _Delta);
u64vec4 subgroupShuffleUp(u64vec4 _Value, uint _Delta);

int64_t subgroupShuffleDown(int64_t _Value, uint _Delta);
i64vec2 subgroupShuffleDown(i64vec2 _Value, uint _Delta);
i64vec3 subgroupShuffleDown(i64vec3 _Value, uint _Delta);
i64vec4 subgroupShuffleDown(i64vec4 _Value, uint _Delta);
uint64_t subgroupShuffleDown(uint64_t _Value, uint _Delta);
u64vec2 subgroupShuffleDown(u64vec2 _Value, uint _Delta);
u64vec3 subgroupShuffleDown(u64vec3 _Value, uint _Delta);
u64vec4 subgroupShuffleDown(u64vec4 _Value, uint _Delta);

int64_t subgroupAdd(int64_t _Value);
i64vec2 subgroupAdd(i64vec2 _Value);
i64vec3 subgroupAdd(i64vec3 _Value);
i64vec4 subgroupAdd(i64vec4 _Value);
uint64_t subgroupAdd(uint64_t _Value);
u64vec2 subgroupAdd(u64vec2 _Value);
u64vec3 subgroupAdd(u64vec3 _Value);
u64vec4 subgroupAdd(u64vec4 _Value);

int64_t subgroupMul(int64_t _Value);
i64vec2 subgroupMul(i64vec2 _Value);
i64vec3 subgroupMul(i64vec3 _Value);
i64vec4 subgroupMul(i64vec4 _Value);
uint64_t subgroupMul(uint64_t _Value);
u64vec2 subgroupMul(u64vec2 _Value);
u64vec3 subgroupMul(u64vec3 _Value);
u64vec4 subgroupMul(u64vec4 _Value);

int64_t subgroupMin(int64_t _Value);
i64vec2 subgroupMin(i64vec2 _Value);
i64vec3 subgroupMin(i64vec3 _Value);
i64vec4 subgroupMin(i64vec4 _Value);
uint64_t subgroupMin(uint64_t _Value);
u64vec2 subgroupMin(u64vec2 _Value);
u64vec3 subgroupMin(u64vec3 _Value);
u64vec4 subgroupMin(u64vec4 _Value);

int64_t subgroupMax(int64_t _Value);
i64vec2 subgroupMax(i64vec2 _Value);
i64vec3 subgroupMax(i64vec3 _Value);
i64vec4 subgroupMax(i64vec4 _Value);
uint64_t subgroupMax(uint64_t _Value);
u64vec2 subgroupMax(u64vec2 _Value);
u64vec3 subgroupMax(u64vec3 _Value);
u64vec4 subgroupMax(u64vec4 _Value);

int64_t subgroupAnd(int64_t _Value);
i64vec2 subgroupAnd(i64vec2 _Value);
i64vec3 subgroupAnd(i64vec3 _Value);
i64vec4 subgroupAnd(i64vec4 _Value);
uint64_t subgroupAnd(uint64_t _Value);
u64vec2 subgroupAnd(u64vec2 _Value);
u64vec3 subgroupAnd(u64vec3 _Value);
u64vec4 subgroupAnd(u64vec4 _Value);

int64_t subgroupOr(int64_t _Value);
i64vec2 subgroupOr(i64vec2 _Value);
i64vec3 subgroupOr(i64vec3 _Value);
i64vec4 subgroupOr(i64vec4 _Value);
uint64_t subgroupOr(uint64_t _Value);
u64vec2 subgroupOr(u64vec2 _Value);
u64vec3 subgroupOr(u64vec3 _Value);
u64vec4 subgroupOr(u64vec4 _Value);

int64_t subgroupXor(int64_t _Value);
i64vec2 subgroupXor(i64vec2 _Value);
i64vec3 subgroupXor(i64vec3 _Value);
i64vec4 subgroupXor(i64vec4 _Value);
uint64_t subgroupXor(uint64_t _Value);
u64vec2 subgroupXor(u64vec2 _Value);
u64vec3 subgroupXor(u64vec3 _Value);
u64vec4 subgroupXor(u64vec4 _Value);

int64_t subgroupInclusiveAdd(int64_t _Value);
i64vec2 subgroupInclusiveAdd(i64vec2 _Value);
i64vec3 subgroupInclusiveAdd(i64vec3 _Value);
i64vec4 subgroupInclusiveAdd(i64vec4 _Value);
uint64_t subgroupInclusiveAdd(uint64_t _Value);
u64vec2 subgroupInclusiveAdd(u64vec2 _Value);
u64vec3 subgroupInclusiveAdd(u64vec3 _Value);
u64vec4 subgroupInclusiveAdd(u64vec4 _Value);

int64_t subgroupInclusiveMul(int64_t _Value);
i64vec2 subgroupInclusiveMul(i64vec2 _Value);
i64vec3 subgroupInclusiveMul(i64vec3 _Value);
i64vec4 subgroupInclusiveMul(i64vec4 _Value);
uint64_t subgroupInclusiveMul(uint64_t _Value);
u64vec2 subgroupInclusiveMul(u64vec2 _Value);
u64vec3 subgroupInclusiveMul(u64vec3 _Value);
u64vec4 subgroupInclusiveMul(u64vec4 _Value);

int64_t subgroupInclusiveMin(int64_t _Value);
i64vec2 subgroupInclusiveMin(i64vec2 _Value);
i64vec3 subgroupInclusiveMin(i64vec3 _Value);
i64vec4 subgroupInclusiveMin(i64vec4 _Value);
uint64_t subgroupInclusiveMin(uint64_t _Value);
u64vec2 subgroupInclusiveMin(u64vec2 _Value);
u64vec3 subgroupInclusiveMin(u64vec3 _Value);
u64vec4 subgroupInclusiveMin(u64vec4 _Value);

int64_t subgroupInclusiveMax(int64_t _Value);
i64vec2 subgroupInclusiveMax(i64vec2 _Value);
i64vec3 subgroupInclusiveMax(i64vec3 _Value);
i64vec4 subgroupInclusiveMax(i64vec4 _Value);
uint64_t subgroupInclusiveMax(uint64_t _Value);
u64vec2 subgroupInclusiveMax(u64vec2 _Value);
u64vec3 subgroupInclusiveMax(u64vec3 _Value);
u64vec4 subgroupInclusiveMax(u64vec4 _Value);

int64_t subgroupInclusiveAnd(int64_t _Value);
i64vec2 subgroupInclusiveAnd(i64vec2 _Value);
i64vec3 subgroupInclusiveAnd(i64vec3 _Value);
i64vec4 subgroupInclusiveAnd(i64vec4 _Value);
uint64_t subgroupInclusiveAnd(uint64_t _Value);
u64vec2 subgroupInclusiveAnd(u64vec2 _Value);
u64vec3 subgroupInclusiveAnd(u64vec3 _Value);
u64vec4 subgroupInclusiveAnd(u64vec4 _Value);

int64_t subgroupInclusiveOr(int64_t _Value);
i64vec2 subgroupInclusiveOr(i64vec2 _Value);
i64vec3 subgroupInclusiveOr(i64vec3 _Value);
i64vec4 subgroupInclusiveOr(i64vec4 _Value);
uint64_t subgroupInclusiveOr(uint64_t _Value);
u64vec2 subgroupInclusiveOr(u64vec2 _Value);
u64vec3 subgroupInclusiveOr(u64vec3 _Value);
u64vec4 subgroupInclusiveOr(u64vec4 _Value);

int64_t subgroupInclusiveXor(int64_t _Value);
i64vec2 subgroupInclusiveXor(i64vec2 _Value);
i64vec3 subgroupInclusiveXor(i64vec3 _Value);
i64vec4 subgroupInclusiveXor(i64vec4 _Value);
uint64_t subgroupInclusiveXor(uint64_t _Value);
u64vec2 subgroupInclusiveXor(u64vec2 _Value);
u64vec3 subgroupInclusiveXor(u64vec3 _Value);
u64vec4 subgroupInclusiveXor(u64vec4 _Value);

int64_t subgroupExclusiveAdd(int64_t _Value);
i64vec2 subgroupExclusiveAdd(i64vec2 _Value);
i64vec3 subgroupExclusiveAdd(i64vec3 _Value);
i64vec4 subgroupExclusiveAdd(i64vec4 _Value);
uint64_t subgroupExclusiveAdd(uint64_t _Value);
u64vec2 subgroupExclusiveAdd(u64vec2 _Value);
u64vec3 subgroupExclusiveAdd(u64vec3 _Value);
u64vec4 subgroupExclusiveAdd(u64vec4 _Value);

int64_t subgroupExclusiveMul(int64_t _Value);
i64vec2 subgroupExclusiveMul(i64vec2 _Value);
i64vec3 subgroupExclusiveMul(i64vec3 _Value);
i64vec4 subgroupExclusiveMul(i64vec4 _Value);
uint64_t subgroupExclusiveMul(uint64_t _Value);
u64vec2 subgroupExclusiveMul(u64vec2 _Value);
u64vec3 subgroupExclusiveMul(u64vec3 _Value);
u64vec4 subgroupExclusiveMul(u64vec4 _Value);

int64_t subgroupExclusiveMin(int64_t _Value);
i64vec2 subgroupExclusiveMin(i64vec2 _Value);
i64vec3 subgroupExclusiveMin(i64vec3 _Value);
i64vec4 subgroupExclusiveMin(i64vec4 _Value);
uint64_t subgroupExclusiveMin(uint64_t _Value);
u64vec2 subgroupExclusiveMin(u64vec2 _Value);
u64vec3 subgroupExclusiveMin(u64vec3 _Value);
u64vec4 subgroupExclusiveMin(u64vec4 _Value);

int64_t subgroupExclusiveMax(int64_t _Value);
i64vec2 subgroupExclusiveMax(i64vec2 _Value);
i64vec3 subgroupExclusiveMax(i64vec3 _Value);
i64vec4 subgroupExclusiveMax(i64vec4 _Value);
uint64_t subgroupExclusiveMax(uint64_t _Value);
u64vec2 subgroupExclusiveMax(u64vec2 _Value);
u64vec3 subgroupExclusiveMax(u64vec3 _Value);
u64vec4 subgroupExclusiveMax(u64vec4 _Value);

int64_t subgroupExclusiveAnd(int64_t _Value);
i64vec2 subgroupExclusiveAnd(i64vec2 _Value);
i64vec3 subgroupExclusiveAnd(i64vec3 _Value);
i64vec4 subgroupExclusiveAnd(i64vec4 _Value);
uint64_t subgroupExclusiveAnd(uint64_t _Value);
u64vec2 subgroupExclusiveAnd(u64vec2 _Value);
u64vec3 subgroupExclusiveAnd(u64vec3 _Value);
u64vec4 subgroupExclusiveAnd(u64vec4 _Value);

int64_t subgroupExclusiveOr(int64_t _Value);
i64vec2 subgroupExclusiveOr(i64vec2 _Value);
i64vec3 subgroupExclusiveOr(i64vec3 _Value);
i64vec4 subgroupExclusiveOr(i64vec4 _Value);
uint64_t subgroupExclusiveOr(uint64_t _Value);
u64vec2 subgroupExclusiveOr(u64vec2 _Value);
u64vec3 subgroupExclusiveOr(u64vec3 _Value);
u64vec4 subgroupExclusiveOr(u64vec4 _Value);

int64_t subgroupExclusiveXor(int64_t _Value);
i64vec2 subgroupExclusiveXor(i64vec2 _Value);
i64vec3 subgroupExclusiveXor(i64vec3 _Value);
i64vec4 subgroupExclusiveXor(i64vec4 _Value);
uint64_t subgroupExclusiveXor(uint64_t _Value);
u64vec2 subgroupExclusiveXor(u64vec2 _Value);
u64vec3 subgroupExclusiveXor(u64vec3 _Value);
u64vec4 subgroupExclusiveXor(u64vec4 _Value);

int64_t subgroupClusteredAdd(int64_t _Value);
i64vec2 subgroupClusteredAdd(i64vec2 _Value);
i64vec3 subgroupClusteredAdd(i64vec3 _Value);
i64vec4 subgroupClusteredAdd(i64vec4 _Value);
uint64_t subgroupClusteredAdd(uint64_t _Value);
u64vec2 subgroupClusteredAdd(u64vec2 _Value);
u64vec3 subgroupClusteredAdd(u64vec3 _Value);
u64vec4 subgroupClusteredAdd(u64vec4 _Value);

int64_t subgroupClusteredMul(int64_t _Value);
i64vec2 subgroupClusteredMul(i64vec2 _Value);
i64vec3 subgroupClusteredMul(i64vec3 _Value);
i64vec4 subgroupClusteredMul(i64vec4 _Value);
uint64_t subgroupClusteredMul(uint64_t _Value);
u64vec2 subgroupClusteredMul(u64vec2 _Value);
u64vec3 subgroupClusteredMul(u64vec3 _Value);
u64vec4 subgroupClusteredMul(u64vec4 _Value);

int64_t subgroupClusteredMin(int64_t _Value);
i64vec2 subgroupClusteredMin(i64vec2 _Value);
i64vec3 subgroupClusteredMin(i64vec3 _Value);
i64vec4 subgroupClusteredMin(i64vec4 _Value);
uint64_t subgroupClusteredMin(uint64_t _Value);
u64vec2 subgroupClusteredMin(u64vec2 _Value);
u64vec3 subgroupClusteredMin(u64vec3 _Value);
u64vec4 subgroupClusteredMin(u64vec4 _Value);

int64_t subgroupClusteredMax(int64_t _Value);
i64vec2 subgroupClusteredMax(i64vec2 _Value);
i64vec3 subgroupClusteredMax(i64vec3 _Value);
i64vec4 subgroupClusteredMax(i64vec4 _Value);
uint64_t subgroupClusteredMax(uint64_t _Value);
u64vec2 subgroupClusteredMax(u64vec2 _Value);
u64vec3 subgroupClusteredMax(u64vec3 _Value);
u64vec4 subgroupClusteredMax(u64vec4 _Value);

int64_t subgroupClusteredAnd(int64_t _Value);
i64vec2 subgroupClusteredAnd(i64vec2 _Value);
i64vec3 subgroupClusteredAnd(i64vec3 _Value);
i64vec4 subgroupClusteredAnd(i64vec4 _Value);
uint64_t subgroupClusteredAnd(uint64_t _Value);
u64vec2 subgroupClusteredAnd(u64vec2 _Value);
u64vec3 subgroupClusteredAnd(u64vec3 _Value);
u64vec4 subgroupClusteredAnd(u64vec4 _Value);

int64_t subgroupClusteredOr(int64_t _Value);
i64vec2 subgroupClusteredOr(i64vec2 _Value);
i64vec3 subgroupClusteredOr(i64vec3 _Value);
i64vec4 subgroupClusteredOr(i64vec4 _Value);
uint64_t subgroupClusteredOr(uint64_t _Value);
u64vec2 subgroupClusteredOr(u64vec2 _Value);
u64vec3 subgroupClusteredOr(u64vec3 _Value);
u64vec4 subgroupClusteredOr(u64vec4 _Value);

int64_t subgroupClusteredXor(int64_t _Value);
i64vec2 subgroupClusteredXor(i64vec2 _Value);
i64vec3 subgroupClusteredXor(i64vec3 _Value);
i64vec4 subgroupClusteredXor(i64vec4 _Value);
uint64_t subgroupClusteredXor(uint64_t _Value);
u64vec2 subgroupClusteredXor(u64vec2 _Value);
u64vec3 subgroupClusteredXor(u64vec3 _Value);
u64vec4 subgroupClusteredXor(u64vec4 _Value);

int64_t subgroupQuadBroadcast(int64_t _Value, uint _Id);
i64vec2 subgroupQuadBroadcast(i64vec2 _Value, uint _Id);
i64vec3 subgroupQuadBroadcast(i64vec3 _Value, uint _Id);
i64vec4 subgroupQuadBroadcast(i64vec4 _Value, uint _Id);
uint64_t subgroupQuadBroadcast(uint64_t _Value, uint _Id);
u64vec2 subgroupQuadBroadcast(u64vec2 _Value, uint _Id);
u64vec3 subgroupQuadBroadcast(u64vec3 _Value, uint _Id);
u64vec4 subgroupQuadBroadcast(u64vec4 _Value, uint _Id);

int64_t subgroupQuadSwapHorizontal(int64_t _Value);
i64vec2 subgroupQuadSwapHorizontal(i64vec2 _Value);
i64vec3 subgroupQuadSwapHorizontal(i64vec3 _Value);
i64vec4 subgroupQuadSwapHorizontal(i64vec4 _Value);
uint64_t subgroupQuadSwapHorizontal(uint64_t _Value);
u64vec2 subgroupQuadSwapHorizontal(u64vec2 _Value);
u64vec3 subgroupQuadSwapHorizontal(u64vec3 _Value);
u64vec4 subgroupQuadSwapHorizontal(u64vec4 _Value);

int64_t subgroupQuadSwapVertical(int64_t _Value);
i64vec2 subgroupQuadSwapVertical(i64vec2 _Value);
i64vec3 subgroupQuadSwapVertical(i64vec3 _Value);
i64vec4 subgroupQuadSwapVertical(i64vec4 _Value);
uint64_t subgroupQuadSwapVertical(uint64_t _Value);
u64vec2 subgroupQuadSwapVertical(u64vec2 _Value);
u64vec3 subgroupQuadSwapVertical(u64vec3 _Value);
u64vec4 subgroupQuadSwapVertical(u64vec4 _Value);

int64_t subgroupQuadSwapDiagonal(int64_t _Value);
i64vec2 subgroupQuadSwapDiagonal(i64vec2 _Value);
i64vec3 subgroupQuadSwapDiagonal(i64vec3 _Value);
i64vec4 subgroupQuadSwapDiagonal(i64vec4 _Value);
uint64_t subgroupQuadSwapDiagonal(uint64_t _Value);
u64vec2 subgroupQuadSwapDiagonal(u64vec2 _Value);
u64vec3 subgroupQuadSwapDiagonal(u64vec3 _Value);
u64vec4 subgroupQuadSwapDiagonal(u64vec4 _Value);

int64_t subgroupPartitionedAddNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedAddNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedAddNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedAddNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedAddNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedAddNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedAddNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedAddNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedMulNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedMulNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedMulNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedMulNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedMulNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedMulNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedMulNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedMulNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedMinNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedMinNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedMinNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedMinNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedMinNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedMinNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedMinNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedMinNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedMaxNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedMaxNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedMaxNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedMaxNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedMaxNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedMaxNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedMaxNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedMaxNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedAndNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedAndNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedAndNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedAndNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedAndNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedAndNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedAndNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedAndNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedOrNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedOrNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedOrNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedOrNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedOrNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedOrNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedOrNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedOrNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedXorNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedXorNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedXorNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedXorNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedXorNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedXorNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedXorNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedXorNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveAddNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveAddNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveAddNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveAddNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveAddNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveAddNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveAddNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveAddNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveMulNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveMulNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveMulNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveMulNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveMulNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveMulNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveMulNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveMulNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveMinNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveMinNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveMinNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveMinNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveMinNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveMinNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveMinNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveMinNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveMaxNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveMaxNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveMaxNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveMaxNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveMaxNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveMaxNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveMaxNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveMaxNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveAndNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveAndNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveAndNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveAndNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveAndNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveAndNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveAndNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveAndNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveOrNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveOrNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveOrNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveOrNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveOrNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveOrNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveOrNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveOrNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedInclusiveXorNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedInclusiveXorNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedInclusiveXorNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedInclusiveXorNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedInclusiveXorNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedInclusiveXorNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedInclusiveXorNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedInclusiveXorNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveAddNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveAddNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveAddNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveAddNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveAddNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveAddNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveAddNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveAddNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveMulNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveMulNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveMulNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveMulNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveMulNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveMulNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveMulNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveMulNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveMinNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveMinNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveMinNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveMinNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveMinNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveMinNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveMinNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveMinNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveMaxNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveMaxNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveMaxNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveMaxNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveMaxNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveMaxNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveMaxNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveMaxNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveAndNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveAndNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveAndNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveAndNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveAndNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveAndNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveAndNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveAndNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveOrNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveOrNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveOrNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveOrNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveOrNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveOrNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveOrNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveOrNV(u64vec4 _Value, uvec4 _Ballot);

int64_t subgroupPartitionedExclusiveXorNV(int64_t _Value, uvec4 _Ballot);
i64vec2 subgroupPartitionedExclusiveXorNV(i64vec2 _Value, uvec4 _Ballot);
i64vec3 subgroupPartitionedExclusiveXorNV(i64vec3 _Value, uvec4 _Ballot);
i64vec4 subgroupPartitionedExclusiveXorNV(i64vec4 _Value, uvec4 _Ballot);
uint64_t subgroupPartitionedExclusiveXorNV(uint64_t _Value, uvec4 _Ballot);
u64vec2 subgroupPartitionedExclusiveXorNV(u64vec2 _Value, uvec4 _Ballot);
u64vec3 subgroupPartitionedExclusiveXorNV(u64vec3 _Value, uvec4 _Ballot);
u64vec4 subgroupPartitionedExclusiveXorNV(u64vec4 _Value, uvec4 _Ballot);

uvec4 subgroupPartitionNV(int64_t _Value);
uvec4 subgroupPartitionNV(i64vec2 _Value);
uvec4 subgroupPartitionNV(i64vec3 _Value);
uvec4 subgroupPartitionNV(i64vec4 _Value);
uvec4 subgroupPartitionNV(uint64_t _Value);
uvec4 subgroupPartitionNV(u64vec2 _Value);
uvec4 subgroupPartitionNV(u64vec3 _Value);
uvec4 subgroupPartitionNV(u64vec4 _Value);
