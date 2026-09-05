#define GL_EXT_shader_invocation_reorder 1

#include "GL_EXT_buffer_reference_uvec2.glsl"
#include "GL_EXT_ray_query.glsl"
#include "GL_EXT_ray_tracing.glsl"

void hitObjectTraceRayEXT(hitObjectEXT _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, int _Payload);
void hitObjectTraceRayMotionEXT(hitObjectEXT _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, float _CurrentTime, int _Payload);
void hitObjectRecordFromQueryEXT(hitObjectEXT _Hitobject, rayQueryEXT _RayQuery, uint _SbtRecordIndex, int _AttributeLocation);
void hitObjectRecordFromQueryEXT(hitObjectEXT _Hitobject, rayQueryEXT _RayQuery, uint _SbtRecordIndex, int _AttributeLocation, uint _HitKind);
void hitObjectRecordMissEXT(hitObjectEXT _HitObject, uint _RayFlags, uint _SbtRecordIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _TMax);
void hitObjectRecordMissMotionEXT(hitObjectEXT _HitObject, uint _RayFlags, uint _SbtRecordIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _TMax, float _CurrentTime);
void hitObjectRecordEmptyEXT(hitObjectEXT _HitObject);
void hitObjectExecuteShaderEXT(hitObjectEXT _HitObject, int _Payload);
bool hitObjectIsEmptyEXT(hitObjectEXT _HitObject);
bool hitObjectIsMissEXT(hitObjectEXT _HitObject);
bool hitObjectIsHitEXT(hitObjectEXT _HitObject);
float hitObjectGetRayTMinEXT(hitObjectEXT _HitObject);
float hitObjectGetRayTMaxEXT(hitObjectEXT _HitObject);
uint hitObjectGetRayFlagsEXT(hitObjectEXT _HitObject);
vec3 hitObjectGetObjectRayOriginEXT(hitObjectEXT _HitObject);
vec3 hitObjectGetObjectRayDirectionEXT(hitObjectEXT _HitObject);
vec3 hitObjectGetWorldRayOriginEXT(hitObjectEXT _HitObject);
vec3 hitObjectGetWorldRayDirectionEXT(hitObjectEXT _HitObject);
mat4x3 hitObjectGetObjectToWorldEXT(hitObjectEXT _HitObject);
mat4x3 hitObjectGetWorldToObjectEXT(hitObjectEXT _HitObject);
void hitObjectGetIntersectionTriangleVertexPositionsEXT(hitObjectEXT _HitObject, out vec3 _Positions[3]);
int hitObjectGetInstanceCustomIndexEXT(hitObjectEXT _HitObject);
int hitObjectGetInstanceIdEXT(hitObjectEXT _HitObject);
int hitObjectGetGeometryIndexEXT(hitObjectEXT _HitObject);
int hitObjectGetPrimitiveIndexEXT(hitObjectEXT _HitObject);
uint hitObjectGetHitKindEXT(hitObjectEXT _HitObject);
void hitObjectGetAttributesEXT(hitObjectEXT _HitObject, int _AttributeLocation);
uvec2 hitObjectGetShaderRecordBufferHandleEXT(hitObjectEXT _HitObject);
uint hitObjectGetShaderBindingTableRecordIndexEXT(hitObjectEXT _HitObject);
void hitObjectSetShaderBindingTableRecordIndexEXT(hitObjectEXT _HitObject, uint _Index);
float hitObjectGetCurrentTimeEXT(hitObjectEXT _HitObject);

void reorderThreadEXT(uint _Hint, uint _Bits);
void reorderThreadEXT(hitObjectEXT _HitObject);
void reorderThreadEXT(hitObjectEXT _HitObject, uint _Hint, uint _Bits);

void hitObjectReorderExecuteEXT(hitObjectEXT _HitObject, int _Payload);
void hitObjectReorderExecuteEXT(hitObjectEXT _HitObject, uint _Hint, uint _Bits, int _Payload);
void hitObjectTraceReorderExecuteEXT(hitObjectEXT _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, int _Payload);
void hitObjectTraceReorderExecuteEXT(hitObjectEXT _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, uint _Hint, uint _Bits, int _Payload);
void hitObjectTraceMotionReorderExecuteEXT(hitObjectEXT _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, float _CurrentTime, uint _Hint, uint _Bits, int _Payload);
