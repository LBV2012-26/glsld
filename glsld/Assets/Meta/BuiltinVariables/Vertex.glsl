// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

#include "Constants.glsl"

// --- Built-In Language Variables ---

// --- Vertex Shader Special Variables ---
in int gl_VertexID;       // only present when not targeting Vulkan;
in int gl_InstanceID;     // only present when not targeting Vulkan;
in int gl_VertexIndex;    // only present when targeting Vulkan;
in int gl_InstanceIndex;  // only present when targeting Vulkan;
in int gl_DrawID;
in int gl_BaseVertex;
in int gl_BaseInstance;

out gl_PerVertex {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
};
