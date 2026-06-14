// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

#include "Constants.glsl"

// --- Built-In Language Variables ---

// --- Compute Shader Special Variables ---
// workgroup dimensions
in    uvec3 gl_NumWorkGroups;
const uvec3 gl_WorkGroupSize;

// workgroup and invocation IDs
in uvec3 gl_WorkGroupID;
in uvec3 gl_LocalInvocationID;

// derived variables
// [Note] gl_GlobalInvocationID = gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID;
in uvec3 gl_GlobalInvocationID;

// [Note] gl_LocalInvocationIndex =
//        gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
//        gl_LocalInvocationID.y * gl_WorkGroupSize.x +
//        gl_LocalInvocationID.x;
in uint  gl_LocalInvocationIndex;
