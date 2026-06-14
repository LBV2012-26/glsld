// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

#include "Constants.glsl"

// --- Built-In Language Variables ---

// --- Fragment Shader Special Variables ---
in vec4  gl_FragCoord;
in bool  gl_FrontFacing;
in float gl_ClipDistance[];
in float gl_CullDistance[];
in vec2  gl_PointCoord;
in int   gl_PrimitiveID;
in int   gl_SampleID;
in vec2  gl_SamplePosition;
in int   gl_SampleMaskIn[];
in int   gl_Layer;
in int   gl_ViewportIndex;
in bool  gl_HelperInvocation;

out float gl_FragDepth;
out int   gl_SampleMask[];
