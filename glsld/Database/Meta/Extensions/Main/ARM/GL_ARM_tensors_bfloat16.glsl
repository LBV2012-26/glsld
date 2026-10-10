#define GL_ARM_tensors_bfloat16 1

// GL_ARM_tensors_bfloat16 (defined by the GL_ARM_tensors document)
// GL_EXT_bfloat16 must also be enabled to use these overloads.
#include "../../Common/GL_ARM_tensors_defines.glsl"
// GL_EXT_bfloat16 is optional; do not enable it through an include.

#if defined(GL_EXT_bfloat16)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out bfloat16_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _Tensor, uint _Coords[], bfloat16_t _Data[], uint _TensorOperands);
#endif

#if defined(GL_EXT_bfloat16)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], bfloat16_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], bfloat16_t _Data[], uint _TensorOperands);
#endif
