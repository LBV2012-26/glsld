#define GL_KHR_shader_subgroup_basic 1

highp in uint gl_NumSubgroups;
highp in uint gl_SubgroupID;
mediump in uint gl_SubgroupSize;
mediump in uint gl_SubgroupInvocationID;

void subgroupBarrier();
void subgroupMemoryBarrier();
void subgroupMemoryBarrierBuffer();
void subgroupMemoryBarrierShared();
void subgroupMemoryBarrierImage();

bool subgroupElect();

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
