highp in uint gl_NumSubgroups;
highp in uint gl_SubgroupID;
mediump in uint gl_SubgroupSize;
mediump in uint gl_SubgroupInvocationID;
highp in uvec4 gl_SubgroupEqMask;
highp in uvec4 gl_SubgroupGeMask;
highp in uvec4 gl_SubgroupGtMask;
highp in uvec4 gl_SubgroupLeMask;
highp in uvec4 gl_SubgroupLtMask;

void subgroupBarrier();
void subgroupMemoryBarrier();
void subgroupMemoryBarrierBuffer();
void subgroupMemoryBarrierShared();
void subgroupMemoryBarrierImage();

bool subgroupElect();

bool subgroupAll(bool _Value);
bool subgroupAny(bool _Value);
bool subgroupAllEqual(float _Value);
bool subgroupAllEqual(int _Value);
bool subgroupAllEqual(uint _Value);
bool subgroupAllEqual(bool _Value);
bool subgroupAllEqual(double _Value);

float subgroupBroadcast(float _Value, uint _Id);
int subgroupBroadcast(int _Value, uint _Id);
uint subgroupBroadcast(uint _Value, uint _Id);
bool subgroupBroadcast(bool _Value, uint _Id);
double subgroupBroadcast(double _Value, uint _Id);

float subgroupBroadcastFirst(float _Value);
int subgroupBroadcastFirst(int _Value);
uint subgroupBroadcastFirst(uint _Value);
bool subgroupBroadcastFirst(bool _Value);
double subgroupBroadcastFirst(double _Value);

uvec4 subgroupBallot(bool _Value);
bool subgroupInverseBallot(uvec4 _Value);
bool subgroupBallotBitExtract(uvec4 _Value, uint _Index);
uint subgroupBallotBitCount(uvec4 _Value);
uint subgroupBallotInclusiveBitCount(uvec4 _Value);
uint subgroupBallotExclusiveBitCount(uvec4 _Value);
uint subgroupBallotFindLSB(uvec4 _Value);
uint subgroupBallotFindMSB(uvec4 _Value);

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
