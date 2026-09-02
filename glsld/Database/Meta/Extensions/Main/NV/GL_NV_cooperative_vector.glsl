#define GL_NV_cooperative_vector 1

const int gl_ComponentTypeFloat16NV            = 0;
const int gl_ComponentTypeFloat32NV            = 1;
const int gl_ComponentTypeFloat64NV            = 2;
const int gl_ComponentTypeSignedInt8NV         = 3;
const int gl_ComponentTypeSignedInt16NV        = 4;
const int gl_ComponentTypeSignedInt32NV        = 5;
const int gl_ComponentTypeSignedInt64NV        = 6;
const int gl_ComponentTypeUnsignedInt8NV       = 7;
const int gl_ComponentTypeUnsignedInt16NV      = 8;
const int gl_ComponentTypeUnsignedInt32NV      = 9;
const int gl_ComponentTypeUnsignedInt64NV      = 10;
const int gl_ComponentTypeSignedInt8PackedNV   = 1000491000;
const int gl_ComponentTypeUnsignedInt8PackedNV = 1000491001;
const int gl_ComponentTypeFloatE4M3NV          = 1000491002;
const int gl_ComponentTypeFloatE5M2NV          = 1000491003;

const int gl_CooperativeVectorMatrixLayoutRowMajorNV           = 0;
const int gl_CooperativeVectorMatrixLayoutColumnMajorNV        = 1;
const int gl_CooperativeVectorMatrixLayoutInferencingOptimalNV = 2;
const int gl_CooperativeVectorMatrixLayoutTrainingOptimalNV    = 3;

void coopVecMatMulAddNV(out coopvecNV<ResultTy, ResultComps> _Result, coopvecNV<InputTy, InputComps> _Input, int _InputInterpretation, const MatrixTy[] _Matrix, uint _MatrixOffset, int _MatrixInterpretation, const BiasTy[] _Bias, uint _BiasOffset, int _BiasInterpretation, uint _M, uint _K, int _MatrixLayout, bool _Transpose, uint _MatrixStride);
void coopVecMatMulNV(out coopvecNV<ResultTy, ResultComps> _Result, coopvecNV<InputTy, InputComps> _Input, int _InputInterpretation, const MatrixTy[] _Matrix, uint _MatrixOffset, int _MatrixInterpretation, uint _M, uint _K, int _MatrixLayout, bool _Transpose, uint _MatrixStride);
void coopVecLoadNV(out coopvecNV<VectorElemTy, NumComps> _V, volatile coherent ArrayElemTy[] _Buffer, uint _Offset);
void coopVecStoreNV(coopvecNV<VectorElemTy, NumComps> _V, volatile coherent ArrayElemTy[] _Buffer, uint _Offset);
void coopVecOuterProductAccumulateNV(const coopvecNV<T, M> _V1, const coopvecNV<T, N> _V2, T[] _Buffer, uint _Offset, uint _Stride, int _MatrixLayout, int _MatrixInterpretation);
void coopVecReduceSumAccumulateNV(const coopvecNV<VectorElemTy, NumComps> _V, T[] _Buffer, uint _Offset);
