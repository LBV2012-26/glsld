// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

#include "Constants.glsl"

// --- Built-In Language Variables ---
// --- Geometry Shader Special Variables ---

// --- Geometry Shader Input Variables ---
in gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_in[];

in int gl_PrimitiveIDIn;
in int gl_InvocationID;

// --- Geometry Shader Output Variables ---
out gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};

out int gl_PrimitiveID;
out int gl_Layer;
out int gl_ViewportIndex;
