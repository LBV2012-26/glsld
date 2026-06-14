// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

#include "Constants.glsl"

// --- Built-In Language Variables ---
// --- Tessellation Evaluation Shader Special Variables ---

// --- Tessellation Evaluation Input Variables ---
// [Note] gl_TessCoord.x == 1.0 - (1.0 - gl_TessCoord.x) // two operations performed
// [Note] gl_TessCoord.y == 1.0 - (1.0 - gl_TessCoord.y) // two operations performed
// [Note] gl_TessCoord.z == 1.0 - (1.0 - gl_TessCoord.z) // two operations performed

in gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_in[gl_MaxPatchVertices];

in int         gl_PatchVerticesIn;
in int         gl_PrimitiveID;
in vec3        gl_TessCoord;
patch in float gl_TessLevelOuter[4];
patch in float gl_TessLevelInner[2];

// --- Tessellation Evaluation Output Variables ---
out gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};
