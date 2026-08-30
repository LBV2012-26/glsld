#version 460
#pragma shader_stage(compute)
#extension GL_EXT_shader_explicit_arithmetic_types : enable

// ============================================================================
// 1. 角度与三角函数 (Section 8.1)
// ============================================================================
// [合法求值]
const float kRadiansVal     = radians(180.0);                   // 期望: ~3.14159265
const vec2  kDegreesVal     = degrees(vec2(kRadiansVal, 0.0));  // 期望: vec2(180.0, 0.0)
const float kSinVal         = sin(0.0);                         // 期望: 0.0
const vec3  kCosVal         = cos(vec3(0.0));                   // 期望: vec3(1.0, 1.0, 1.0)
const float kTanVal         = tan(0.0);                         // 期望: 0.0
const float kAsinVal        = asin(1.0);                        // 期望: ~1.5707963 (PI/2)
const float kAcosVal        = acos(1.0);                        // 期望: 0.0
const float kAtan1Val       = atan(1.0);                        // 期望: ~0.78539816 (PI/4)
const vec2  kAtan2Val       = atan(vec2(1.0, 0.0), vec2(1.0, 1.0)); // 期望: vec2(PI/4, 0.0)
const float kSinhVal        = sinh(0.0);                        // 期望: 0.0
const float kCoshVal        = cosh(0.0);                        // 期望: 1.0
const float kTanhVal        = tanh(0.0);                        // 期望: 0.0
const float kAsinhVal       = asinh(0.0);                       // 期望: 0.0
const float kAcoshVal       = acosh(1.0);                       // 期望: 0.0
const float kAtanhVal       = atanh(0.0);                       // 期望: 0.0

// [不合法 / 越界 (应安全产生 NaN/Inf，不崩溃)]
const float kAsinInvalid    = asin(2.0);                        // 期望: NaN
const float kAcosInvalid    = acos(-2.0);                       // 期望: NaN
const float kAcoshInvalid   = acosh(0.5);                       // 期望: NaN
const float kAtanhInf       = atanh(1.0);                       // 期望: Inf
const float kAtanhInvalid   = atanh(2.0);                       // 期望: NaN

// ============================================================================
// 2. 指数与对数函数 (Section 8.2)
// ============================================================================
// [合法求值]
const float kPowVal         = pow(2.0, 8.0);                    // 期望: 256.0
const vec2  kExpVal         = exp(vec2(0.0, 1.0));              // 期望: vec2(1.0, 2.7182818)
const float kExp2Val        = exp2(4.0);                        // 期望: 16.0
const float kLogVal         = log(2.718281828);                 // 期望: ~1.0
const vec3  kLog2Val        = log2(vec3(2.0, 4.0, 8.0));        // 期望: vec3(1.0, 2.0, 3.0)
const float kSqrtVal        = sqrt(100.0);                      // 期望: 10.0
const float kInvSqrtVal     = inversesqrt(16.0);                // 期望: 0.25

// [不合法 / 越界]
const float kSqrtNeg        = sqrt(-4.0);                       // 期望: NaN
const float kLogZero        = log(0.0);                         // 期望: -Inf
const float kLogNeg         = log(-1.0);                        // 期望: NaN
const float kPowInvalid     = pow(-2.0, 0.5);                   // 期望: NaN
const float kInvSqrtZero    = inversesqrt(0.0);                 // 期望: Inf

// ============================================================================
// 3. 通用函数 (Section 8.3)
// ============================================================================
// [合法求值 - 标量与向量重载]
const int   kAbsInt         = abs(-50);                         // 期望: 50
const ivec2 kAbsIvec        = abs(ivec2(10u, 20u));             // 期望: uvec2(10u, 20u)
const float kSignFloat      = sign(-3.5);                       // 期望: -1.0
const ivec3 kSignIvec       = sign(ivec3(-10, 0, 10));          // 期望: ivec3(-1, 0, 1)
const vec2  kSignVec        = sign(vec2(0.0, 5.0));             // 期望: vec2(0.0, 1.0)
const vec2  kFloorVal       = floor(vec2(1.9, -1.1));           // 期望: vec2(1.0, -2.0)
const vec2  kTruncVal       = trunc(vec2(1.9, -1.1));           // 期望: vec2(1.0, -1.0)
const float kRoundEvenVal   = roundEven(2.5);                   // 期望: 2.0 (银行家舍入)
const float kCeilVal        = ceil(4.1);                        // 期望: 5.0
const float kFractVal       = fract(5.75);                      // 期望: 0.75
const float kModGlsl        = mod(-5.0, 3.0);                   // 期望: 1.0
const int   kMinInt         = min(-10, 20);                     // 期望: -10
const uvec2 kMaxUint        = max(uvec2(10u, 100u), uvec2(50u, 5u)); // 期望: uvec2(50u, 100u)
const float kClampVal       = clamp(15.0, 0.0, 10.0);           // 期望: 10.0
const vec2  kMixFloat       = mix(vec2(0.0), vec2(10.0), 0.5);  // 期望: vec2(5.0, 5.0)
const vec2  kMixVecFactor   = mix(vec2(0.0), vec2(10.0), vec2(0.2, 0.8)); // 期望: vec2(2.0, 8.0)
const ivec2 kMixBoolIvec    = mix(ivec2(1, 2), ivec2(10, 20), bvec2(true, false)); // 期望: ivec2(10, 2)
const float kStepVal        = step(5.0, 7.0);                   // 期望: 1.0
const float kSmoothstepVal  = smoothstep(0.0, 1.0, 0.5);        // 期望: 0.5
const float kFmaVal         = fma(2.0, 3.0, 4.0);               // 期望: 10.0
const float kLdexpVal       = ldexp(1.5, 3);                    // 期望: 12.0 (1.5 * 2^3)

// [浮点特征判断]
const bool  kIsNanTrue      = isnan(kSqrtNeg);                  // 期望: true
const bool  kIsNanFalse     = isnan(1.0);                       // 期望: false
const bool  kIsInfTrue      = isinf(kLogZero);                  // 期望: true

// [基础 32 位重解释]
const int   kFloatBits      = floatBitsToInt(1.0);              // 期望: 0x3F800000 (1065353216)
const float kIntBits        = intBitsToFloat(0x3F800000);       // 期望: 1.0

// [GL_EXT 显式位宽位重解释]
const int16_t   kHalfBits      = halfBitsToInt16(1.0hf);            // 期望: 0x3C00 (15360)
const float16_t kInt16ToHalf   = int16BitsToHalf(int16_t(0x3C00));  // 期望: 1.0
const int64_t   kDoubleBits    = doubleBitsToInt64(1.0);            // 期望: 0x3FF0000000000000 (4607182418800017408)
const double    kInt64ToDouble = int64BitsToDouble(kDoubleBits);    // 期望: 1.0

// ============================================================================
// 4. 打包与解包函数 (Section 8.4 及 EXT 扩展)
// ============================================================================
// [GLSL 4.60 核心打包/解包]
const uint   kPackUnorm2x16    = packUnorm2x16(vec2(0.0, 1.0));          // 期望: 0xFFFF0000u (4294901760)
const vec2   kUnpackUnorm2x16  = unpackUnorm2x16(kPackUnorm2x16);        // 期望: vec2(0.0, 1.0)
const uint   kPackHalf2x16     = packHalf2x16(vec2(1.0, 2.0));           // 期望: 0x40003C00u (1074266112)
const vec2   kUnpackHalf2x16   = unpackHalf2x16(kPackHalf2x16);          // 期望: vec2(1.0, 2.0)
const uint   kPackUnorm4x8     = packUnorm4x8(vec4(1.0, 0.0, 0.0, 1.0)); // 期望: 0xFF0000FFu (4278190335)
const vec4   kUnpackUnorm4x8   = unpackUnorm4x8(kPackUnorm4x8);          // 期望: vec4(1.0, 0.0, 0.0, 1.0)
const double kPackDouble2x32   = packDouble2x32(uvec2(0, 0x3FF00000u));  // 期望: 1.0
const uvec2  kUnpackDouble2x32 = unpackDouble2x32(kPackDouble2x32);      // 期望: uvec2(0, 1072693248u)

// [GL_EXT 扩展整数/半精度打包]
const uint32_t kPackFloat2x16   = packFloat2x16(f16vec2(1.0, 2.0)); // 期望: 0x40003C00u (1074266112)
const f16vec2  kUnpackFloat2x16 = unpackFloat2x16(kPackFloat2x16);  // 期望: f16vec2(1.0, 2.0)
const int32_t  kPackInt2x16     = packInt2x16(i16vec2(-1, 1));      // 期望: 0x0001FFFF (131071)
const i16vec2  kUnpackInt2x16   = unpackInt2x16(kPackInt2x16);      // 期望: i16vec2(-1, 1)
const int64_t  kPackInt2x32     = packInt2x32(ivec2(-1, 1));        // 期望: 0x00000001FFFFFFFF (8589934591)
const ivec2    kUnpackInt2x32   = unpackInt2x32(kPackInt2x32);      // 期望: ivec2(-1, 1)

// ============================================================================
// 5. 几何函数 (Section 8.5)
// ============================================================================
const float kLengthVal      = length(vec3(0.0, 3.0, 4.0));                                       // 期望: 5.0
const float kDistanceVal    = distance(vec2(1.0, 2.0), vec2(4.0, 6.0));                          // 期望: 5.0
const float kDotVal         = dot(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0));                     // 期望: 32.0
const vec3  kCrossVal       = cross(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));                   // 期望: vec3(0.0, 0.0, 1.0)
const vec2  kNormalizeVal   = normalize(vec2(0.0, 5.0));                                         // 期望: vec2(0.0, 1.0)
const vec3  kFaceForwardVal = faceforward(vec3(1.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, -1.0)); // 期望: vec3(1.0)
const vec2  kReflectVal     = reflect(vec2(1.0, -1.0), vec2(0.0, 1.0));                          // 期望: vec2(1.0, 1.0)
const vec2  kRefractVal     = refract(vec2(0.0, 1.0), vec2(0.0, -1.0), 1.0);                     // 期望: vec2(0.0, 1.0)

// ============================================================================
// 6. 矩阵操作函数 (Section 8.6)
// ============================================================================
const mat2   kMatA           = mat2(1.0, 2.0, 3.0, 4.0);
const mat2   kMatB           = mat2(2.0, 0.0, 1.0, 2.0);
const mat2   kMatCompMult    = matrixCompMult(kMatA, kMatB);     // 期望: mat2(2.0, 0.0, 3.0, 8.0)
const mat2   kTransposeMat   = transpose(kMatA);                 // 期望: mat2(1.0, 3.0, 2.0, 4.0)
const float  kDeterminantVal = determinant(kMatA);               // 期望: -2.0 (1*4 - 2*3)
const mat2   kInverseMat     = inverse(kMatA);                   // 期望: mat2(-2.0, 1.0, 1.5, -0.5)
const mat2x3 kOuterProd      = outerProduct(vec3(1.0, 2.0, 3.0), vec2(4.0, 5.0)); // 期望: 2列3行矩阵

// [矩阵与 Swizzle 组合]
const vec4 kMatrixSwizzleSource = vec4(1.0, 2.0, 3.0, 4.0);
const mat2 kMatrixFromSwizzles  = mat2(
    kMatrixSwizzleSource.yx,
    kMatrixSwizzleSource.wz
); // 期望: mat2(2.0, 1.0, 4.0, 3.0)

const vec2 kMatrixTimesSwizzle = (kMatrixFromSwizzles * kMatrixSwizzleSource.zx).yx; // 乘法结果 (10,6) 重排后期望: vec2(6.0, 10.0)
const vec2 kSwizzleTimesMatrix = (kMatrixSwizzleSource.xz * kMatrixFromSwizzles).yx; // 乘法结果 (5,13) 重排后期望: vec2(13.0, 5.0)

const mat2x3 kMatrixOuterFromSwizzles = outerProduct(kMatrixSwizzleSource.wzy, kMatrixSwizzleSource.yx); // 期望: mat2x3(8,6,4, 4,3,2)

// 矩阵不能直接 Swizzle；先索引取出列向量后再 Swizzle
const vec2 kMatrixColumnSwizzle    = kMatrixOuterFromSwizzles[1].zy;            // 第1列 (4,3,2) => vec2(2.0, 3.0)
const vec2 kTransposeColumnSwizzle = transpose(kMatrixOuterFromSwizzles)[2].yx; // 第2列 (4,2)   => vec2(2.0, 4.0)

// ============================================================================
// 7. 向量关系判断函数 (Section 8.7)
// ============================================================================
const bvec3 kLessThanVal = lessThan(vec3(1.0, 5.0, 3.0), vec3(2.0, 4.0, 3.0)); // 期望: bvec3(true, false, false)
const bvec2 kEqualVal    = equal(uvec2(5u, 10u), uvec2(5u, 20u));              // 期望: bvec2(true, false)
const bool  kAnyVal      = any(kLessThanVal);                                  // 期望: true
const bool  kAllVal      = all(kLessThanVal);                                  // 期望: false
const bvec2 kNotVal      = not(bvec2(true, false));                            // 期望: bvec2(false, true)

// ============================================================================
// 8. 整数位级操作函数 (Section 8.8 - 重点验证 32 位截断)
// ============================================================================
const int   kBitCountPos   = bitCount(0x00FF00FF);                // 期望: 16
const int   kBitCountNeg   = bitCount(-1);                        // 期望: 32 (32位全1)
const int   kFindLSBVal    = findLSB(0x00000080);                 // 期望: 7
const int   kFindLSBZero   = findLSB(0);                          // 期望: -1
const int   kFindMSBPos    = findMSB(0x00000080);                 // 期望: 7
const int   kFindMSBZero   = findMSB(0);                          // 期望: -1
const int   kFindMSBNeg    = findMSB(-1);                         // 期望: -1
const uint  kBitReverseVal = bitfieldReverse(0x80000000u);        // 期望: 1u (0x00000001u)
const uint  kExtractVal    = bitfieldExtract(0x1234u, 4, 4);      // 期望: 3u (提取0x3)
const uint  kInsertVal     = bitfieldInsert(0x0000u, 0xAu, 4, 4); // 期望: 0x00A0u (160u)

// ============================================================================
// 9. 向量 Swizzle
// ============================================================================
const vec4  kSwizzleSource = vec4(1.0, 2.0, 3.0, 4.0);

// [单分量、重排与重复分量]
const float kSwizzleScalar  = kSwizzleSource.z;                 // 期望: 3.0
const vec2  kSwizzleReorder = kSwizzleSource.yx;                // 期望: vec2(2.0, 1.0)
const vec4  kSwizzleRepeat  = kSwizzleSource.wwxx;              // 期望: vec4(4.0, 4.0, 1.0, 1.0)

// [xyzw / rgba / stpq 三套命名]
const vec3  kSwizzlePosition = kSwizzleSource.zyx;              // 期望: vec3(3.0, 2.0, 1.0)
const vec4  kSwizzleColor    = kSwizzleSource.bgra;             // 期望: vec4(3.0, 2.0, 1.0, 4.0)
const vec4  kSwizzleTexture  = kSwizzleSource.qpts;             // 期望: vec4(4.0, 3.0, 2.0, 1.0)

// [整数、无符号整数与布尔向量]
const ivec3 kSwizzleInt    = ivec4(-1, 2, -3, 4).wzx;           // 期望: ivec3(4, -3, -1)
const uvec2 kSwizzleUint   = uvec3(10u, 20u, 30u).zx;           // 期望: uvec2(30u, 10u)
const bvec4 kSwizzleBool   = bvec2(true, false).xyyx;           // 期望: bvec4(true, false, false, true)

// [链式 Swizzle 与表达式结果 Swizzle]
const vec2  kSwizzleChain  = kSwizzleSource.wzy.yx;             // wzy = (4,3,2), yx = (3,4)
const vec2  kSwizzleExpr   = max(kSwizzleSource, vec4(2.0)).wz; // 期望: vec2(4.0, 3.0)

// [复杂混合复合调用]
const vec4 kSwizzleComplexConstruct = vec4(
    kSwizzleSource.wy,
    max(kSwizzleSource.zx, vec2(2.5))
).zwyx;                                                         // 期望: vec4(3.0, 2.5, 2.0, 4.0)

const vec4 kSwizzleComplexBuiltins = mix(
    abs(vec4(-1.0, -2.0, 3.0, -4.0)).wzyx,
    max(kSwizzleSource, vec4(2.0)).xyzw,
    bvec4(true, false, true, false)
).bgra;                                                         // 期望: vec4(3.0, 3.0, 2.0, 1.0)

const vec4 kSwizzleComplexArithmetic = (
    pow(kSwizzleSource.wzyx, vec4(2.0)) + vec4(1.0)
).zwxy;                                                         // 期望: vec4(5.0, 2.0, 17.0, 10.0)

const ivec4 kSwizzleComplexBits = bitCount(
    ivec4(-1, 0x000000F0, 0x0000FF00, 1).wzyx
).yxwz;                                                         // 期望: ivec4(8, 1, 32, 4)

const bvec4 kSwizzleComplexRelation = not(
    lessThan(kSwizzleSource.wzyx, vec4(3.0)).yxwz
).zxyw;                                                         // 期望: bvec4(false, true, true, false)

const float kSwizzleComplexScalar = sqrt(pow(
    max(kSwizzleSource.wzy.x, kSwizzleSource.yx.y),
    2.0
));                                                             // 期望: 4.0

// [Swizzle 直接作为函数参数]
const float kSwizzleArgUnary = length(kSwizzleSource.wzy);      // 期望: sqrt(29.0)
const float kSwizzleArgBinary = dot(
    kSwizzleSource.wzy,
    kSwizzleSource.xyz
);                                                              // 期望: 16.0

const vec2 kSwizzleArgTernary = clamp(
    kSwizzleSource.wz,
    kSwizzleSource.xy,
    kSwizzleSource.zw
);                                                              // 期望: vec2(3.0, 3.0)

const vec4 kSwizzleArgAllSwizzled = mix(
    kSwizzleSource.wzyx,
    kSwizzleSource.xxyy,
    bvec4(true, false, false, true).wzyx
);                                                              // 期望: vec4(1.0, 3.0, 2.0, 2.0)

const float kSwizzleArgScalarized = pow(
    kSwizzleSource.w,
    kSwizzleSource.x
);                                                              // 期望: 4.0

const vec3 kSwizzleArgNested = max(
    abs(vec4(-1.0, -2.0, -3.0, -4.0).wzy),
    min(kSwizzleSource.zyx, vec3(2.5))
);                                                              // 期望: vec3(4.0, 3.0, 2.0)

// ============================================================================
// 10. 数组维度综合验证 (验证 AST 折叠连通性)
// ============================================================================
float array_dim_test[
    int(kDeterminantVal + 4.0) +     // -2.0 + 4.0 = 2
    (int)kLengthVal +                // 5
    int(kInvSqrtVal * 4.0) +         // 1
    kBitCountPos                     // 16
];                                   // 总大小: 2 + 5 + 1 + 16 = 24

const int kFinalArraySize = array_dim_test.length(); // 期望: 24

// ============================================================================
// 11. const 数组构造、索引与复合求值
// ============================================================================
// [初始化列表与数组整体 Hover]
const int kScalarArray[4] = { 10, 20, 30, 40 };                // 期望: int[4](10, 20, 30, 40)
const int kScalarArrayElement = kScalarArray[2];               // 期望: 30
const int kScalarArrayComputedIndex = kScalarArray[bitCount(3) - 1]; // bitCount(3)=2，期望: 20

// [显式尺寸与推导尺寸数组构造器]
const int kExplicitArray[3] = int[3](7, 8, 9);                 // 期望: int[3](7, 8, 9)
const int kExplicitArrayElement = kExplicitArray[2];           // 期望: 9
const int kDeducedArray[] = int[](11, 22, 33, 44);             // 期望: int[4](11, 22, 33, 44)
const int kDeducedArrayLength = kDeducedArray.length();        // 期望: 4

// [元素隐式转换]
const float kConvertedArray[3] = { 1, 2, 3 };                  // 期望: float[3](1.0, 2.0, 3.0)
const float kConvertedArrayElement = kConvertedArray[1];       // 期望: 2.0

// [向量数组、索引后 Swizzle 与函数参数]
const vec4 kVectorArray[2] = {
    vec4(1.0, 2.0, 3.0, 4.0),
    vec4(5.0, 6.0, 7.0, 8.0)
};
const vec2 kVectorArraySwizzle = kVectorArray[1].wz;           // 期望: vec2(8.0, 7.0)
const float kVectorArrayDot = dot(
    kVectorArray[0].xyz,
    kVectorArray[1].zyx
);                                                              // 期望: 34.0
const vec3 kVectorArrayNestedCall = max(
    abs(kVectorArray[0].wzy - vec3(5.0)),
    kVectorArray[1].xyz.yzx
);                                                              // max((1,2,3),(6,7,5)) => vec3(6.0, 7.0, 5.0)

// [矩阵数组、数组索引 + 矩阵列索引 + Swizzle]
const mat2 kMatrixArray[2] = {
    mat2(1.0, 2.0, 3.0, 4.0),
    mat2(5.0, 6.0, 7.0, 8.0)
};
const vec2  kMatrixArrayColumnSwizzle = kMatrixArray[1][0].yx;        // 第0列 (5,6) => vec2(6.0, 5.0)
const float kMatrixArrayDeterminant   = determinant(kMatrixArray[1]); // 5*8 - 6*7 = -2.0

// [多维数组与各层 length()]
const int kNestedArray[2][3] = {
    { 1, 2, 3 },
    { 4, 5, 6 }
};
const int kNestedArrayElement     = kNestedArray[1][2];         // 期望: 6
const int kNestedArrayOuterLength = kNestedArray.length();      // 期望: 2
const int kNestedArrayInnerLength = kNestedArray[0].length();   // 期望: 3

// [多维向量数组的链式索引与 Swizzle]
const vec3 kNestedVectorArray[2][2] = {
    { vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0) },
    { vec3(7.0, 8.0, 9.0), vec3(10.0, 11.0, 12.0) }
};

const vec3 kNestedVectorArraySwizzle = kNestedVectorArray[1][0].zyx; // 期望: vec3(9.0, 8.0, 7.0)

// ============================================================================
// 12. 复杂数组构造器（参考 OverloadTest.glsl）
// ============================================================================
// [临时数组构造器立即索引]
const int kTemporaryScalarArrayIndex = int[](10, 20, 30, 40)[bitCount(3)]; // bitCount(3)=2，期望: 30

const vec3 kTemporaryVectorArrayIndex =
    vec3[](
        vec3(1.0, 2.0, 3.0),
        vec3(vec2(4.0, 5.0), 6.0),
        vec3(7.0)
    )[1].zxy;                                                  // (4,5,6).zxy => vec3(6.0, 4.0, 5.0)

// [向量构造器参数展开 + 数组元素]
const vec4 kConstructorSourceArray[3] = vec4[](
    vec4(vec2(1.0, 2.0), vec2(3.0, 4.0)),
    vec4(ivec2(5, 6), 7.0, 8.0),
    vec4(vec3(9.0, 10.0, 11.0), 12.0)
);

const vec4 kArrayElementReconstructed = vec4(
    kConstructorSourceArray[2].w,
    kConstructorSourceArray[0].yx,
    kConstructorSourceArray[1].z
);                                                              // 期望: vec4(12.0, 2.0, 1.0, 7.0)

const vec3 kArrayElementBuiltinChain = normalize(vec3(
    kConstructorSourceArray[0].xy,
    length(kConstructorSourceArray[1].zw)
));                                                              // 期望: normalize(vec3(1.0, 2.0, sqrt(113.0)))

// [显式尺寸二维数组构造器]
const int kConstructedInt2D[2][3] = int[2][3](
    int[3](1, 2, 3),
    int[](4, 5, 6)
);
const int kConstructedInt2DValue =
    kConstructedInt2D[kConstructedInt2D.length() - 1]
                     [kConstructedInt2D[0].length() - 2];       // [1][1]，期望: 5

// [所有维度均由构造参数推导]
const float kDeducedFloat3D[][][] = float[][][](
    float[][](
        float[](1.0, 2.0, 3.0),
        float[](4.0, 5.0, 6.0)
    ),
    float[][](
        float[](7.0, 8.0, 9.0),
        float[](10.0, 11.0, 12.0)
    )
);
const float kDeducedFloat3DValue        = kDeducedFloat3D[1][0][bitCount(7) - 1]; // bitCount(7)=3，[1][0][2]，期望: 9.0
const int   kDeducedFloat3DOuterLength  = kDeducedFloat3D.length();
const int   kDeducedFloat3DMiddleLength = kDeducedFloat3D[0].length();
const int   kDeducedFloat3DInnerLength  = kDeducedFloat3D[0][0].length(); // 均期望: 2, 2, 3

// [多维向量数组构造器 + 临时结果索引 + Swizzle]
const vec2 kConstructedVector3D[2][2][2] = vec2[2][2][2](
    vec2[2][2](
        vec2[2](vec2(1.0, 2.0), vec2(3.0, 4.0)),
        vec2[](vec2(5.0, 6.0), vec2(7.0, 8.0))
    ),
    vec2[][](
        vec2[](vec2(9.0, 10.0), vec2(11.0, 12.0)),
        vec2[2](vec2(13.0, 14.0), vec2(15.0, 16.0))
    )
);
const vec4 kConstructedVector3DMix = vec4(
    kConstructedVector3D[1][1][0].yx,
    vec2[][](vec2[](vec2(17.0), vec2(18.0)))[0][1].xy
);                                                              // 期望: vec4(14.0, 13.0, 18.0, 18.0)

// [矩阵构造器嵌入数组构造器]
const mat2x3 kConstructedMatrixArray[2] = mat2x3[](
    mat2x3(
        vec3(1.0, 2.0, 3.0),
        vec3(vec2(4.0, 5.0), 6.0)
    ),
    mat2x3(
        vec3(7.0, 8.0, 9.0),
        vec3(10.0, 11.0, 12.0)
    )
);
const vec3 kConstructedMatrixColumn = kConstructedMatrixArray[1][bitCount(1)].zxy; // bitCount(1)=1，第1列 => vec3(12,10,11)
const float kConstructedMatrixDot = dot(
    kConstructedMatrixArray[0][0].zyx,
    kConstructedMatrixArray[1][1].xyz
);                                                              // (3,2,1) dot (10,11,12)，期望: 64.0

// [数组元素参与矩阵、向量和内置函数的深层复合调用]
const vec3 kDeepArrayConstructorExpression = max(
    transpose(mat3(
        kConstructorSourceArray[0].xyz,
        kConstructorSourceArray[1].xyz,
        kConstructorSourceArray[2].xyz
    ))[bitCount(1)].zyx,
    vec3[](
        abs(kConstructorSourceArray[0].wzy),
        sqrt(kConstructorSourceArray[2].zyx)
    )[1]
);                                                              // 期望: max(vec3(10,6,2), sqrt(vec3(11,10,9))) = vec3(10,6,3)

// ============================================================================
// 13. 结构体常量求值
// ============================================================================
struct ConstInner {
    int  v;
    vec3 data;
};

struct ConstOuter {
    ConstInner inner;
    float      scale;
    int        indices[3];
};

const ConstInner kStructInner = ConstInner(
    42,
    vec3(1.0, 2.0, 3.0)
);

const ConstOuter kStructOuter = ConstOuter(
    kStructInner,
    1.5,
    int[3](10, 20, 30)
);

const int  kStructNestedScalar  = kStructOuter.inner.v;                  // 期望: 42
const vec2 kStructNestedSwizzle = kStructOuter.inner.data.zy;            // 期望: vec2(3.0, 2.0)
const int  kStructArrayField    = kStructOuter.indices[bitCount(3) - 1]; // 期望: 20

const float kStructBuiltinExpression = dot(
    kStructOuter.inner.data,
    vec3(kStructOuter.scale)
); // 期望: 9.0

const ConstInner kStructArray[2] = ConstInner[](
    ConstInner(10, vec3(2.0)),
    ConstInner(20, vec3(4.0))
);

const int  kStructArrayNestedField = kStructArray[1].v;                               // 期望: 20
const vec3 kStructArrayBuiltin     = max(kStructArray[0].data, kStructArray[1].data); // 期望: vec3(4.0)

// ============================================================================
// 14. OverloadTest 风格的结构体大杂烩
// ============================================================================
struct InnerData {
    float Float;
    int   Int;
    vec3  float3;
    mat2  mat2Array1D_2[2];
    int   intArray2D_2x3[2][3];
};

struct MiddleData {
    InnerData Inner;
    vec4      vec4Array1D_3[3];
    mat4      float4x4;
};

struct OuterData {
    MiddleData Middle[2];
    dvec2      double2;
};

const InnerData kInnerDataA = InnerData(
    1.25,
    7,
    vec3(1.0, 2.0, 3.0),
    mat2[](
        mat2(1.0, 2.0, 3.0, 4.0),
        mat2(2.0, 0.0, 0.0, 3.0)
    ),
    int[2][3](
        int[3](1, 2, 3),
        int[3](4, 5, 6)
    )
);

const InnerData kInnerDataB = InnerData(
    -2.5,
    11,
    vec3(4.0, 5.0, 6.0),
    mat2[2](
        mat2(4.0),
        mat2(5.0, 6.0, 7.0, 8.0)
    ),
    int[][](
        int[](7, 8, 9),
        int[](10, 11, 12)
    )
);

const MiddleData kMiddleDataA = MiddleData(
    kInnerDataA,
    vec4[](
        vec4(1.0, 2.0, 3.0, 4.0),
        vec4(5.0, 6.0, 7.0, 8.0),
        vec4(9.0, 10.0, 11.0, 12.0)
    ),
    mat4(
        vec4(1.0, 2.0, 3.0, 4.0),
        vec4(5.0, 6.0, 7.0, 8.0),
        vec4(9.0, 10.0, 11.0, 12.0),
        vec4(13.0, 14.0, 15.0, 16.0)
    )
);

const MiddleData kMiddleDataB = MiddleData(
    kInnerDataB,
    vec4[3](
        vec4(16.0, 15.0, 14.0, 13.0),
        vec4(12.0, 11.0, 10.0, 9.0),
        vec4(8.0, 7.0, 6.0, 5.0)
    ),
    mat4(
        vec4(2.0, 0.0, 0.0, 0.0),
        vec4(0.0, 3.0, 0.0, 0.0),
        vec4(0.0, 0.0, 4.0, 0.0),
        vec4(0.0, 0.0, 0.0, 5.0)
    )
);

const OuterData kOuterData = OuterData(
    MiddleData[](kMiddleDataA, kMiddleDataB),
    dvec2(0.5, 1.5)
);

// [连续结构体成员访问]
const int   kDeepStructInt    = kOuterData.Middle[1].Inner.Int;    // 期望: 11
const float kDeepStructFloat  = kOuterData.Middle[0].Inner.Float;  // 期望: 1.25
const vec3  kDeepStructVector = kOuterData.Middle[1].Inner.float3; // 期望: vec3(4.0, 5.0, 6.0)

// [结构体字段数组的各层 length()]
const int kStructMiddleLength        = kOuterData.Middle.length();                            // 期望: 2
const int kStructMatrixArrayLength   = kOuterData.Middle[0].Inner.mat2Array1D_2.length();     // 期望: 2
const int kStructIntArrayOuterLength = kOuterData.Middle[1].Inner.intArray2D_2x3.length();    // 期望: 2
const int kStructIntArrayInnerLength = kOuterData.Middle[1].Inner.intArray2D_2x3[0].length(); // 期望: 3

// [结构体 → 数组 → 数组 → 标量]
const int kDeepStructArrayScalar = kOuterData.Middle[1].Inner.intArray2D_2x3[bitCount(3) - 1][bitCount(7) - 1]; // [1][2]，期望: 12

// [结构体 → 数组 → 向量 → Swizzle]
const vec3 kDeepStructArraySwizzle = kOuterData.Middle[0].vec4Array1D_3[2].wzx; // 期望: vec3(12.0, 11.0, 9.0)

// [结构体 → 矩阵数组 → 矩阵列 → Swizzle]
const vec2 kDeepStructMatrixArrayColumn = kOuterData.Middle[0].Inner.mat2Array1D_2[0][1].yx; // 第1列 (3,4)，期望: vec2(4.0, 3.0)
const vec3 kDeepStructMatrixColumn      = kOuterData.Middle[0].float4x4[2].wzx;              // 第2列 (9,10,11,12)，期望: vec3(12,11,9)

// [取出字段后继续调用内置函数]
const float kDeepStructDot = dot(
    kOuterData.Middle[0].Inner.float3,
    kOuterData.Middle[1].Inner.float3.zyx
); // (1,2,3) dot (6,5,4)，期望: 28.0

const float kDeepStructDeterminant = determinant(
    kOuterData.Middle[0].Inner.mat2Array1D_2[1]
); // determinant(mat2(2,0,0,3))，期望: 6.0

const vec4 kDeepStructBuiltinMix = max(
    abs(kOuterData.Middle[1].vec4Array1D_3[2].wzyx - vec4(7.0)),
    sqrt(kOuterData.Middle[0].vec4Array1D_3[2].wzyx)
); // max((2,1,0,1), sqrt(12,11,10,9))

// [结构体字段参与新的结构体构造]
const InnerData kReconstructedInnerData = InnerData(
    abs(kOuterData.Middle[1].Inner.Float),
    kOuterData.Middle[0].Inner.Int + kOuterData.Middle[1].Inner.Int,
    max(
        kOuterData.Middle[0].Inner.float3,
        kOuterData.Middle[1].Inner.float3.zyx
    ),
    mat2[](
        transpose(kOuterData.Middle[0].Inner.mat2Array1D_2[0]),
        inverse(kOuterData.Middle[0].Inner.mat2Array1D_2[1])
    ),
    int[2][3](
        kOuterData.Middle[0].Inner.intArray2D_2x3[1],
        kOuterData.Middle[1].Inner.intArray2D_2x3[0]
    )
);

const int  kReconstructedStructInt        = kReconstructedInnerData.Int;                  // 期望: 18
const vec3 kReconstructedStructVector     = kReconstructedInnerData.float3;               // max((1,2,3),(6,5,4)) => vec3(6,5,4)
const int  kReconstructedStructArrayValue = kReconstructedInnerData.intArray2D_2x3[1][2]; // 期望: 9

// [临时结构体构造结果立即进行成员、数组和 Swizzle 访问]
const int kTemporaryStructMember = InnerData(
    3.5,
    99,
    vec3(7.0, 8.0, 9.0),
    mat2[](mat2(1.0), mat2(2.0)),
    int[][](int[](1, 2, 3), int[](4, 5, 6))
).intArray2D_2x3[1][0];                                       // 期望: 4

const vec2 kTemporaryNestedStructSwizzle = MiddleData(
    kInnerDataA,
    vec4[](vec4(1.0), vec4(2.0), vec4(3.0, 4.0, 5.0, 6.0)),
    mat4(1.0)
).vec4Array1D_3[2].wz;                                        // 期望: vec2(6.0, 5.0)

// [结构体数组构造结果立即索引并访问深层字段]
const float kTemporaryStructArrayField = OuterData[](
    kOuterData,
    OuterData(
        MiddleData[](kMiddleDataB, kMiddleDataA),
        dvec2(2.5, 3.5)
    )
)[1].Middle[0].Inner.Float;                                   // 期望: -2.5

const dvec2 kTemporaryStructArrayDouble = OuterData[](
    kOuterData,
    OuterData(
        MiddleData[](kMiddleDataB, kMiddleDataA),
        dvec2(2.5, 3.5)
    )
)[1].double2.yx;                                              // 期望: dvec2(3.5, 2.5)

// ============================================================================
// 15. 结构体大括号初始化
// ============================================================================
// [简单结构体]
const ConstInner kBraceSimpleInner = {
    64,
    vec3(2.0, 4.0, 8.0)
};
const int  kBraceSimpleInnerValue   = kBraceSimpleInner.v;        // 期望: 64
const vec3 kBraceSimpleInnerSwizzle = kBraceSimpleInner.data.zyx; // 期望: vec3(8.0, 4.0, 2.0)

// [嵌套结构体及结构体字段数组全部使用大括号]
const ConstOuter kBraceNestedOuter = {
    {
        65,
        vec3(3.0, 6.0, 9.0)
    },
    2.5,
    { 100, 200, 300 }
};
const int kBraceNestedStructValue = kBraceNestedOuter.inner.v;    // 期望: 65
const int kBraceNestedArrayValue  = kBraceNestedOuter.indices[1]; // 期望: 200
const float kBraceNestedBuiltin = dot(
    kBraceNestedOuter.inner.data,
    vec3(kBraceNestedOuter.scale)
);                                                                // (3+6+9)*2.5，期望: 45.0

// [结构体数组，每个元素均为大括号初始化的结构体]
const ConstInner kBraceStructArray[3] = {
    { 10, vec3(1.0, 2.0, 3.0) },
    { 20, vec3(4.0, 5.0, 6.0) },
    { 30, vec3(7.0, 8.0, 9.0) }
};
const int kBraceStructArrayMember = kBraceStructArray[bitCount(3)].v; // bitCount(3)=2，期望: 30
const vec4 kBraceStructArrayMixed = vec4(
    kBraceStructArray[2].data.yx,
    kBraceStructArray[0].data.zy
);                                                              // 期望: vec4(8.0, 7.0, 3.0, 2.0)

// [OverloadTest 的 InnerData 完全使用嵌套大括号初始化]
const InnerData kBraceInnerData = {
    6.25,
    123,
    vec3(2.0, 3.0, 4.0),
    {
        mat2(1.0, 2.0, 3.0, 4.0),
        mat2(5.0, 6.0, 7.0, 8.0)
    },
    {
        { 11, 12, 13 },
        { 21, 22, 23 }
    }
};
const float kBraceInnerDataFloat        = kBraceInnerData.Float;                  // 期望: 6.25
const vec2  kBraceInnerDataMatrixColumn = kBraceInnerData.mat2Array1D_2[1][0].yx; // 第0列 (5,6)，期望: vec2(6.0, 5.0)
const int   kBraceInnerDataDeepArray    = kBraceInnerData.intArray2D_2x3[1][2];   // 期望: 23

// [MiddleData：结构体、向量数组和矩阵混合大括号初始化]
const MiddleData kBraceMiddleData = {
    {
        7.5,
        321,
        vec3(5.0, 10.0, 15.0),
        {
            mat2(2.0),
            mat2(3.0)
        },
        {
            { 31, 32, 33 },
            { 41, 42, 43 }
        }
    },
    {
        vec4(1.0, 2.0, 3.0, 4.0),
        vec4(5.0, 6.0, 7.0, 8.0),
        vec4(9.0, 10.0, 11.0, 12.0)
    },
    mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 2.0, 0.0, 0.0),
        vec4(0.0, 0.0, 3.0, 0.0),
        vec4(0.0, 0.0, 0.0, 4.0)
    )
};
const int  kBraceMiddleDeepMember    = kBraceMiddleData.Inner.intArray2D_2x3[0][1]; // 期望: 32
const vec3 kBraceMiddleVectorSwizzle = kBraceMiddleData.vec4Array1D_3[1].wzx;       // 期望: vec3(8.0, 7.0, 5.0)
const vec4 kBraceMiddleMatrixVector  = kBraceMiddleData.float4x4 * vec4(1.0);       // 期望: vec4(1.0, 2.0, 3.0, 4.0)
// [OuterData 全层级大括号初始化]
const OuterData kBraceOuterData = {
    {
        kBraceMiddleData,
        {
            kBraceInnerData,
            {
                vec4(20.0, 21.0, 22.0, 23.0),
                vec4(24.0, 25.0, 26.0, 27.0),
                vec4(28.0, 29.0, 30.0, 31.0)
            },
            mat4(5.0)
        }
    },
    dvec2(9.5, 10.5)
};
const int   kBraceOuterDeepScalar    = kBraceOuterData.Middle[1].Inner.intArray2D_2x3[1][1]; // 期望: 22
const vec2  kBraceOuterDeepSwizzle   = kBraceOuterData.Middle[1].vec4Array1D_3[2].wz;        // 期望: vec2(31.0, 30.0)
const dvec2 kBraceOuterDoubleSwizzle = kBraceOuterData.double2.yx;                           // 期望: dvec2(10.5, 9.5)

// [大括号初始化的 OuterData 数组]
const OuterData kBraceOuterArray[2] = {
    kOuterData,
    {
        { kMiddleDataB, kBraceMiddleData },
        dvec2(20.5, 30.5)
    }
};
const float kBraceOuterArrayNestedFloat = kBraceOuterArray[1].Middle[1].Inner.Float; // 期望: 7.5
const vec3 kBraceOuterArrayNestedBuiltin = max(
    kBraceOuterArray[0].Middle[0].Inner.float3,
    kBraceOuterArray[1].Middle[0].Inner.float3
); // max((1,2,3),(4,5,6))，期望: vec3(4,5,6)

// ============================================================================
// 16. 向量、矩阵与全层级大括号初始化
// ============================================================================
// [向量分量按目标类型解析，并进行允许的隐式转换]
const vec4  kBraceVector        = { 1, 2.5, 3, 4.5 };
const ivec3 kBraceIntegerVector = { -7, 8, -9 };
const bvec4 kBraceBooleanVector = { true, false, true, false };
const vec3  kBraceVectorResult  = max(kBraceVector.wzy, abs(vec3(kBraceIntegerVector))); // 期望: vec3(7.0, 8.0, 9.0)

// [矩阵的每个初始化项对应一列向量]
const mat2 kBraceMatrix2 = {
    { 1.0, 2.0 },
    { 3.0, 4.0 }
};
const mat2x3 kBraceMatrix2x3 = {
    { 1, 2, 3 },
    { 4, 5, 6 }
};
const vec2  kBraceMatrixColumnSwizzle = kBraceMatrix2x3[1].zy;                 // 期望: vec2(6.0, 5.0)
const float kBraceMatrixDeterminant   = determinant(kBraceMatrix2);            // 期望: -2.0
const vec3  kBraceMatrixVectorProduct = kBraceMatrix2x3 * kBraceVector.xy;     // 期望: vec3(11.0, 14.5, 18.0)

// [未定长数组从初始化列表推导外层长度]
const vec3 kBraceDeducedVectorArray[] = {
    { 1.0, 2.0, 3.0 },
    { 4.0, 5.0, 6.0 },
    { 7.0, 8.0, 9.0 }
};
const int  kBraceDeducedVectorArrayLength = kBraceDeducedVectorArray.length(); // 期望: 3
const vec2 kBraceDeducedVectorArrayValue  = kBraceDeducedVectorArray[2].zx;    // 期望: vec2(9.0, 7.0)

const mat2 kBraceDeducedMatrixArray[] = {
    { { 1.0, 2.0 }, { 3.0, 4.0 } },
    { { 5.0, 6.0 }, { 7.0, 8.0 } }
};
const vec2  kBraceDeducedMatrixColumn = kBraceDeducedMatrixArray[1][0].yx;        // 期望: vec2(6.0, 5.0)
const float kBraceDeducedMatrixDet    = determinant(kBraceDeducedMatrixArray[1]); // 期望: -2.0

// [结构体字段中的向量、矩阵和数组全部使用大括号]
const InnerData kBraceAllCompositeInner = {
    8.5,
    456,
    { 2.0, 4.0, 8.0 },
    {
        { { 1.0, 2.0 }, { 3.0, 4.0 } },
        { { 5.0, 6.0 }, { 7.0, 8.0 } }
    },
    {
        { 51, 52, 53 },
        { 61, 62, 63 }
    }
};
const vec3  kBraceAllCompositeVector = kBraceAllCompositeInner.float3.zyx;             // 期望: vec3(8.0, 4.0, 2.0)
const vec2  kBraceAllCompositeColumn = kBraceAllCompositeInner.mat2Array1D_2[1][1].yx; // 期望: vec2(8.0, 7.0)
const int   kBraceAllCompositeArray  = kBraceAllCompositeInner.intArray2D_2x3[1][2];   // 期望: 63
const float kBraceAllCompositeStable = dot(
    kBraceAllCompositeInner.float3,
    vec3(kBraceAllCompositeInner.mat2Array1D_2[0] * kBraceVector.xy, kBraceAllCompositeInner.Float)
); // dot((2,4,8), (8.5,12.0,8.5))，期望: 133.0

void main() {
    return;
}
