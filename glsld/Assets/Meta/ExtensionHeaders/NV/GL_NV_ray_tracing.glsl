in uvec3 gl_LaunchIDNV;
in uvec3 gl_LaunchSizeNV;

in int gl_PrimitiveID;
in int gl_InstanceID;
in int gl_InstanceCustomIndexNV;

in vec3 gl_WorldRayOriginNV;
in vec3 gl_WorldRayDirectionNV;
in vec3 gl_ObjectRayOriginNV;
in vec3 gl_ObjectRayDirectionNV;

in float gl_RayTminNV;
in float gl_RayTmaxNV;
in uint  gl_IncomingRayFlagsNV;

in float gl_HitTNV;
in uint  gl_HitKindNV;

in mat4x3 gl_ObjectToWorldNV;
in mat4x3 gl_WorldToObjectNV;

const uint gl_RayFlagsNoneNV                     = 0U;
const uint gl_RayFlagsOpaqueNV                   = 1U;
const uint gl_RayFlagsNoOpaqueNV                 = 2U;
const uint gl_RayFlagsTerminateOnFirstHitNV      = 4U;
const uint gl_RayFlagsSkipClosestHitShaderNV     = 8U;
const uint gl_RayFlagsCullBackFacingTrianglesNV  = 16U;
const uint gl_RayFlagsCullFrontFacingTrianglesNV = 32U;
const uint gl_RayFlagsCullOpaqueNV               = 64U;
const uint gl_RayFlagsCullNoOpaqueNV             = 128U;

void traceNV(accelerationStructureNV _TopLevel, uint _RayFlags, uint _CullMask, uint _SbtRecordOffset, uint _SbtRecordStride, uint _MissIndex, vec3 _Origin, float _Tmin, vec3 _Direction, float _Tmax, int _Payload);
bool reportIntersectionNV(float _HitT, uint _HitKind);
void ignoreIntersectionNV();
void terminateRayNV();
void executeCallableNV(uint _SbtRecordIndex, int _Callable);
