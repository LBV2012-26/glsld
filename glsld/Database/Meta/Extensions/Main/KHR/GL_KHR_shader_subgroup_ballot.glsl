#define GL_KHR_shader_subgroup_ballot 1

// Enabling this extension implicitly enables GL_KHR_shader_subgroup_basic.
#include "GL_KHR_shader_subgroup_basic.glsl"

highp in uvec4 gl_SubgroupEqMask;
highp in uvec4 gl_SubgroupGeMask;
highp in uvec4 gl_SubgroupGtMask;
highp in uvec4 gl_SubgroupLeMask;
highp in uvec4 gl_SubgroupLtMask;

uvec4 subgroupBallot(bool _Value);
bool subgroupInverseBallot(uvec4 _Value);
bool subgroupBallotBitExtract(uvec4 _Value, uint _Index);
uint subgroupBallotBitCount(uvec4 _Value);
uint subgroupBallotInclusiveBitCount(uvec4 _Value);
uint subgroupBallotExclusiveBitCount(uvec4 _Value);
uint subgroupBallotFindLSB(uvec4 _Value);
uint subgroupBallotFindMSB(uvec4 _Value);
