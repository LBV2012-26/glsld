#define GL_EXT_cooperative_matrix_maintenance1 1

// [Note] The "_Func" primitive is used for LSP placeholders of user-defined
// combineOp / elemOp function arguments and should not be used directly in
// shader code.

// GLSL_EXT_cooperative_matrix_maintenance1
// Spec: "This extension depends on GL_KHR_cooperative_matrix."
#include "../KHR/GL_KHR_cooperative_matrix.glsl"

const int gl_CooperativeMatrixReduceRowEXT          = 0x1;
const int gl_CooperativeMatrixReduceColumnEXT       = 0x2;
const int gl_CooperativeMatrixReduceRowAndColumnEXT = 0x3;
const int gl_CooperativeMatrixReduce2x2EXT          = 0x4;

void coopMatReduceEXT(out coopmat _Result, coopmat _Matrix, int _ReduceMask, _Func _CombineOp);
void coopMatPerElementEXT(out coopmat _Result, coopmat _Matrix, _Func _ElemOp, ...);
void coopMatTransposeEXT(out coopmat _Result, coopmat _Matrix);
uvec2 coopMatGetCoordinateEXT(coopmat _Matrix, uint _Index);
