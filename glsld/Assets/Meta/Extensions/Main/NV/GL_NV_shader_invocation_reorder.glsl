#define GL_NV_shader_invocation_reorder 1

#define GL_EXT_ray_tracing
#include "../../Common/RayTracingCommon.glsl"

void hitObjectTraceRayNV(hitObjectNV _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, int _Payload);
void hitObjectTraceRayMotionNV(hitObjectNV _Hitobject, accelerationStructureEXT _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, float _CurrentTime, int _Payload);
void hitObjectRecordHitNV(hitObjectNV _Hitobject, accelerationStructureEXT _TopLevel, int _Instanceid, int _Primitiveid, int _Geometryindex, uint _HitKind, uint _SbtRecordOffset, uint _SbtRecordStride, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, int _AttributeLocation);
void hitObjectRecordHitMotionNV(hitObjectNV _Hitobject, accelerationStructureEXT _TopLevel, int _Instanceid, int _Primitiveid, int _Geometryindex, uint _HitKind, uint _SbtRecordOffset, uint _SbtRecordStride, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, float _CurrentTime, int _AttributeLocation);
void hitObjectRecordHitWithIndexNV(hitObjectNV _Hitobject, accelerationStructureEXT _TopLevel, int _Instanceid, int _Primitiveid, int _Geometryindex, uint _HitKind, uint _SbtRecordIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, int _AttributeLocation);
void hitObjectRecordHitWithIndexMotionNV(hitObjectNV _Hitobject, accelerationStructureEXT _TopLevel, int _Instanceid, int _Primitiveid, int _Geometryindex, uint _HitKind, uint _SbtRecordIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, float _CurrentTime, int _AttributeLocation);
void hitObjectRecordMissNV(hitObjectNV _HitObject, uint _SbtRecordIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _TMax);
void hitObjectRecordMissMotionNV(hitObjectNV _HitObject, uint _SbtRecordIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _TMax, float _CurrentTime);
void hitObjectRecordEmptyNV(hitObjectNV _HitObject);
void hitObjectExecuteShaderNV(hitObjectNV _HitObject, int _Payload);
bool hitObjectIsEmptyNV(hitObjectNV _HitObject);
bool hitObjectIsMissNV(hitObjectNV _HitObject);
bool hitObjectIsHitNV(hitObjectNV _HitObject);
float hitObjectGetRayTMinNV(hitObjectNV _HitObject);
float hitObjectGetRayTMaxNV(hitObjectNV _HitObject);
vec3 hitObjectGetObjectRayOriginNV(hitObjectNV _HitObject);
vec3 hitObjectGetObjectRayDirectionNV(hitObjectNV _HitObject);
vec3 hitObjectGetWorldRayOriginNV(hitObjectNV _HitObject);
vec3 hitObjectGetWorldRayDirectionNV(hitObjectNV _HitObject);
mat4x3 hitObjectGetObjectToWorldNV(hitObjectNV _HitObject);
mat4x3 hitObjectGetWorldToObjectNV(hitObjectNV _HitObject);
int hitObjectGetInstanceCustomIndexNV(hitObjectNV _HitObject);
int hitObjectGetInstanceIdNV(hitObjectNV _HitObject);
int hitObjectGetGeometryIndexNV(hitObjectNV _HitObject);
int hitObjectGetPrimitiveIndexNV(hitObjectNV _HitObject);
uint hitObjectGetHitKindNV(hitObjectNV _HitObject);
void hitObjectGetAttributesNV(hitObjectNV _HitObject, int _AttributeLocation);
uvec2 hitObjectGetShaderRecordBufferHandleNV(hitObjectNV _HitObject);
uint hitObjectGetShaderBindingTableRecordIndexNV(hitObjectNV _HitObject);
float hitObjectGetCurrentTimeNV(hitObjectNV _HitObject);

void reorderThreadNV(uint _Hint, uint _Bits);
void reorderThreadNV(hitObjectNV _HitObject);
void reorderThreadNV(hitObjectNV _HitObject, uint _Hint, uint _Bits);
