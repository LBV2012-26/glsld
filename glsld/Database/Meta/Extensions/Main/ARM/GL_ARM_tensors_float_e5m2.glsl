#define GL_ARM_tensors_float_e5m2 1

// GL_ARM_tensors_float_e5m2 (defined by the GL_ARM_tensors document)
// GL_EXT_float_e5m2 must also be enabled to use these overloads.
#include "../EXT/GL_EXT_float_e5m2.glsl"

void tensorReadARM(tensorARM _Tensor, uint _Coords[], out floate5m2_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _Tensor, uint _Coords[], floate5m2_t _Data[], uint _TensorOperands);

void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate5m2_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate5m2_t _Data[], uint _TensorOperands);
