#define GL_ARM_tensors_float_e4m3 1

// GL_ARM_tensors_float_e4m3 (defined by the GL_ARM_tensors document)
// GL_EXT_float_e4m3 must also be enabled to use these overloads.
#include "../EXT/GL_EXT_float_e4m3.glsl"

void tensorReadARM(tensorARM _Tensor, uint _Coords[], out floate4m3_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _Tensor, uint _Coords[], floate4m3_t _Data[], uint _TensorOperands);

void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate4m3_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], floate4m3_t _Data[], uint _TensorOperands);
