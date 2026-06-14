#define GL_ARM_tensors 1

const uint gl_TensorOperandsNonTemporalARM      = 0x1U;
const uint gl_TensorOperandsOutOfBoundsValueARM = 0x2U;

uint tensorSizeARM(tensorARM _T, uint _Dim);

void tensorReadARM(tensorARM _T, uint _Coords[], out bool _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out int8_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out int16_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out int32_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out int64_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out uint8_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out uint16_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out uint32_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out uint64_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out float16_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out float32_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out float64_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out bfloat16_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out floate5m2_t _Data, uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], out floate4m3_t _Data, uint _TensorOperands);

void tensorReadARM(tensorARM _T, uint _Coords[], bool _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], int8_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], int16_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], int32_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], int64_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], uint8_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], uint16_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], uint32_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], uint64_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], float16_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], float32_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], float64_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], bfloat16_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], floate5m2_t _Data[], uint _TensorOperands);
void tensorReadARM(tensorARM _T, uint _Coords[], floate4m3_t _Data[], uint _TensorOperands);

void tensorWriteARM(tensorARM _T, uint _Coords[], bool _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int8_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int16_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int32_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int64_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint8_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint16_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint32_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint64_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], float16_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], float32_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], float64_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], bfloat16_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], floate5m2_t _Data, uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], floate4m3_t _Data, uint _TensorOperands);

void tensorWriteARM(tensorARM _T, uint _Coords[], bool _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int8_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int16_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int32_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], int64_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint8_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint16_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint32_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], uint64_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], float16_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], float32_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], float64_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], bfloat16_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], floate5m2_t _Data[], uint _TensorOperands);
void tensorWriteARM(tensorARM _T, uint _Coords[], floate4m3_t _Data[], uint _TensorOperands);
