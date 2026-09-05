#define GL_KHR_shader_subgroup_vote 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

bool subgroupAll(bool _Value);
bool subgroupAny(bool _Value);
bool subgroupAllEqual(float _Value);
bool subgroupAllEqual(int _Value);
bool subgroupAllEqual(uint _Value);
bool subgroupAllEqual(bool _Value);
bool subgroupAllEqual(double _Value);
