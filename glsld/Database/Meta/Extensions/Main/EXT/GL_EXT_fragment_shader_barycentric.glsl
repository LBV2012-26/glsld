#define __GLSLD_EXTENSION_ENABLED_GL_EXT_fragment_shader_barycentric 1

#define pervertexEXT spirv_decorate(extensions = ["SPV_KHR_fragment_shader_barycentric"], capabilities = [5284], 5285)

in vec3 gl_BaryCoordEXT;
in vec3 gl_BaryCoordNoPerspEXT;
