// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

#include "Constants.glsl"

// --- Built-In Language Variables ---
// --- Tessellation Control Shader Special Variables ---

// --- Tessellation Control Input Variables ---
in gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_in[gl_MaxPatchVertices];

in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
in int gl_InvocationID;

// --- Tessellation Control Output Variables ---
out gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_out[];

patch out float gl_TessLevelOuter[4];
patch out float gl_TessLevelInner[2];
