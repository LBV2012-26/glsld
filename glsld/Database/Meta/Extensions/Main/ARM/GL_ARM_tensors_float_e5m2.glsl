#define GL_ARM_tensors_float_e5m2 1

// GL_ARM_tensors_float_e5m2 (defined by the GL_ARM_tensors document)
// GL_EXT_float_e5m2 must also be enabled to use these overloads.
#include "../../Common/GL_ARM_tensors_defines.glsl"
// GL_EXT_float_e5m2 is optional; do not enable it through an include.

#if defined(GL_EXT_float_e5m2)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out floate5m2_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _Tensor, uint _Coords[], floate5m2_t _Data[], uint _TensorOperands);
#endif

#if defined(GL_EXT_float_e5m2)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate5m2_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate5m2_t _Data[], uint _TensorOperands);
#endif
