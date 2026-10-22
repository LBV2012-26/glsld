#define __GLSLD_EXTENSION_ENABLED_GL_ARM_tensors_float_e4m3 1

// GL_ARM_tensors_float_e4m3 (defined by the GL_ARM_tensors document)
// GL_EXT_float_e4m3 must also be enabled to use these overloads.
#include "../../Common/GL_ARM_tensors_defines.glsl"
// GL_EXT_float_e4m3 is optional; do not enable it through an include.

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_float_e4m3)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out floate4m3_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _Tensor, uint _Coords[], floate4m3_t _Data[], uint _TensorOperands);
#endif

#if defined(__GLSLD_EXTENSION_ENABLED_GL_EXT_float_e4m3)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate4m3_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate4m3_t _Data[], uint _TensorOperands);
#endif
