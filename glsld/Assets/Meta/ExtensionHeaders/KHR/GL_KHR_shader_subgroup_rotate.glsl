#define GL_KHR_shader_subgroup_rotate 1

float subgroupRotate(float _Value, uint _Delta);
int subgroupRotate(int _Value, uint _Delta);
uint subgroupRotate(uint _Value, uint _Delta);
bool subgroupRotate(bool _Value, uint _Delta);
double subgroupRotate(double _Value, uint _Delta);

float subgroupClusteredRotate(float _Value, uint _Delta, uint _ClusterSize);
int subgroupClusteredRotate(int _Value, uint _Delta, uint _ClusterSize);
uint subgroupClusteredRotate(uint _Value, uint _Delta, uint _ClusterSize);
bool subgroupClusteredRotate(bool _Value, uint _Delta, uint _ClusterSize);
double subgroupClusteredRotate(double _Value, uint _Delta, uint _ClusterSize);

int8_t subgroupRotate(int8_t _Value, uint _Delta);
uint8_t subgroupRotate(uint8_t _Value, uint _Delta);
int8_t subgroupClusteredRotate(int8_t _Value, uint _Delta, uint _ClusterSize);
uint8_t subgroupClusteredRotate(uint8_t _Value, uint _Delta, uint _ClusterSize);

int16_t subgroupRotate(int16_t _Value, uint _Delta);
uint16_t subgroupRotate(uint16_t _Value, uint _Delta);
int16_t subgroupClusteredRotate(int16_t _Value, uint _Delta, uint _ClusterSize);
uint16_t subgroupClusteredRotate(uint16_t _Value, uint _Delta, uint _ClusterSize);

int64_t subgroupRotate(int64_t _Value, uint _Delta);
uint64_t subgroupRotate(uint64_t _Value, uint _Delta);
int64_t subgroupClusteredRotate(int64_t _Value, uint _Delta, uint _ClusterSize);
uint64_t subgroupClusteredRotate(uint64_t _Value, uint _Delta, uint _ClusterSize);

float16_t subgroupRotate(float16_t _Value, uint _Delta);
float16_t subgroupClusteredRotate(float16_t _Value, uint _Delta, uint _ClusterSize);
