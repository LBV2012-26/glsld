#define GL_EXT_shader_subgroup_extended_types_float16 1

// GL_EXT_shader_subgroup_extended_types_float16 (F16)
// GLSL_EXT_shader_subgroup_extended_types: genF16 overloads of subgroup built-ins,
// expanded over the scalar + vec2/3/4 forms of each gen type.

bool subgroupAllEqual(float16_t _Value);
bool subgroupAllEqual(f16vec2 _Value);
bool subgroupAllEqual(f16vec3 _Value);
bool subgroupAllEqual(f16vec4 _Value);

float16_t subgroupBroadcast(float16_t _Value, uint _Id);
f16vec2 subgroupBroadcast(f16vec2 _Value, uint _Id);
f16vec3 subgroupBroadcast(f16vec3 _Value, uint _Id);
f16vec4 subgroupBroadcast(f16vec4 _Value, uint _Id);

float16_t subgroupBroadcastFirst(float16_t _Value);
f16vec2 subgroupBroadcastFirst(f16vec2 _Value);
f16vec3 subgroupBroadcastFirst(f16vec3 _Value);
f16vec4 subgroupBroadcastFirst(f16vec4 _Value);

float16_t subgroupShuffle(float16_t _Value, uint _Id);
f16vec2 subgroupShuffle(f16vec2 _Value, uint _Id);
f16vec3 subgroupShuffle(f16vec3 _Value, uint _Id);
f16vec4 subgroupShuffle(f16vec4 _Value, uint _Id);

float16_t subgroupShuffleXor(float16_t _Value, uint _Mask);
f16vec2 subgroupShuffleXor(f16vec2 _Value, uint _Mask);
f16vec3 subgroupShuffleXor(f16vec3 _Value, uint _Mask);
f16vec4 subgroupShuffleXor(f16vec4 _Value, uint _Mask);

float16_t subgroupShuffleUp(float16_t _Value, uint _Delta);
f16vec2 subgroupShuffleUp(f16vec2 _Value, uint _Delta);
f16vec3 subgroupShuffleUp(f16vec3 _Value, uint _Delta);
f16vec4 subgroupShuffleUp(f16vec4 _Value, uint _Delta);

float16_t subgroupShuffleDown(float16_t _Value, uint _Delta);
f16vec2 subgroupShuffleDown(f16vec2 _Value, uint _Delta);
f16vec3 subgroupShuffleDown(f16vec3 _Value, uint _Delta);
f16vec4 subgroupShuffleDown(f16vec4 _Value, uint _Delta);

float16_t subgroupAdd(float16_t _Value);
f16vec2 subgroupAdd(f16vec2 _Value);
f16vec3 subgroupAdd(f16vec3 _Value);
f16vec4 subgroupAdd(f16vec4 _Value);

float16_t subgroupMul(float16_t _Value);
f16vec2 subgroupMul(f16vec2 _Value);
f16vec3 subgroupMul(f16vec3 _Value);
f16vec4 subgroupMul(f16vec4 _Value);

float16_t subgroupMin(float16_t _Value);
f16vec2 subgroupMin(f16vec2 _Value);
f16vec3 subgroupMin(f16vec3 _Value);
f16vec4 subgroupMin(f16vec4 _Value);

float16_t subgroupMax(float16_t _Value);
f16vec2 subgroupMax(f16vec2 _Value);
f16vec3 subgroupMax(f16vec3 _Value);
f16vec4 subgroupMax(f16vec4 _Value);

float16_t subgroupInclusiveAdd(float16_t _Value);
f16vec2 subgroupInclusiveAdd(f16vec2 _Value);
f16vec3 subgroupInclusiveAdd(f16vec3 _Value);
f16vec4 subgroupInclusiveAdd(f16vec4 _Value);

float16_t subgroupInclusiveMul(float16_t _Value);
f16vec2 subgroupInclusiveMul(f16vec2 _Value);
f16vec3 subgroupInclusiveMul(f16vec3 _Value);
f16vec4 subgroupInclusiveMul(f16vec4 _Value);

float16_t subgroupInclusiveMin(float16_t _Value);
f16vec2 subgroupInclusiveMin(f16vec2 _Value);
f16vec3 subgroupInclusiveMin(f16vec3 _Value);
f16vec4 subgroupInclusiveMin(f16vec4 _Value);

float16_t subgroupInclusiveMax(float16_t _Value);
f16vec2 subgroupInclusiveMax(f16vec2 _Value);
f16vec3 subgroupInclusiveMax(f16vec3 _Value);
f16vec4 subgroupInclusiveMax(f16vec4 _Value);

float16_t subgroupExclusiveAdd(float16_t _Value);
f16vec2 subgroupExclusiveAdd(f16vec2 _Value);
f16vec3 subgroupExclusiveAdd(f16vec3 _Value);
f16vec4 subgroupExclusiveAdd(f16vec4 _Value);

float16_t subgroupExclusiveMul(float16_t _Value);
f16vec2 subgroupExclusiveMul(f16vec2 _Value);
f16vec3 subgroupExclusiveMul(f16vec3 _Value);
f16vec4 subgroupExclusiveMul(f16vec4 _Value);

float16_t subgroupExclusiveMin(float16_t _Value);
f16vec2 subgroupExclusiveMin(f16vec2 _Value);
f16vec3 subgroupExclusiveMin(f16vec3 _Value);
f16vec4 subgroupExclusiveMin(f16vec4 _Value);

float16_t subgroupExclusiveMax(float16_t _Value);
f16vec2 subgroupExclusiveMax(f16vec2 _Value);
f16vec3 subgroupExclusiveMax(f16vec3 _Value);
f16vec4 subgroupExclusiveMax(f16vec4 _Value);

float16_t subgroupClusteredAdd(float16_t _Value);
f16vec2 subgroupClusteredAdd(f16vec2 _Value);
f16vec3 subgroupClusteredAdd(f16vec3 _Value);
f16vec4 subgroupClusteredAdd(f16vec4 _Value);

float16_t subgroupClusteredMul(float16_t _Value);
f16vec2 subgroupClusteredMul(f16vec2 _Value);
f16vec3 subgroupClusteredMul(f16vec3 _Value);
f16vec4 subgroupClusteredMul(f16vec4 _Value);

float16_t subgroupClusteredMin(float16_t _Value);
f16vec2 subgroupClusteredMin(f16vec2 _Value);
f16vec3 subgroupClusteredMin(f16vec3 _Value);
f16vec4 subgroupClusteredMin(f16vec4 _Value);

float16_t subgroupClusteredMax(float16_t _Value);
f16vec2 subgroupClusteredMax(f16vec2 _Value);
f16vec3 subgroupClusteredMax(f16vec3 _Value);
f16vec4 subgroupClusteredMax(f16vec4 _Value);

float16_t subgroupQuadBroadcast(float16_t _Value, uint _Id);
f16vec2 subgroupQuadBroadcast(f16vec2 _Value, uint _Id);
f16vec3 subgroupQuadBroadcast(f16vec3 _Value, uint _Id);
f16vec4 subgroupQuadBroadcast(f16vec4 _Value, uint _Id);

float16_t subgroupQuadSwapHorizontal(float16_t _Value);
f16vec2 subgroupQuadSwapHorizontal(f16vec2 _Value);
f16vec3 subgroupQuadSwapHorizontal(f16vec3 _Value);
f16vec4 subgroupQuadSwapHorizontal(f16vec4 _Value);

float16_t subgroupQuadSwapVertical(float16_t _Value);
f16vec2 subgroupQuadSwapVertical(f16vec2 _Value);
f16vec3 subgroupQuadSwapVertical(f16vec3 _Value);
f16vec4 subgroupQuadSwapVertical(f16vec4 _Value);

float16_t subgroupQuadSwapDiagonal(float16_t _Value);
f16vec2 subgroupQuadSwapDiagonal(f16vec2 _Value);
f16vec3 subgroupQuadSwapDiagonal(f16vec3 _Value);
f16vec4 subgroupQuadSwapDiagonal(f16vec4 _Value);

float16_t subgroupPartitionedAddNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedAddNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedAddNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedAddNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedMulNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedMulNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedMulNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedMulNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedMinNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedMinNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedMinNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedMinNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedMaxNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedMaxNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedMaxNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedMaxNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedInclusiveAddNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedInclusiveAddNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedInclusiveAddNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedInclusiveAddNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedInclusiveMulNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedInclusiveMulNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedInclusiveMulNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedInclusiveMulNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedInclusiveMinNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedInclusiveMinNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedInclusiveMinNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedInclusiveMinNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedInclusiveMaxNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedInclusiveMaxNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedInclusiveMaxNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedInclusiveMaxNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedExclusiveAddNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedExclusiveAddNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedExclusiveAddNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedExclusiveAddNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedExclusiveMulNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedExclusiveMulNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedExclusiveMulNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedExclusiveMulNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedExclusiveMinNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedExclusiveMinNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedExclusiveMinNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedExclusiveMinNV(f16vec4 _Value, uvec4 _Ballot);

float16_t subgroupPartitionedExclusiveMaxNV(float16_t _Value, uvec4 _Ballot);
f16vec2 subgroupPartitionedExclusiveMaxNV(f16vec2 _Value, uvec4 _Ballot);
f16vec3 subgroupPartitionedExclusiveMaxNV(f16vec3 _Value, uvec4 _Ballot);
f16vec4 subgroupPartitionedExclusiveMaxNV(f16vec4 _Value, uvec4 _Ballot);

uvec4 subgroupPartitionNV(float16_t _Value);
uvec4 subgroupPartitionNV(f16vec2 _Value);
uvec4 subgroupPartitionNV(f16vec3 _Value);
uvec4 subgroupPartitionNV(f16vec4 _Value);
