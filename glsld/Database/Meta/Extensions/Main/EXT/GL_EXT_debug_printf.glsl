#define GL_EXT_debug_printf 1

#include "../KHR/GL_KHR_vulkan_glsl.glsl"

// LSP placeholder for the literal-string argument of debugPrintfEXT.
class string;

void debugPrintfEXT(const in string _Format, ...);
