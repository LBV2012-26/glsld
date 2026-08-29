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
const int16_t  kHalfBits    = halfBitsToInt16(1.0hf);            // 期望: 0x3C00 (15360)
const float16_t kInt16ToHalf= int16BitsToHalf(int16_t(0x3C00));  // 期望: 1.0
const int64_t  kDoubleBits  = doubleBitsToInt64(1.0);            // 期望: 0x3FF0000000000000 (4607182418800017408)
const double   kInt64ToDouble = int64BitsToDouble(kDoubleBits);  // 期望: 1.0

// ============================================================================
// 4. 打包与解包函数 (Section 8.4 及 EXT 扩展)
// ============================================================================
// [GLSL 4.60 核心打包/解包]
const uint  kPackUnorm2x16  = packUnorm2x16(vec2(0.0, 1.0));    // 期望: 0xFFFF0000u (4294901760)
const vec2  kUnpackUnorm2x16= unpackUnorm2x16(kPackUnorm2x16);  // 期望: vec2(0.0, 1.0)
const uint  kPackHalf2x16   = packHalf2x16(vec2(1.0, 2.0));     // 期望: 0x40003C00u (1074266112)
const vec2  kUnpackHalf2x16 = unpackHalf2x16(kPackHalf2x16);    // 期望: vec2(1.0, 2.0)
const uint  kPackUnorm4x8   = packUnorm4x8(vec4(1.0, 0.0, 0.0, 1.0)); // 期望: 0xFF0000FFu (4278190335)
const vec4  kUnpackUnorm4x8 = unpackUnorm4x8(kPackUnorm4x8);    // 期望: vec4(1.0, 0.0, 0.0, 1.0)
const double kPackDouble2x32= packDouble2x32(uvec2(0, 0x3FF00000u)); // 期望: 1.0
const uvec2 kUnpackDouble2x32= unpackDouble2x32(kPackDouble2x32); // 期望: uvec2(0, 1072693248u)

// [GL_EXT 扩展整数/半精度打包]
const uint32_t  kPackFloat2x16 = packFloat2x16(f16vec2(1.0, 2.0)); // 期望: 0x40003C00u (1074266112)
const f16vec2   kUnpackFloat2x16= unpackFloat2x16(kPackFloat2x16); // 期望: f16vec2(1.0, 2.0)
const int32_t   kPackInt2x16   = packInt2x16(i16vec2(-1, 1));   // 期望: 0x0001FFFF (131071)
const i16vec2   kUnpackInt2x16 = unpackInt2x16(kPackInt2x16);   // 期望: i16vec2(-1, 1)
const int64_t   kPackInt2x32   = packInt2x32(ivec2(-1, 1));     // 期望: 0x00000001FFFFFFFF (8589934591)
const ivec2     kUnpackInt2x32 = unpackInt2x32(kPackInt2x32);   // 期望: ivec2(-1, 1)

// ============================================================================
// 5. 几何函数 (Section 8.5)
// ============================================================================
const float kLengthVal      = length(vec3(0.0, 3.0, 4.0));      // 期望: 5.0
const float kDistanceVal    = distance(vec2(1.0, 2.0), vec2(4.0, 6.0)); // 期望: 5.0
const float kDotVal         = dot(vec3(1.0, 2.0, 3.0), vec3(4.0, 5.0, 6.0)); // 期望: 32.0
const vec3  kCrossVal       = cross(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0)); // 期望: vec3(0.0, 0.0, 1.0)
const vec2  kNormalizeVal   = normalize(vec2(0.0, 5.0));        // 期望: vec2(0.0, 1.0)
const vec3  kFaceForwardVal = faceforward(vec3(1.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, -1.0)); // 期望: vec3(1.0)
const vec2  kReflectVal     = reflect(vec2(1.0, -1.0), vec2(0.0, 1.0)); // 期望: vec2(1.0, 1.0)
const vec2  kRefractVal     = refract(vec2(0.0, 1.0), vec2(0.0, -1.0), 1.0); // 期望: vec2(0.0, 1.0)

// ============================================================================
// 6. 矩阵操作函数 (Section 8.6)
// ============================================================================
const mat2  kMatA           = mat2(1.0, 2.0, 3.0, 4.0);
const mat2  kMatB           = mat2(2.0, 0.0, 1.0, 2.0);
const mat2  kMatCompMult    = matrixCompMult(kMatA, kMatB);     // 期望: mat2(2.0, 0.0, 3.0, 8.0)
const mat2  kTransposeMat   = transpose(kMatA);                 // 期望: mat2(1.0, 3.0, 2.0, 4.0)
const float kDeterminantVal = determinant(kMatA);               // 期望: -2.0 (1*4 - 2*3)
const mat2  kInverseMat     = inverse(kMatA);                   // 期望: mat2(-2.0, 1.0, 1.5, -0.5)
const mat2x3 kOuterProd     = outerProduct(vec3(1.0, 2.0, 3.0), vec2(4.0, 5.0)); // 期望: 2列3行矩阵

// [矩阵与 Swizzle 组合]
const vec4  kMatrixSwizzleSource = vec4(1.0, 2.0, 3.0, 4.0);
const mat2  kMatrixFromSwizzles  = mat2(
    kMatrixSwizzleSource.yx,
    kMatrixSwizzleSource.wz
);                                                              // 期望: mat2(2.0, 1.0, 4.0, 3.0)

const vec2 kMatrixTimesSwizzle = (
    kMatrixFromSwizzles * kMatrixSwizzleSource.zx
).yx;                                                           // 乘法结果 (10,6) 重排后期望: vec2(6.0, 10.0)

const vec2 kSwizzleTimesMatrix = (
    kMatrixSwizzleSource.xz * kMatrixFromSwizzles
).yx;                                                           // 乘法结果 (5,13) 重排后期望: vec2(13.0, 5.0)

const mat2x3 kMatrixOuterFromSwizzles = outerProduct(
    kMatrixSwizzleSource.wzy,
    kMatrixSwizzleSource.yx
);                                                              // 期望: mat2x3(8,6,4, 4,3,2)

// 矩阵不能直接 Swizzle；先索引取出列向量后再 Swizzle。
const vec2 kMatrixColumnSwizzle =
    kMatrixOuterFromSwizzles[1].zy;                             // 第1列 (4,3,2) => vec2(2.0, 3.0)

const vec2 kTransposeColumnSwizzle =
    transpose(kMatrixOuterFromSwizzles)[2].yx;                  // 第2列 (4,2) => vec2(2.0, 4.0)

// ============================================================================
// 7. 向量关系判断函数 (Section 8.7)
// ============================================================================
const bvec3 kLessThanVal    = lessThan(vec3(1.0, 5.0, 3.0), vec3(2.0, 4.0, 3.0)); // 期望: bvec3(true, false, false)
const bvec2 kEqualVal       = equal(uvec2(5u, 10u), uvec2(5u, 20u)); // 期望: bvec2(true, false)
const bool  kAnyVal         = any(kLessThanVal);                // 期望: true
const bool  kAllVal         = all(kLessThanVal);                // 期望: false
const bvec2 kNotVal         = not(bvec2(true, false));          // 期望: bvec2(false, true)

// ============================================================================
// 8. 整数位级操作函数 (Section 8.8 - 重点验证 32 位截断)
// ============================================================================
const int   kBitCountPos    = bitCount(0x00FF00FF);             // 期望: 16
const int   kBitCountNeg    = bitCount(-1);                     // 期望: 32 (32位全1)
const int   kFindLSBVal     = findLSB(0x00000080);              // 期望: 7
const int   kFindLSBZero    = findLSB(0);                       // 期望: -1
const int   kFindMSBPos     = findMSB(0x00000080);              // 期望: 7
const int   kFindMSBZero    = findMSB(0);                       // 期望: -1
const int   kFindMSBNeg     = findMSB(-1);                      // 期望: -1
const uint  kBitReverseVal  = bitfieldReverse(0x80000000u);     // 期望: 1u (0x00000001u)
const uint  kExtractVal     = bitfieldExtract(0x1234u, 4, 4);   // 期望: 3u (提取0x3)
const uint  kInsertVal      = bitfieldInsert(0x0000u, 0xAu, 4, 4); // 期望: 0x00A0u (160u)

// ============================================================================
// 9. 向量 Swizzle
// ============================================================================
const vec4  kSwizzleSource = vec4(1.0, 2.0, 3.0, 4.0);

// [单分量、重排与重复分量]
const float kSwizzleScalar = kSwizzleSource.z;                 // 期望: 3.0
const vec2  kSwizzleReorder= kSwizzleSource.yx;                // 期望: vec2(2.0, 1.0)
const vec4  kSwizzleRepeat = kSwizzleSource.wwxx;              // 期望: vec4(4.0, 4.0, 1.0, 1.0)

// [xyzw / rgba / stpq 三套命名]
const vec3  kSwizzlePosition = kSwizzleSource.zyx;             // 期望: vec3(3.0, 2.0, 1.0)
const vec4  kSwizzleColor    = kSwizzleSource.bgra;            // 期望: vec4(3.0, 2.0, 1.0, 4.0)
const vec4  kSwizzleTexture  = kSwizzleSource.qpts;            // 期望: vec4(4.0, 3.0, 2.0, 1.0)

// [整数、无符号整数与布尔向量]
const ivec3 kSwizzleInt    = ivec4(-1, 2, -3, 4).wzx;          // 期望: ivec3(4, -3, -1)
const uvec2 kSwizzleUint   = uvec3(10u, 20u, 30u).zx;          // 期望: uvec2(30u, 10u)
const bvec4 kSwizzleBool   = bvec2(true, false).xyyx;          // 期望: bvec4(true, false, false, true)

// [链式 Swizzle 与表达式结果 Swizzle]
const vec2  kSwizzleChain  = kSwizzleSource.wzy.yx;            // wzy = (4,3,2), yx = (3,4)
const vec2  kSwizzleExpr   = max(kSwizzleSource, vec4(2.0)).wz; // 期望: vec2(4.0, 3.0)

// [复杂混合复合调用]
const vec4 kSwizzleComplexConstruct = vec4(
    kSwizzleSource.wy,
    max(kSwizzleSource.zx, vec2(2.5))
).zwyx;                                                        // 期望: vec4(3.0, 2.5, 2.0, 4.0)

const vec4 kSwizzleComplexBuiltins = mix(
    abs(vec4(-1.0, -2.0, 3.0, -4.0)).wzyx,
    max(kSwizzleSource, vec4(2.0)).xyzw,
    bvec4(true, false, true, false)
).bgra;                                                        // 期望: vec4(3.0, 3.0, 2.0, 1.0)

const vec4 kSwizzleComplexArithmetic = (
    pow(kSwizzleSource.wzyx, vec4(2.0)) + vec4(1.0)
).zwxy;                                                        // 期望: vec4(5.0, 2.0, 17.0, 10.0)

const ivec4 kSwizzleComplexBits = bitCount(
    ivec4(-1, 0x000000F0, 0x0000FF00, 1).wzyx
).yxwz;                                                        // 期望: ivec4(8, 1, 32, 4)

const bvec4 kSwizzleComplexRelation = not(
    lessThan(kSwizzleSource.wzyx, vec4(3.0)).yxwz
).zxyw;                                                        // 期望: bvec4(false, true, true, false)

const float kSwizzleComplexScalar = sqrt(pow(
    max(kSwizzleSource.wzy.x, kSwizzleSource.yx.y),
    2.0
));                                                            // 期望: 4.0

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
];                                   // 总大小期望: 2 + 5 + 1 + 16 = 24

const int kFinalArraySize = array_dim_test.length(); // 期望: 24

void main() {
    return;
}
