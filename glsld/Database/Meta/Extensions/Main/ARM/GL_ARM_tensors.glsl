#define GL_ARM_tensors 1

// Umbrella document "GL_ARM_tensors" defines:
//   GL_ARM_tensors, GL_ARM_tensors_bfloat16,
//   GL_ARM_tensors_float_e5m2, GL_ARM_tensors_float_e4m3.

#include "../../Common/GL_ARM_tensors_defines.glsl"
#if defined(GL_ARM_tensors_bfloat16) && defined(GL_EXT_bfloat16)
#include "GL_ARM_tensors_bfloat16.glsl"
#endif
#if defined(GL_ARM_tensors_float_e4m3) && defined(GL_EXT_float_e4m3)
#include "GL_ARM_tensors_float_e4m3.glsl"
#endif
#if defined(GL_ARM_tensors_float_e5m2) && defined(GL_EXT_float_e5m2)
#include "GL_ARM_tensors_float_e5m2.glsl"
#endif

const uint gl_TensorOperandsNonTemporalARM      = 0x1U;
const uint gl_TensorOperandsOutOfBoundsValueARM = 0x2U;

uint tensorSizeARM(tensorARM _Tensor, uint _Dim);

void tensorReadARM(tensorARM _Tensor, uint _Coords[], out bool _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out int8_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out int16_t _Data, uint _TensorOperands);
#endif
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out int _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out int64_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out uint8_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out uint16_t _Data, uint _TensorOperands);
#endif
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out uint _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out uint64_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out float16_t _Data, uint _TensorOperands);
#endif
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out float _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], out float64_t _Data, uint _TensorOperands);
#endif

void tensorReadARM(tensorARM _Tensor, uint _Coords[], bool _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], int8_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], int16_t _Data[], uint _TensorOperands);
#endif
void tensorReadARM(tensorARM _Tensor, uint _Coords[], int _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], int64_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], uint8_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], uint16_t _Data[], uint _TensorOperands);
#endif
void tensorReadARM(tensorARM _Tensor, uint _Coords[], uint _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], uint64_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], float16_t _Data[], uint _TensorOperands);
#endif
void tensorReadARM(tensorARM _Tensor, uint _Coords[], float _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorReadARM(tensorARM _Tensor, uint _Coords[], float64_t _Data[], uint _TensorOperands);
#endif

void tensorWriteARM(tensorARM _Tensor, uint _Coords[], bool _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int8_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int16_t _Data, uint _TensorOperands);
#endif
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int64_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint8_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint16_t _Data, uint _TensorOperands);
#endif
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint64_t _Data, uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], float16_t _Data, uint _TensorOperands);
#endif
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], float _Data, uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], float64_t _Data, uint _TensorOperands);
#endif

void tensorWriteARM(tensorARM _Tensor, uint _Coords[], bool _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int8_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int16_t _Data[], uint _TensorOperands);
#endif
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], int64_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int8) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint8_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_int16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint16_t _Data[], uint _TensorOperands);
#endif
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_int64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], uint64_t _Data[], uint _TensorOperands);
#endif
#if defined(GL_EXT_shader_explicit_arithmetic_types_float16) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], float16_t _Data[], uint _TensorOperands);
#endif
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], float _Data[], uint _TensorOperands);
#if defined(GL_EXT_shader_explicit_arithmetic_types_float64) || defined(GL_EXT_shader_explicit_arithmetic_types)
void tensorWriteARM(tensorARM _Tensor, uint _Coords[], float64_t _Data[], uint _TensorOperands);
#endif
