// ===================================================================
// Built-in Functions Extracted from GLSL Specification
// ===================================================================

// Radians
// Converts degrees to radians, i.e., (PI / 180) * degrees.
// [Generic] genFType radians(genFType degrees);
float radians(float _Degrees);
vec2 radians(vec2 _Degrees);
vec3 radians(vec3 _Degrees);
vec4 radians(vec4 _Degrees);

// Degrees
// Converts radians to degrees, i.e., (180 / PI) * radians.
// [Generic] genFType degrees(genFType radians);
float degrees(float _Radians);
vec2 degrees(vec2 _Radians);
vec3 degrees(vec3 _Radians);
vec4 degrees(vec4 _Radians);

// Sin
// The standard trigonometric sine function.
// [Generic] genFType sin(genFType angle);
float sin(float _Angle);
vec2 sin(vec2 _Angle);
vec3 sin(vec3 _Angle);
vec4 sin(vec4 _Angle);

// Cos
// The standard trigonometric cosine function.
// [Generic] genFType cos(genFType angle);
float cos(float _Angle);
vec2 cos(vec2 _Angle);
vec3 cos(vec3 _Angle);
vec4 cos(vec4 _Angle);

// Tan
// The standard trigonometric tangent.
// [Generic] genFType tan(genFType angle);
float tan(float _Angle);
vec2 tan(vec2 _Angle);
vec3 tan(vec3 _Angle);
vec4 tan(vec4 _Angle);

// Asin
// Arc sine. Returns an angle whose sine is x.
// The range of values returned by this function is [-PI / 2, PI / 2].
// Results are undefined if |x| > 1.
// [Generic] genFType asin(genFType x);
float asin(float _X);
vec2 asin(vec2 _X);
vec3 asin(vec3 _X);
vec4 asin(vec4 _X);

// Acos
// Arc cosine. Returns an angle whose cosine is x.
// The range of values returned by this function is [0, PI].
// Results are undefined if |x| > 1.
// [Generic] genFType acos(genFType x);
float acos(float _X);
vec2 acos(vec2 _X);
vec3 acos(vec3 _X);
vec4 acos(vec4 _X);

// Atan
// Arc tangent. Returns an angle whose tangent is y / x.
// The signs of x and y are used to determine what quadrant the angle is in.
// The range of values returned by this function is [-PI, PI].
// Results are undefined if x and y are both 0.
// [Generic] genFType atan(genFType y, genFType x);
float atan(float _Y, float _X);
vec2 atan(vec2 _Y, vec2 _X);
vec3 atan(vec3 _Y, vec3 _X);
vec4 atan(vec4 _Y, vec4 _X);

// Sinh
// Returns the hyperbolic sine function (e^x^ - e^-x^) / 2.
// [Generic] genFType sinh(genFType x);
float sinh(float _X);
vec2 sinh(vec2 _X);
vec3 sinh(vec3 _X);
vec4 sinh(vec4 _X);

// Cosh
// Returns the hyperbolic cosine function (e^x^ + e^-x^) / 2.
// [Generic] genFType cosh(genFType x);
float cosh(float _X);
vec2 cosh(vec2 _X);
vec3 cosh(vec3 _X);
vec4 cosh(vec4 _X);

// Tanh
// Returns the hyperbolic tangent function sinh(x) / cosh(x).
// [Generic] genFType tanh(genFType x);
float tanh(float _X);
vec2 tanh(vec2 _X);
vec3 tanh(vec3 _X);
vec4 tanh(vec4 _X);

// Asinh
// Arc hyperbolic sine; returns the inverse of sinh.
// [Generic] genFType asinh(genFType x);
float asinh(float _X);
vec2 asinh(vec2 _X);
vec3 asinh(vec3 _X);
vec4 asinh(vec4 _X);

// Acosh
// Arc hyperbolic cosine. Returns the non-negative inverse of cosh.
// Results are undefined if x < 1.
// [Generic] genFType acosh(genFType x);
float acosh(float _X);
vec2 acosh(vec2 _X);
vec3 acosh(vec3 _X);
vec4 acosh(vec4 _X);

// Atanh
// Arc hyperbolic tangent. Returns the inverse of tanh.
// Results are undefined if |x| >= 1.
// [Generic] genFType atanh(genFType x);
float atanh(float _X);
vec2 atanh(vec2 _X);
vec3 atanh(vec3 _X);
vec4 atanh(vec4 _X);

// Pow
// Returns x raised to the y power, i.e., x^y^.
// Results are undefined if x < 0.
// Results are undefined if x = 0 and y <= 0.
// [Generic] genFType pow(genFType x, genFType y);
float pow(float _X, float _Y);
vec2 pow(vec2 _X, vec2 _Y);
vec3 pow(vec3 _X, vec3 _Y);
vec4 pow(vec4 _X, vec4 _Y);

// Exp
// Returns the natural exponentiation of x, i.e., e^x^.
// [Generic] genFType exp(genFType x);
float exp(float _X);
vec2 exp(vec2 _X);
vec3 exp(vec3 _X);
vec4 exp(vec4 _X);

// Log
// Returns the natural logarithm of x, i.e., returns the value y
// which satisfies the equation x = e^y^.
// Results are undefined if x <= 0.
// [Generic] genFType log(genFType x);
float log(float _X);
vec2 log(vec2 _X);
vec3 log(vec3 _X);
vec4 log(vec4 _X);

// Exp2
// Returns 2 raised to the x power, i.e., 2^x^.
// [Generic] genFType exp2(genFType x);
float exp2(float _X);
vec2 exp2(vec2 _X);
vec3 exp2(vec3 _X);
vec4 exp2(vec4 _X);

// Log2
// Returns the base 2 logarithm of x, i.e., returns the value y which
// satisfies the equation x = 2^y^.
// Results are undefined if x <= 0.
// [Generic] genFType log2(genFType x);
float log2(float _X);
vec2 log2(vec2 _X);
vec3 log2(vec3 _X);
vec4 log2(vec4 _X);

// Sqrt
// Returns sqrt(x).
// Results are undefined if x < 0.
// [Generic] genFType sqrt(genFType x);
float sqrt(float _X);
vec2 sqrt(vec2 _X);
vec3 sqrt(vec3 _X);
vec4 sqrt(vec4 _X);
// [Generic] genDType sqrt(genDType x);
double sqrt(double _X);
dvec2 sqrt(dvec2 _X);
dvec3 sqrt(dvec3 _X);
dvec4 sqrt(dvec4 _X);

// Inverse Sqrt
// Returns 1 / sqrt(x).
// Results are undefined if x <= 0.
// [Generic] genFType inversesqrt(genFType x);
float inversesqrt(float _X);
vec2 inversesqrt(vec2 _X);
vec3 inversesqrt(vec3 _X);
vec4 inversesqrt(vec4 _X);
// [Generic] genDType inversesqrt(genDType x);
double inversesqrt(double _X);
dvec2 inversesqrt(dvec2 _X);
dvec3 inversesqrt(dvec3 _X);
dvec4 inversesqrt(dvec4 _X);

// Abs
// Returns x if x >= 0; otherwise it returns -x.
// [Generic] genFType abs(genFType x);
float abs(float _X);
vec2 abs(vec2 _X);
vec3 abs(vec3 _X);
vec4 abs(vec4 _X);
// [Generic] genIType abs(genIType x);
int abs(int _X);
ivec2 abs(ivec2 _X);
ivec3 abs(ivec3 _X);
ivec4 abs(ivec4 _X);
// [Generic] genDType abs(genDType x);
double abs(double _X);
dvec2 abs(dvec2 _X);
dvec3 abs(dvec3 _X);
dvec4 abs(dvec4 _X);

// Sign
// Returns 1.0 if x > 0, 0.0 if x = 0, or -1.0 if x < 0.
// [Generic] genFType sign(genFType x);
float sign(float _X);
vec2 sign(vec2 _X);
vec3 sign(vec3 _X);
vec4 sign(vec4 _X);
// [Generic] genIType sign(genIType x);
int sign(int _X);
ivec2 sign(ivec2 _X);
ivec3 sign(ivec3 _X);
ivec4 sign(ivec4 _X);
// [Generic] genDType sign(genDType x);
double sign(double _X);
dvec2 sign(dvec2 _X);
dvec3 sign(dvec3 _X);
dvec4 sign(dvec4 _X);

// Floor
// Returns a value equal to the nearest integer that is less than or equal to x.
// [Generic] genFType floor(genFType x);
float floor(float _X);
vec2 floor(vec2 _X);
vec3 floor(vec3 _X);
vec4 floor(vec4 _X);
// [Generic] genDType floor(genDType x);
double floor(double _X);
dvec2 floor(dvec2 _X);
dvec3 floor(dvec3 _X);
dvec4 floor(dvec4 _X);

// Trunc
// Returns a value equal to the nearest integer to x whose absolute
// value is not larger than the absolute value of x.
// [Generic] genFType trunc(genFType x);
float trunc(float _X);
vec2 trunc(vec2 _X);
vec3 trunc(vec3 _X);
vec4 trunc(vec4 _X);
// [Generic] genDType trunc(genDType x);
double trunc(double _X);
dvec2 trunc(dvec2 _X);
dvec3 trunc(dvec3 _X);
dvec4 trunc(dvec4 _X);

// Round
// Returns a value equal to the nearest integer to x.
// The fraction 0.5 will round in a direction chosen by the
// implementation, presumably the direction that is fastest.
// This includes the possibility that round(x) returns the same value
// as roundEven(x) for all values of x.
// [Generic] genFType round(genFType x);
float round(float _X);
vec2 round(vec2 _X);
vec3 round(vec3 _X);
vec4 round(vec4 _X);
// [Generic] genDType round(genDType x);
double round(double _X);
dvec2 round(dvec2 _X);
dvec3 round(dvec3 _X);
dvec4 round(dvec4 _X);

// Round Even
// Returns a value equal to the nearest integer to x.
// A fractional part of 0.5 will round toward the nearest even integer.
// (Both 3.5 and 4.5 for x will return 4.0.)
// [Generic] genFType roundEven(genFType x);
float roundEven(float _X);
vec2 roundEven(vec2 _X);
vec3 roundEven(vec3 _X);
vec4 roundEven(vec4 _X);
// [Generic] genDType roundEven(genDType x);
double roundEven(double _X);
dvec2 roundEven(dvec2 _X);
dvec3 roundEven(dvec3 _X);
dvec4 roundEven(dvec4 _X);

// Ceil
// Returns a value equal to the nearest integer that is greater than or equal to x.
// [Generic] genFType ceil(genFType x);
float ceil(float _X);
vec2 ceil(vec2 _X);
vec3 ceil(vec3 _X);
vec4 ceil(vec4 _X);
// [Generic] genDType ceil(genDType x);
double ceil(double _X);
dvec2 ceil(dvec2 _X);
dvec3 ceil(dvec3 _X);
dvec4 ceil(dvec4 _X);

// Fract
// Returns x - floor(x).
// [Generic] genFType fract(genFType x);
float fract(float _X);
vec2 fract(vec2 _X);
vec3 fract(vec3 _X);
vec4 fract(vec4 _X);
// [Generic] genDType fract(genDType x);
double fract(double _X);
dvec2 fract(dvec2 _X);
dvec3 fract(dvec3 _X);
dvec4 fract(dvec4 _X);

// Mod
// Modulus. Returns x - y  floor * (x / y).
// Note that implementations may use a cheap approximation to the remainder,
// and the error can be large due to the discontinuity in floor. This can
// produce mathematically unexpected results in some cases, such as
// mod(x, x) computing x rather than 0, and can also cause the result
// to have a different sign than the infinitely precise result.
// [Generic] genFType mod(genFType x, float y);
float mod(float _X, float _Y);
vec2 mod(vec2 _X, float _Y);
vec3 mod(vec3 _X, float _Y);
vec4 mod(vec4 _X, float _Y);
// [Generic] genFType mod(genFType x, genFType y);
vec2 mod(vec2 _X, vec2 _Y);
vec3 mod(vec3 _X, vec3 _Y);
vec4 mod(vec4 _X, vec4 _Y);
// [Generic] genDType mod(genDType x, double y);
double mod(double _X, double _Y);
dvec2 mod(dvec2 _X, double _Y);
dvec3 mod(dvec3 _X, double _Y);
dvec4 mod(dvec4 _X, double _Y);
// [Generic] genDType mod(genDType x, genDType y);
dvec2 mod(dvec2 _X, dvec2 _Y);
dvec3 mod(dvec3 _X, dvec3 _Y);
dvec4 mod(dvec4 _X, dvec4 _Y);

// Modf
// Returns the fractional part of x and sets i to the integer part (as
// a whole number floating-point value).
// Both the return value and the output parameter will have the same sign as x.
// If x has the value +/- Inf, the return value should be NaN and must
// be either NaN or 0.0.
// For highp non-constant expressions, the value returned must be
// consistent.
// [Generic] genFType modf(genFType x, out genFType i);
float modf(float _X, out float _I);
vec2 modf(vec2 _X, out vec2 _I);
vec3 modf(vec3 _X, out vec3 _I);
vec4 modf(vec4 _X, out vec4 _I);
// [Generic] genDType modf(genDType x, out genDType i);
double modf(double _X, out double _I);
dvec2 modf(dvec2 _X, out dvec2 _I);
dvec3 modf(dvec3 _X, out dvec3 _I);
dvec4 modf(dvec4 _X, out dvec4 _I);

// Min
// Returns y if y < x; otherwise it returns x.
// Which operand is the result is undefined if one of the operands is a NaN.
// [Generic] genFType min(genFType x, genFType y);
float min(float _X, float _Y);
vec2 min(vec2 _X, vec2 _Y);
vec3 min(vec3 _X, vec3 _Y);
vec4 min(vec4 _X, vec4 _Y);
// [Generic] genFType min(genFType x, float y);
vec2 min(vec2 _X, float _Y);
vec3 min(vec3 _X, float _Y);
vec4 min(vec4 _X, float _Y);
// [Generic] genDType min(genDType x, genDType y);
double min(double _X, double _Y);
dvec2 min(dvec2 _X, dvec2 _Y);
dvec3 min(dvec3 _X, dvec3 _Y);
dvec4 min(dvec4 _X, dvec4 _Y);
// [Generic] genDType min(genDType x, double y);
dvec2 min(dvec2 _X, double _Y);
dvec3 min(dvec3 _X, double _Y);
dvec4 min(dvec4 _X, double _Y);
// [Generic] genIType min(genIType x, genIType y);
int min(int _X, int _Y);
ivec2 min(ivec2 _X, ivec2 _Y);
ivec3 min(ivec3 _X, ivec3 _Y);
ivec4 min(ivec4 _X, ivec4 _Y);
// [Generic] genIType min(genIType x, int y);
ivec2 min(ivec2 _X, int _Y);
ivec3 min(ivec3 _X, int _Y);
ivec4 min(ivec4 _X, int _Y);
// [Generic] genUType min(genUType x, genUType y);
uint min(uint _X, uint _Y);
uvec2 min(uvec2 _X, uvec2 _Y);
uvec3 min(uvec3 _X, uvec3 _Y);
uvec4 min(uvec4 _X, uvec4 _Y);
// [Generic] genUType min(genUType x, uint y);
uvec2 min(uvec2 _X, uint _Y);
uvec3 min(uvec3 _X, uint _Y);
uvec4 min(uvec4 _X, uint _Y);

// Max
// Returns y if x < y; otherwise it returns x.
// Which operand is the result is undefined if one of the operands is a NaN.
// [Generic] genFType max(genFType x, genFType y);
float max(float _X, float _Y);
vec2 max(vec2 _X, vec2 _Y);
vec3 max(vec3 _X, vec3 _Y);
vec4 max(vec4 _X, vec4 _Y);
// [Generic] genFType max(genFType x, float y);
vec2 max(vec2 _X, float _Y);
vec3 max(vec3 _X, float _Y);
vec4 max(vec4 _X, float _Y);
// [Generic] genDType max(genDType x, genDType y);
double max(double _X, double _Y);
dvec2 max(dvec2 _X, dvec2 _Y);
dvec3 max(dvec3 _X, dvec3 _Y);
dvec4 max(dvec4 _X, dvec4 _Y);
// [Generic] genDType max(genDType x, double y);
dvec2 max(dvec2 _X, double _Y);
dvec3 max(dvec3 _X, double _Y);
dvec4 max(dvec4 _X, double _Y);
// [Generic] genIType max(genIType x, genIType y);
int max(int _X, int _Y);
ivec2 max(ivec2 _X, ivec2 _Y);
ivec3 max(ivec3 _X, ivec3 _Y);
ivec4 max(ivec4 _X, ivec4 _Y);
// [Generic] genIType max(genIType x, int y);
ivec2 max(ivec2 _X, int _Y);
ivec3 max(ivec3 _X, int _Y);
ivec4 max(ivec4 _X, int _Y);
// [Generic] genUType max(genUType x, genUType y);
uint max(uint _X, uint _Y);
uvec2 max(uvec2 _X, uvec2 _Y);
uvec3 max(uvec3 _X, uvec3 _Y);
uvec4 max(uvec4 _X, uvec4 _Y);
// [Generic] genUType max(genUType x, uint y);
uvec2 max(uvec2 _X, uint _Y);
uvec3 max(uvec3 _X, uint _Y);
uvec4 max(uvec4 _X, uint _Y);

// Clamp
// Returns min(max(x, minVal), maxVal).
// Results are undefined if minVal > maxVal.
// [Generic] genFType clamp(genFType x, genFType minVal, genFType maxVal);
float clamp(float _X, float _MinVal, float _MaxVal);
vec2 clamp(vec2 _X, vec2 _MinVal, vec2 _MaxVal);
vec3 clamp(vec3 _X, vec3 _MinVal, vec3 _MaxVal);
vec4 clamp(vec4 _X, vec4 _MinVal, vec4 _MaxVal);
// [Generic] genFType clamp(genFType x, float minVal, float maxVal);
vec2 clamp(vec2 _X, float _MinVal, float _MaxVal);
vec3 clamp(vec3 _X, float _MinVal, float _MaxVal);
vec4 clamp(vec4 _X, float _MinVal, float _MaxVal);
// [Generic] genDType clamp(genDType x, genDType minVal, genDType maxVal);
double clamp(double _X, double _MinVal, double _MaxVal);
dvec2 clamp(dvec2 _X, dvec2 _MinVal, dvec2 _MaxVal);
dvec3 clamp(dvec3 _X, dvec3 _MinVal, dvec3 _MaxVal);
dvec4 clamp(dvec4 _X, dvec4 _MinVal, dvec4 _MaxVal);
// [Generic] genDType clamp(genDType x, double minVal, double maxVal);
dvec2 clamp(dvec2 _X, double _MinVal, double _MaxVal);
dvec3 clamp(dvec3 _X, double _MinVal, double _MaxVal);
dvec4 clamp(dvec4 _X, double _MinVal, double _MaxVal);
// [Generic] genIType clamp(genIType x, genIType minVal, genIType maxVal);
int clamp(int _X, int _MinVal, int _MaxVal);
ivec2 clamp(ivec2 _X, ivec2 _MinVal, ivec2 _MaxVal);
ivec3 clamp(ivec3 _X, ivec3 _MinVal, ivec3 _MaxVal);
ivec4 clamp(ivec4 _X, ivec4 _MinVal, ivec4 _MaxVal);
// [Generic] genIType clamp(genIType x, int minVal, int maxVal);
ivec2 clamp(ivec2 _X, int _MinVal, int _MaxVal);
ivec3 clamp(ivec3 _X, int _MinVal, int _MaxVal);
ivec4 clamp(ivec4 _X, int _MinVal, int _MaxVal);
// [Generic] genUType clamp(genUType x, genUType minVal, genUType maxVal);
uint clamp(uint _X, uint _MinVal, uint _MaxVal);
uvec2 clamp(uvec2 _X, uvec2 _MinVal, uvec2 _MaxVal);
uvec3 clamp(uvec3 _X, uvec3 _MinVal, uvec3 _MaxVal);
uvec4 clamp(uvec4 _X, uvec4 _MinVal, uvec4 _MaxVal);
// [Generic] genUType clamp(genUType x, uint minVal, uint maxVal);
uvec2 clamp(uvec2 _X, uint _MinVal, uint _MaxVal);
uvec3 clamp(uvec3 _X, uint _MinVal, uint _MaxVal);
uvec4 clamp(uvec4 _X, uint _MinVal, uint _MaxVal);

// Mix
// Returns the linear blend of x and y, i.e., x  (1 - a) + y  a.
// [Generic] genFType mix(genFType x, genFType y, genFType a);
float mix(float _X, float _Y, float _A);
vec2 mix(vec2 _X, vec2 _Y, vec2 _A);
vec3 mix(vec3 _X, vec3 _Y, vec3 _A);
vec4 mix(vec4 _X, vec4 _Y, vec4 _A);
// [Generic] genFType mix(genFType x, genFType y, float a);
vec2 mix(vec2 _X, vec2 _Y, float _A);
vec3 mix(vec3 _X, vec3 _Y, float _A);
vec4 mix(vec4 _X, vec4 _Y, float _A);
// [Generic] genDType mix(genDType x, genDType y, genDType a);
double mix(double _X, double _Y, double _A);
dvec2 mix(dvec2 _X, dvec2 _Y, dvec2 _A);
dvec3 mix(dvec3 _X, dvec3 _Y, dvec3 _A);
dvec4 mix(dvec4 _X, dvec4 _Y, dvec4 _A);
// [Generic] genDType mix(genDType x, genDType y, double a);
dvec2 mix(dvec2 _X, dvec2 _Y, double _A);
dvec3 mix(dvec3 _X, dvec3 _Y, double _A);
dvec4 mix(dvec4 _X, dvec4 _Y, double _A);

// Step
// Returns 0.0 if x < edge; otherwise it returns 1.0.
// [Generic] genFType step(genFType edge, genFType x);
float step(float _Edge, float _X);
vec2 step(vec2 _Edge, vec2 _X);
vec3 step(vec3 _Edge, vec3 _X);
vec4 step(vec4 _Edge, vec4 _X);
// [Generic] genFType step(float edge, genFType x);
vec2 step(float _Edge, vec2 _X);
vec3 step(float _Edge, vec3 _X);
vec4 step(float _Edge, vec4 _X);
// [Generic] genDType step(genDType edge, genDType x);
double step(double _Edge, double _X);
dvec2 step(dvec2 _Edge, dvec2 _X);
dvec3 step(dvec3 _Edge, dvec3 _X);
dvec4 step(dvec4 _Edge, dvec4 _X);
// [Generic] genDType step(double edge, genDType x);
dvec2 step(double _Edge, dvec2 _X);
dvec3 step(double _Edge, dvec3 _X);
dvec4 step(double _Edge, dvec4 _X);

// Smoothstep
// Returns 0.0 if x <= edge0 and 1.0 if x >= edge1, and
// performs smooth Hermite interpolation between 0 and 1 when edge0 < x < edge1.
// This is useful in cases where you would want a threshold function with a smooth transition.
// This is equivalent to:
// genFType t;
// t = clamp((x - edge0) / (edge1 - edge0), 0, 1);
// return t * t * (3 - 2 * t);
// [Generic] genFType smoothstep(genFType edge0, genFType edge1, genFType x);
float smoothstep(float _Edge0, float _Edge1, float _X);
vec2 smoothstep(vec2 _Edge0, vec2 _Edge1, vec2 _X);
vec3 smoothstep(vec3 _Edge0, vec3 _Edge1, vec3 _X);
vec4 smoothstep(vec4 _Edge0, vec4 _Edge1, vec4 _X);
// [Generic] genFType smoothstep(float edge0, float edge1, genFType x);
vec2 smoothstep(float _Edge0, float _Edge1, vec2 _X);
vec3 smoothstep(float _Edge0, float _Edge1, vec3 _X);
vec4 smoothstep(float _Edge0, float _Edge1, vec4 _X);
// [Generic] genDType smoothstep(genDType edge0, genDType edge1, genDType x);
double smoothstep(double _Edge0, double _Edge1, double _X);
dvec2 smoothstep(dvec2 _Edge0, dvec2 _Edge1, dvec2 _X);
dvec3 smoothstep(dvec3 _Edge0, dvec3 _Edge1, dvec3 _X);
dvec4 smoothstep(dvec4 _Edge0, dvec4 _Edge1, dvec4 _X);
// [Generic] genDType smoothstep(double edge0, double edge1, genDType x);
dvec2 smoothstep(double _Edge0, double _Edge1, dvec2 _X);
dvec3 smoothstep(double _Edge0, double _Edge1, dvec3 _X);
dvec4 smoothstep(double _Edge0, double _Edge1, dvec4 _X);

// Is NaN
// Returns true if x holds a NaN. Returns false otherwise.
// Always returns false if NaNs are not implemented.
// [Generic] genBType isnan(genFType x);
bool isnan(float _X);
bvec2 isnan(vec2 _X);
bvec3 isnan(vec3 _X);
bvec4 isnan(vec4 _X);
// [Generic] genBType isnan(genDType x);
bool isnan(double _X);
bvec2 isnan(dvec2 _X);
bvec3 isnan(dvec3 _X);
bvec4 isnan(dvec4 _X);

// Is Inf
// Returns true if x holds a positive infinity or negative infinity. Returns false otherwise.
// [Generic] genBType isinf(genFType x);
bool isinf(float _X);
bvec2 isinf(vec2 _X);
bvec3 isinf(vec3 _X);
bvec4 isinf(vec4 _X);
// [Generic] genBType isinf(genDType x);
bool isinf(double _X);
bvec2 isinf(dvec2 _X);
bvec3 isinf(dvec3 _X);
bvec4 isinf(dvec4 _X);

// Float Bits to Int
// Returns a signed or unsigned integer value representing the encoding of a floating-point value.
// The float value's bit-level representation is preserved.
// [Generic] genIType floatBitsToInt(highp genFType value);
int floatBitsToInt(highp float _Value);
ivec2 floatBitsToInt(highp vec2 _Value);
ivec3 floatBitsToInt(highp vec3 _Value);
ivec4 floatBitsToInt(highp vec4 _Value);
// [Generic] genUType floatBitsToUint(highp genFType value);
uint floatBitsToUint(highp float _Value);
uvec2 floatBitsToUint(highp vec2 _Value);
uvec3 floatBitsToUint(highp vec3 _Value);
uvec4 floatBitsToUint(highp vec4 _Value);

// Int Bits to Float
// Returns a floating-point value corresponding to a signed or unsigned integer encoding of a
// floating-point value.
// If a NaN is passed in, it will not signal, and the resulting value is unspecified.
// If an Inf is passed in, the resulting value is the corresponding Inf.
// If an Inf or NaN is passed in, it will not signal, and the resulting floating-point value is
// unspecified.
// If a subnormal number is passed in, the result might be flushed to 0.
// Otherwise, the bit-level representation is preserved.
// [Generic] genFType intBitsToFloat(highp genIType value);
float intBitsToFloat(highp int _Value);
vec2 intBitsToFloat(highp ivec2 _Value);
vec3 intBitsToFloat(highp ivec3 _Value);
vec4 intBitsToFloat(highp ivec4 _Value);
// [Generic] genFType uintBitsToFloat(highp genUType value);
float uintBitsToFloat(highp uint _Value);
vec2 uintBitsToFloat(highp uvec2 _Value);
vec3 uintBitsToFloat(highp uvec3 _Value);
vec4 uintBitsToFloat(highp uvec4 _Value);

// Fma
// Computes and returns a * b + c.
// In uses where the return value is eventually consumed by a variable declared as precise:
// --
// fma() is considered a single operation, whereas the expression a * b + c
// consumed by a variable declared precise is considered two
// operations.
// The precision of fma*() can differ from the precision of the expression
// a * b + c.
// fma() will be computed with the same precision as any other fma*()
// consumed by a precise variable, giving invariant results for the same
// input values of a, b, and c.
// Otherwise, in the absence of precise consumption, there are no special
// constraints on the number of operations or difference in precision between
// fma() and the expression a * b + c.
// --
// [Generic] genFType fma(genFType a, genFType b, genFType c);
float fma(float _A, float _B, float _C);
vec2 fma(vec2 _A, vec2 _B, vec2 _C);
vec3 fma(vec3 _A, vec3 _B, vec3 _C);
vec4 fma(vec4 _A, vec4 _B, vec4 _C);
// [Generic] genDType fma(genDType a, genDType b, genDType c);
double fma(double _A, double _B, double _C);
dvec2 fma(dvec2 _A, dvec2 _B, dvec2 _C);
dvec3 fma(dvec3 _A, dvec3 _B, dvec3 _C);
dvec4 fma(dvec4 _A, dvec4 _B, dvec4 _C);

// Frexp
// Splits x into a floating-point significand in the range
// [Generic] genFType frexp(highp genFType x, out highp genIType exp);
float frexp(highp float _X, out highp int _Exp);
vec2 frexp(highp vec2 _X, out highp ivec2 _Exp);
vec3 frexp(highp vec3 _X, out highp ivec3 _Exp);
vec4 frexp(highp vec4 _X, out highp ivec4 _Exp);
// [Generic] genDType frexp(genDType x, out genIType exp);
double frexp(double _X, out int _Exp);
dvec2 frexp(dvec2 _X, out ivec2 _Exp);
dvec3 frexp(dvec3 _X, out ivec3 _Exp);
dvec4 frexp(dvec4 _X, out ivec4 _Exp);

// Ldexp
// Builds a floating-point number from x and the corresponding integral
// exponent of two in exp, returning:
// [Generic] genFType ldexp(highp genFType x, highp genIType exp);
float ldexp(highp float _X, highp int _Exp);
vec2 ldexp(highp vec2 _X, highp ivec2 _Exp);
vec3 ldexp(highp vec3 _X, highp ivec3 _Exp);
vec4 ldexp(highp vec4 _X, highp ivec4 _Exp);
// [Generic] genDType ldexp(genDType x, genIType exp);
double ldexp(double _X, int _Exp);
dvec2 ldexp(dvec2 _X, ivec2 _Exp);
dvec3 ldexp(dvec3 _X, ivec3 _Exp);
dvec4 ldexp(dvec4 _X, ivec4 _Exp);

// Pack Unorm
// First, converts each component of the normalized floating-point value
// v into 16-bit (2x16) or 8-bit (4x8) integer values.
// Then, the results are packed into the returned 32-bit unsigned
// integer.
// The conversion for component c of v to fixed point is done as
// follows:
// packUnorm2x16: round(clamp(c, 0, +1) * 65535.0)
// packSnorm2x16: round(clamp(c, -1, +1) * 32767.0)
// packUnorm4x8: round(clamp(c, 0, +1) * 255.0)
// packSnorm4x8: round(clamp(c, -1, +1) * 127.0)
// The first component of the vector will be written to the least
// significant bits of the output; the last component will be written to
// the most significant bits.
// [Generic] highp uint packUnorm2x16(vec2 v);
highp uint packUnorm2x16(vec2 _V);
// [Generic] highp uint packSnorm2x16(vec2 v);
highp uint packSnorm2x16(vec2 _V);
// [Generic] uint packUnorm4x8(vec4 v);
uint packUnorm4x8(vec4 _V);
// [Generic] uint packSnorm4x8(vec4 v);
uint packSnorm4x8(vec4 _V);

// Unpack Unorm
// First, unpacks a single 32-bit unsigned integer p into a pair of
// 16-bit unsigned integers, a pair of 16-bit signed integers, four 8-bit
// unsigned integers, or four 8-bit signed integers, respectively.
// Then, each component is converted to a normalized floating-point value
// to generate the returned two- or four-component vector.
// The conversion for unpacked fixed-point value f to floating-point is
// done as follows:
// unpackUnorm2x16: f / 65535.0
// unpackSnorm2x16: clamp(f / 32767.0, -1, +1)
// unpackUnorm4x8: f / 255.0
// unpackSnorm4x8: clamp(f / 127.0, -1, +1)
// The first component of the returned vector will be extracted from the
// least significant bits of the input; the last component will be
// extracted from the most significant bits.
// [Generic] vec2 unpackUnorm2x16(highp uint p);
vec2 unpackUnorm2x16(highp uint _P);
// [Generic] vec2 unpackSnorm2x16(highp uint p);
vec2 unpackSnorm2x16(highp uint _P);
// [Generic] vec4 unpackUnorm4x8(highp uint p);
vec4 unpackUnorm4x8(highp uint _P);
// [Generic] vec4 unpackSnorm4x8(highp uint p);
vec4 unpackSnorm4x8(highp uint _P);

// Pack Half 2x16
// Returns an unsigned integer obtained by converting the components of a
// two-component floating-point vector to the 16-bit floating-point
// representation of the <<references, API>>, and
// then packing these two 16-bit integers into a 32-bit unsigned integer.
// The first vector component specifies the 16 least-significant bits of
// the result; the second component specifies the 16 most-significant
// bits.
// [Generic] uint packHalf2x16(vec2 v);
uint packHalf2x16(vec2 _V);
// [Generic] highp uint packHalf2x16(mediump vec2 v);
highp uint packHalf2x16(mediump vec2 _V);

// Unpack Half 2x16
// Returns a two-component floating-point vector with components obtained
// by unpacking a 32-bit unsigned integer into a pair of 16-bit values,
// interpreting those values as 16-bit floating-point numbers according
// to the <<references, API>>, and converting them to
// 32-bit floating-point values.
// The first component of the vector is obtained from the 16
// least-significant bits of v; the second component is obtained from
// the 16 most-significant bits of v.
// [Generic] vec2 unpackHalf2x16(uint v);
vec2 unpackHalf2x16(uint _V);
// [Generic] mediump vec2 unpackHalf2x16(highp uint v);
mediump vec2 unpackHalf2x16(highp uint _V);

// Pack Double 2x32
// Returns a double-precision value obtained by packing the components of
// v into a 64-bit value.
// If an IEEE 754 Inf or NaN is created, it will not signal, and the
// resulting floating-point value is unspecified.
// Otherwise, the bit-level representation of v is preserved.
// The first vector component specifies the 32 least significant bits;
// the second component specifies the 32 most significant bits.
// [Generic] double packDouble2x32(uvec2 v);
double packDouble2x32(uvec2 _V);

// Unpack Double 2x32
// Returns a two-component unsigned integer vector representation of v.
// The bit-level representation of v is preserved.
// The first component of the vector contains the 32 least significant
// bits of the double; the second component consists of the 32 most
// significant bits.
// [Generic] uvec2 unpackDouble2x32(double v);
uvec2 unpackDouble2x32(double _V);

// Length
// Returns the length of vector x, i.e., sqrt( x~0~^2^ + x~1~^2^ + ... ).
// [Generic] float length(genFType x);
float length(float _X);
float length(vec2 _X);
float length(vec3 _X);
float length(vec4 _X);
// [Generic] double length(genDType x);
double length(double _X);
double length(dvec2 _X);
double length(dvec3 _X);
double length(dvec4 _X);

// Distance
// Returns the distance between p0 and p1, i.e., length(p0 - p1)
// [Generic] float distance(genFType p0, genFType p1);
float distance(float _P0, float _P1);
float distance(vec2 _P0, vec2 _P1);
float distance(vec3 _P0, vec3 _P1);
float distance(vec4 _P0, vec4 _P1);
// [Generic] double distance(genDType p0, genDType p1);
double distance(double _P0, double _P1);
double distance(dvec2 _P0, dvec2 _P1);
double distance(dvec3 _P0, dvec3 _P1);
double distance(dvec4 _P0, dvec4 _P1);

// Dot
// Returns the dot product of x and y, i.e., x~0~  y~0~ + x~1~  y~1~ + ...
// [Generic] float dot(genFType x, genFType y);
float dot(float _X, float _Y);
float dot(vec2 _X, vec2 _Y);
float dot(vec3 _X, vec3 _Y);
float dot(vec4 _X, vec4 _Y);
// [Generic] double dot(genDType x, genDType y);
double dot(double _X, double _Y);
double dot(dvec2 _X, dvec2 _Y);
double dot(dvec3 _X, dvec3 _Y);
double dot(dvec4 _X, dvec4 _Y);

// Cross
// Returns the cross product of x and y, i.e.,
// (x~1~  y~2~ - y~1~  x~2~,
//  x~2~  y~0~ - y~2~  x~0~,
//  x~0~  y~1~ - y~0~  x~1~).
// [Generic] vec3 cross(vec3 x, vec3 y);
vec3 cross(vec3 _X, vec3 _Y);
// [Generic] dvec3 cross(dvec3 x, dvec3 y);
dvec3 cross(dvec3 _X, dvec3 _Y);

// Normalize
// Returns a vector in the same direction as x but with a length of 1, i.e. x / length(x).
// [Generic] genFType normalize(genFType x);
float normalize(float _X);
vec2 normalize(vec2 _X);
vec3 normalize(vec3 _X);
vec4 normalize(vec4 _X);
// [Generic] genDType normalize(genDType x);
double normalize(double _X);
dvec2 normalize(dvec2 _X);
dvec3 normalize(dvec3 _X);
dvec4 normalize(dvec4 _X);

// FTransform
// Compatibility profile only

// Available only when using the compatibility profile. For core {apiname}, use invariant.
// For vertex shaders only. This function will ensure that the incoming vertex value will be
// transformed in a way that produces exactly the same result as would be produced by {apiname}'s
// fixed functionality transform. It is intended to be used to compute glPosition_, e.g.
vec4 ftransform();

// Face Forward
// If dot(Nref, I) < 0 return N, otherwise return -N.
// [Generic] genFType faceforward(genFType N, genFType I, genFType Nref);
float faceforward(float _N, float _I, float _Nref);
vec2 faceforward(vec2 _N, vec2 _I, vec2 _Nref);
vec3 faceforward(vec3 _N, vec3 _I, vec3 _Nref);
vec4 faceforward(vec4 _N, vec4 _I, vec4 _Nref);
// [Generic] genDType faceforward(genDType N, genDType I, genDType Nref);
double faceforward(double _N, double _I, double _Nref);
dvec2 faceforward(dvec2 _N, dvec2 _I, dvec2 _Nref);
dvec3 faceforward(dvec3 _N, dvec3 _I, dvec3 _Nref);
dvec4 faceforward(dvec4 _N, dvec4 _I, dvec4 _Nref);

// Reflect
// For the incident vector I and surface orientation N, returns the reflection direction:
// [Generic] genFType reflect(genFType I, genFType N);
float reflect(float _I, float _N);
vec2 reflect(vec2 _I, vec2 _N);
vec3 reflect(vec3 _I, vec3 _N);
vec4 reflect(vec4 _I, vec4 _N);
// [Generic] genDType reflect(genDType I, genDType N);
double reflect(double _I, double _N);
dvec2 reflect(dvec2 _I, dvec2 _N);
dvec3 reflect(dvec3 _I, dvec3 _N);
dvec4 reflect(dvec4 _I, dvec4 _N);

// Refract
// For the incident vector I and surface normal N, and the ratio of indices of refraction
// eta, return the refraction vector. The result is computed by the refraction equation shown below.
// [Generic] genFType refract(genFType I, genFType N, float eta);
float refract(float _I, float _N, float _Eta);
vec2 refract(vec2 _I, vec2 _N, float _Eta);
vec3 refract(vec3 _I, vec3 _N, float _Eta);
vec4 refract(vec4 _I, vec4 _N, float _Eta);
// [Generic] genDType refract(genDType I, genDType N, double eta);
double refract(double _I, double _N, double _Eta);
dvec2 refract(dvec2 _I, dvec2 _N, double _Eta);
dvec3 refract(dvec3 _I, dvec3 _N, double _Eta);
dvec4 refract(dvec4 _I, dvec4 _N, double _Eta);

// MatrixCompMult
// Multiply matrix x by matrix y component-wise, i.e., result[i][j]
// is the scalar product of x[i][j] and y[i][j].
// Note: To get linear algebraic matrix multiplication, use the multiply operator (*).
// [Generic] mat matrixCompMult(mat x, mat y);
mat2 matrixCompMult(mat2 _X, mat2 _Y);
mat3 matrixCompMult(mat3 _X, mat3 _Y);
mat4 matrixCompMult(mat4 _X, mat4 _Y);
mat2x2 matrixCompMult(mat2x2 _X, mat2x2 _Y);
mat2x3 matrixCompMult(mat2x3 _X, mat2x3 _Y);
mat2x4 matrixCompMult(mat2x4 _X, mat2x4 _Y);
mat3x2 matrixCompMult(mat3x2 _X, mat3x2 _Y);
mat3x3 matrixCompMult(mat3x3 _X, mat3x3 _Y);
mat3x4 matrixCompMult(mat3x4 _X, mat3x4 _Y);
mat4x2 matrixCompMult(mat4x2 _X, mat4x2 _Y);
mat4x3 matrixCompMult(mat4x3 _X, mat4x3 _Y);
mat4x4 matrixCompMult(mat4x4 _X, mat4x4 _Y);

// OuterProduct
// Treats the first parameter c as a column vector (matrix with one
// column) and the second parameter r as a row vector (matrix with one
// row) and does a linear algebraic matrix multiply c * r, yielding a
// matrix whose number of rows is the number of components in c and
// whose number of columns is the number of components in r.
// [Generic] mat2 outerProduct(vec2 c, vec2 r);
mat2 outerProduct(vec2 _C, vec2 _R);
// [Generic] mat3 outerProduct(vec3 c, vec3 r);
mat3 outerProduct(vec3 _C, vec3 _R);
// [Generic] mat4 outerProduct(vec4 c, vec4 r);
mat4 outerProduct(vec4 _C, vec4 _R);
// [Generic] mat2x3 outerProduct(vec3 c, vec2 r);
mat2x3 outerProduct(vec3 _C, vec2 _R);
// [Generic] mat3x2 outerProduct(vec2 c, vec3 r);
mat3x2 outerProduct(vec2 _C, vec3 _R);
// [Generic] mat2x4 outerProduct(vec4 c, vec2 r);
mat2x4 outerProduct(vec4 _C, vec2 _R);
// [Generic] mat4x2 outerProduct(vec2 c, vec4 r);
mat4x2 outerProduct(vec2 _C, vec4 _R);
// [Generic] mat3x4 outerProduct(vec4 c, vec3 r);
mat3x4 outerProduct(vec4 _C, vec3 _R);
// [Generic] mat4x3 outerProduct(vec3 c, vec4 r);
mat4x3 outerProduct(vec3 _C, vec4 _R);

// Transpose
// Returns a matrix that is the transpose of m.
// The input matrix m is not modified.
// [Generic] mat2 transpose(mat2 m);
mat2 transpose(mat2 _M);
// [Generic] mat3 transpose(mat3 m);
mat3 transpose(mat3 _M);
// [Generic] mat4 transpose(mat4 m);
mat4 transpose(mat4 _M);
// [Generic] mat2x3 transpose(mat3x2 m);
mat2x3 transpose(mat3x2 _M);
// [Generic] mat3x2 transpose(mat2x3 m);
mat3x2 transpose(mat2x3 _M);
// [Generic] mat2x4 transpose(mat4x2 m);
mat2x4 transpose(mat4x2 _M);
// [Generic] mat4x2 transpose(mat2x4 m);
mat4x2 transpose(mat2x4 _M);
// [Generic] mat3x4 transpose(mat4x3 m);
mat3x4 transpose(mat4x3 _M);
// [Generic] mat4x3 transpose(mat3x4 m);
mat4x3 transpose(mat3x4 _M);

// Determinant
// Returns the determinant of m.
// [Generic] float determinant(mat2 m);
float determinant(mat2 _M);
// [Generic] float determinant(mat3 m);
float determinant(mat3 _M);
// [Generic] float determinant(mat4 m);
float determinant(mat4 _M);

// Inverse
// Returns a matrix that is the inverse of m.
// The input matrix m is not modified.
// The values in the returned matrix are undefined if m is singular or
// poorly-conditioned (nearly singular).
// [Generic] mat2 inverse(mat2 m);
mat2 inverse(mat2 _M);
// [Generic] mat3 inverse(mat3 m);
mat3 inverse(mat3 _M);
// [Generic] mat4 inverse(mat4 m);
mat4 inverse(mat4 _M);

// LessThan
// Returns the component-wise compare of x < y.
// [Generic] bvec lessThan(vec x, vec y);
bvec2 lessThan(vec2 _X, vec2 _Y);
bvec3 lessThan(vec3 _X, vec3 _Y);
bvec4 lessThan(vec4 _X, vec4 _Y);
bvec2 lessThan(dvec2 _X, dvec2 _Y);
bvec3 lessThan(dvec3 _X, dvec3 _Y);
bvec4 lessThan(dvec4 _X, dvec4 _Y);
// [Generic] bvec lessThan(ivec x, ivec y);
bvec2 lessThan(ivec2 _X, ivec2 _Y);
bvec3 lessThan(ivec3 _X, ivec3 _Y);
bvec4 lessThan(ivec4 _X, ivec4 _Y);
// [Generic] bvec lessThan(uvec x, uvec y);
bvec2 lessThan(uvec2 _X, uvec2 _Y);
bvec3 lessThan(uvec3 _X, uvec3 _Y);
bvec4 lessThan(uvec4 _X, uvec4 _Y);

// LessThanEqual
// Returns the component-wise compare of x <= y.
// [Generic] bvec lessThanEqual(vec x, vec y);
bvec2 lessThanEqual(vec2 _X, vec2 _Y);
bvec3 lessThanEqual(vec3 _X, vec3 _Y);
bvec4 lessThanEqual(vec4 _X, vec4 _Y);
bvec2 lessThanEqual(dvec2 _X, dvec2 _Y);
bvec3 lessThanEqual(dvec3 _X, dvec3 _Y);
bvec4 lessThanEqual(dvec4 _X, dvec4 _Y);
// [Generic] bvec lessThanEqual(ivec x, ivec y);
bvec2 lessThanEqual(ivec2 _X, ivec2 _Y);
bvec3 lessThanEqual(ivec3 _X, ivec3 _Y);
bvec4 lessThanEqual(ivec4 _X, ivec4 _Y);
// [Generic] bvec lessThanEqual(uvec x, uvec y);
bvec2 lessThanEqual(uvec2 _X, uvec2 _Y);
bvec3 lessThanEqual(uvec3 _X, uvec3 _Y);
bvec4 lessThanEqual(uvec4 _X, uvec4 _Y);

// GreaterThan
// Returns the component-wise compare of x > y.
// [Generic] bvec greaterThan(vec x, vec y);
bvec2 greaterThan(vec2 _X, vec2 _Y);
bvec3 greaterThan(vec3 _X, vec3 _Y);
bvec4 greaterThan(vec4 _X, vec4 _Y);
bvec2 greaterThan(dvec2 _X, dvec2 _Y);
bvec3 greaterThan(dvec3 _X, dvec3 _Y);
bvec4 greaterThan(dvec4 _X, dvec4 _Y);
// [Generic] bvec greaterThan(ivec x, ivec y);
bvec2 greaterThan(ivec2 _X, ivec2 _Y);
bvec3 greaterThan(ivec3 _X, ivec3 _Y);
bvec4 greaterThan(ivec4 _X, ivec4 _Y);
// [Generic] bvec greaterThan(uvec x, uvec y);
bvec2 greaterThan(uvec2 _X, uvec2 _Y);
bvec3 greaterThan(uvec3 _X, uvec3 _Y);
bvec4 greaterThan(uvec4 _X, uvec4 _Y);

// GreaterThanEqual
// Returns the component-wise compare of x >= y.
// [Generic] bvec greaterThanEqual(vec x, vec y);
bvec2 greaterThanEqual(vec2 _X, vec2 _Y);
bvec3 greaterThanEqual(vec3 _X, vec3 _Y);
bvec4 greaterThanEqual(vec4 _X, vec4 _Y);
bvec2 greaterThanEqual(dvec2 _X, dvec2 _Y);
bvec3 greaterThanEqual(dvec3 _X, dvec3 _Y);
bvec4 greaterThanEqual(dvec4 _X, dvec4 _Y);
// [Generic] bvec greaterThanEqual(ivec x, ivec y);
bvec2 greaterThanEqual(ivec2 _X, ivec2 _Y);
bvec3 greaterThanEqual(ivec3 _X, ivec3 _Y);
bvec4 greaterThanEqual(ivec4 _X, ivec4 _Y);
// [Generic] bvec greaterThanEqual(uvec x, uvec y);
bvec2 greaterThanEqual(uvec2 _X, uvec2 _Y);
bvec3 greaterThanEqual(uvec3 _X, uvec3 _Y);
bvec4 greaterThanEqual(uvec4 _X, uvec4 _Y);

// Equal
// Returns the component-wise compare of x == y.
// [Generic] bvec equal(vec x, vec y);
bvec2 equal(vec2 _X, vec2 _Y);
bvec3 equal(vec3 _X, vec3 _Y);
bvec4 equal(vec4 _X, vec4 _Y);
bvec2 equal(dvec2 _X, dvec2 _Y);
bvec3 equal(dvec3 _X, dvec3 _Y);
bvec4 equal(dvec4 _X, dvec4 _Y);
// [Generic] bvec equal(ivec x, ivec y);
bvec2 equal(ivec2 _X, ivec2 _Y);
bvec3 equal(ivec3 _X, ivec3 _Y);
bvec4 equal(ivec4 _X, ivec4 _Y);
// [Generic] bvec equal(uvec x, uvec y);
bvec2 equal(uvec2 _X, uvec2 _Y);
bvec3 equal(uvec3 _X, uvec3 _Y);
bvec4 equal(uvec4 _X, uvec4 _Y);
// [Generic] bvec equal(bvec x, bvec y);
bvec2 equal(bvec2 _X, bvec2 _Y);
bvec3 equal(bvec3 _X, bvec3 _Y);
bvec4 equal(bvec4 _X, bvec4 _Y);

// NotEqual
// Returns the component-wise compare of x != y.
// [Generic] bvec notEqual(vec x, vec y);
bvec2 notEqual(vec2 _X, vec2 _Y);
bvec3 notEqual(vec3 _X, vec3 _Y);
bvec4 notEqual(vec4 _X, vec4 _Y);
bvec2 notEqual(dvec2 _X, dvec2 _Y);
bvec3 notEqual(dvec3 _X, dvec3 _Y);
bvec4 notEqual(dvec4 _X, dvec4 _Y);
// [Generic] bvec notEqual(ivec x, ivec y);
bvec2 notEqual(ivec2 _X, ivec2 _Y);
bvec3 notEqual(ivec3 _X, ivec3 _Y);
bvec4 notEqual(ivec4 _X, ivec4 _Y);
// [Generic] bvec notEqual(uvec x, uvec y);
bvec2 notEqual(uvec2 _X, uvec2 _Y);
bvec3 notEqual(uvec3 _X, uvec3 _Y);
bvec4 notEqual(uvec4 _X, uvec4 _Y);
// [Generic] bvec notEqual(bvec x, bvec y);
bvec2 notEqual(bvec2 _X, bvec2 _Y);
bvec3 notEqual(bvec3 _X, bvec3 _Y);
bvec4 notEqual(bvec4 _X, bvec4 _Y);

// Any
// Returns true if any component of x is true.
// [Generic] bool any(bvec x);
bool any(bvec2 _X);
bool any(bvec3 _X);
bool any(bvec4 _X);

// All
// Returns true only if all components of x are true.
// [Generic] bool all(bvec x);
bool all(bvec2 _X);
bool all(bvec3 _X);
bool all(bvec4 _X);

// Not
// Returns the component-wise logical complement of x.
// [Generic] bvec not(bvec x);
bvec2 not(bvec2 _X);
bvec3 not(bvec3 _X);
bvec4 not(bvec4 _X);

// UAddCarry
// Adds 32-bit unsigned integers x and y, returning the sum modulo 2^32^.
// The value carry is set to zero if the sum was less than 2^32^, or one otherwise.
// [Generic] genUType uaddCarry(highp genUType x, highp genUType y, out lowp genUType carry);
uint uaddCarry(highp uint _X, highp uint _Y, out lowp uint _Carry);
uvec2 uaddCarry(highp uvec2 _X, highp uvec2 _Y, out lowp uvec2 _Carry);
uvec3 uaddCarry(highp uvec3 _X, highp uvec3 _Y, out lowp uvec3 _Carry);
uvec4 uaddCarry(highp uvec4 _X, highp uvec4 _Y, out lowp uvec4 _Carry);

// USubBorrow
// Subtracts the 32-bit unsigned integer y from x, returning the difference if non-negative, or
// 2^32^ plus the difference otherwise.
// The value borrow is set to zero if x >= y, or one otherwise.
// [Generic] genUType usubBorrow(highp genUType x, highp genUType y, out lowp genUType borrow);
uint usubBorrow(highp uint _X, highp uint _Y, out lowp uint _Borrow);
uvec2 usubBorrow(highp uvec2 _X, highp uvec2 _Y, out lowp uvec2 _Borrow);
uvec3 usubBorrow(highp uvec3 _X, highp uvec3 _Y, out lowp uvec3 _Borrow);
uvec4 usubBorrow(highp uvec4 _X, highp uvec4 _Y, out lowp uvec4 _Borrow);

// Mul Extended
// Multiplies 32-bit unsigned or signed integers x and y, producing a 64-bit result.
// * The 32 least-significant bits are returned in lsb.
// * The 32 most-significant bits are returned in msb.
// [Generic] void umulExtended(highp genUType x, highp genUType y, out highp genUType msb, out highp genUType lsb);
void umulExtended(highp uint _X, highp uint _Y, out highp uint _Msb, out highp uint _Lsb);
void umulExtended(highp uvec2 _X, highp uvec2 _Y, out highp uvec2 _Msb, out highp uvec2 _Lsb);
void umulExtended(highp uvec3 _X, highp uvec3 _Y, out highp uvec3 _Msb, out highp uvec3 _Lsb);
void umulExtended(highp uvec4 _X, highp uvec4 _Y, out highp uvec4 _Msb, out highp uvec4 _Lsb);
// [Generic] void imulExtended(highp genIType x, highp genIType y, out highp genIType msb, out highp genIType lsb);
void imulExtended(highp int _X, highp int _Y, out highp int _Msb, out highp int _Lsb);
void imulExtended(highp ivec2 _X, highp ivec2 _Y, out highp ivec2 _Msb, out highp ivec2 _Lsb);
void imulExtended(highp ivec3 _X, highp ivec3 _Y, out highp ivec3 _Msb, out highp ivec3 _Lsb);
void imulExtended(highp ivec4 _X, highp ivec4 _Y, out highp ivec4 _Msb, out highp ivec4 _Lsb);

// BitfieldExtract
// Extracts bits [offset, offset + bits - 1] from value, returning them in the least
// significant bits of the result.
// For unsigned data types, the most significant bits of the result will be set to zero.
// For signed data types, the most significant bits will be set to the value of bit
// [Generic] genIType bitfieldExtract(genIType value, int offset, int bits);
int bitfieldExtract(int _Value, int _Offset, int _Bits);
ivec2 bitfieldExtract(ivec2 _Value, int _Offset, int _Bits);
ivec3 bitfieldExtract(ivec3 _Value, int _Offset, int _Bits);
ivec4 bitfieldExtract(ivec4 _Value, int _Offset, int _Bits);
// [Generic] genUType bitfieldExtract(genUType value, int offset, int bits);
uint bitfieldExtract(uint _Value, int _Offset, int _Bits);
uvec2 bitfieldExtract(uvec2 _Value, int _Offset, int _Bits);
uvec3 bitfieldExtract(uvec3 _Value, int _Offset, int _Bits);
uvec4 bitfieldExtract(uvec4 _Value, int _Offset, int _Bits);

// BitfieldInsert
// Inserts the bits least significant bits of insert into base.
// The result will have bits [offset, offset + bits - 1] taken from bits [0, bits - 1] of
// insert, and all other bits taken directly from the corresponding bits of base.
// If bits is zero, the result will simply be base. The result will be undefined if offset or
// bits is negative, or if the sum of offset and bits is greater than the number of bits used
// to store the operand.
// Note that for vector versions of bitfieldInsert(), a single pair of offset and bits values is
// shared for all components.
// [Generic] genIType bitfieldInsert(genIType base, genIType insert, int offset, int bits);
int bitfieldInsert(int _Base, int _Insert, int _Offset, int _Bits);
ivec2 bitfieldInsert(ivec2 _Base, ivec2 _Insert, int _Offset, int _Bits);
ivec3 bitfieldInsert(ivec3 _Base, ivec3 _Insert, int _Offset, int _Bits);
ivec4 bitfieldInsert(ivec4 _Base, ivec4 _Insert, int _Offset, int _Bits);
// [Generic] genUType bitfieldInsert(genUType base, genUType insert, int offset, int bits);
uint bitfieldInsert(uint _Base, uint _Insert, int _Offset, int _Bits);
uvec2 bitfieldInsert(uvec2 _Base, uvec2 _Insert, int _Offset, int _Bits);
uvec3 bitfieldInsert(uvec3 _Base, uvec3 _Insert, int _Offset, int _Bits);
uvec4 bitfieldInsert(uvec4 _Base, uvec4 _Insert, int _Offset, int _Bits);

// BitfieldReverse
// Reverses the bits of value.
// The bit numbered n of the result will be taken from bit (bits -1) - n of value, where
// bits is the total number of bits used to represent value.
// [Generic] genIType bitfieldReverse(highp genIType value);
int bitfieldReverse(highp int _Value);
ivec2 bitfieldReverse(highp ivec2 _Value);
ivec3 bitfieldReverse(highp ivec3 _Value);
ivec4 bitfieldReverse(highp ivec4 _Value);
// [Generic] genUType bitfieldReverse(highp genUType value);
uint bitfieldReverse(highp uint _Value);
uvec2 bitfieldReverse(highp uvec2 _Value);
uvec3 bitfieldReverse(highp uvec3 _Value);
uvec4 bitfieldReverse(highp uvec4 _Value);

// BitCount
// Returns the number of one bits in the binary representation of value.
// [Generic] genIType bitCount(genIType value);
int bitCount(int _Value);
ivec2 bitCount(ivec2 _Value);
ivec3 bitCount(ivec3 _Value);
ivec4 bitCount(ivec4 _Value);
// [Generic] genIType bitCount(genUType value);
int bitCount(uint _Value);
ivec2 bitCount(uvec2 _Value);
ivec3 bitCount(uvec3 _Value);
ivec4 bitCount(uvec4 _Value);

// FindLSB
// Returns the bit number of the least significant one bit in the binary representation of value.
// If value is zero, -1 will be returned.
// [Generic] genIType findLSB(genIType value);
int findLSB(int _Value);
ivec2 findLSB(ivec2 _Value);
ivec3 findLSB(ivec3 _Value);
ivec4 findLSB(ivec4 _Value);
// [Generic] genIType findLSB(genUType value);
int findLSB(uint _Value);
ivec2 findLSB(uvec2 _Value);
ivec3 findLSB(uvec3 _Value);
ivec4 findLSB(uvec4 _Value);

// FindMSB
// Returns the bit number of the most significant bit in the binary representation of value.
// For positive integers, the result will be the bit number of the most significant one bit.
// For negative integers, the result will be the bit number of the most significant zero bit.
// For a value of zero or negative one, -1 will be returned.
// [Generic] genIType findMSB(highp genIType value);
int findMSB(highp int _Value);
ivec2 findMSB(highp ivec2 _Value);
ivec3 findMSB(highp ivec3 _Value);
ivec4 findMSB(highp ivec4 _Value);
// [Generic] genIType findMSB(highp genUType value);
int findMSB(highp uint _Value);
ivec2 findMSB(highp uvec2 _Value);
ivec3 findMSB(highp uvec3 _Value);
ivec4 findMSB(highp uvec4 _Value);

// Texture Query Functions

// The *textureSize* functions query the dimensions of a specific texture level
// for a texture-combined sampler.

// The *textureQueryLod* functions are available only in a fragment shader.
// They take the components of _P_ and compute the level-of-detail information
// that the texture pipe would use to access that texture through a normal
// texture lookup.
// The level-of-detail latexmath:[\lambda^{'}] (equation 3.18 of the
// <<references,{apispec}>>) is obtained after any level-of-detail bias, but
// prior to clamping to [TEXTURE_MIN_LOD, TEXTURE_MAX_LOD].
// The mipmap array(s) that would be accessed are also computed.
// If a single level-of-detail would be accessed, the level-of-detail number
// relative to the base level is returned.
// If multiple levels-of-detail would be accessed, a floating-point number
// between the two levels is returned, with the fractional part equal to the
// fractional part of the computed and clamped level-of-detail.

// The algorithm used is given by the following pseudo-code:

// float ComputeAccessedLod(float computedLod)
// {
//     // Clamp the computed LOD according to the texture LOD clamps.
//     if (computedLod < TEXTURE_MIN_LOD) computedLod = TEXTURE_MIN_LOD;
//     if (computedLod > TEXTURE_MAX_LOD) computedLod = TEXTURE_MAX_LOD;
//
//     // Clamp the computed LOD to the range of accessible levels.
//     if (computedLod < 0.0)
//         computedLod = 0.0;
//     if (computedLod > (float)maxAccessibleLevel)
//         computedLod = (float)maxAccessibleLevel;
//
//     // Return a value according to the min filter.
//     if (TEXTURE_MIN_FILTER is LINEAR or NEAREST) {
//         return 0.0;
//     } else if (TEXTURE_MIN_FILTER is NEAREST_MIPMAP_NEAREST or LINEAR_MIPMAP_NEAREST) {
//         return ceil(computedLod + 0.5) - 1.0;
//     } else {
//         return computedLod;
//     }
// }

// The value maxAccessibleLevel is the level number of the smallest
// accessible level of the mipmap array (the value q in section 8.14.3
// "Mipmapping" of the <<references,{apispec}>>) minus the base level.

// TextureSize
// Returns the dimensions of level lod (if present) for the texture bound to sampler, as described
// in section 11.1.3.4 "Texture Queries" of the <<references,{apispec}>>.
// The components in the return value are filled in, in order, with the width, height, and depth of
// the texture.
// For the array forms, the last component of the return value is the number of layers in the texture
// array, or the number of cubes in the texture cube map array.
// [Generic] int textureSize(gsampler1D sampler, int lod);
int textureSize(sampler1D _Sampler, int _Lod);
int textureSize(isampler1D _Sampler, int _Lod);
int textureSize(usampler1D _Sampler, int _Lod);
// [Generic] ivec2 textureSize(gsampler2D sampler, int lod);
ivec2 textureSize(sampler2D _Sampler, int _Lod);
ivec2 textureSize(isampler2D _Sampler, int _Lod);
ivec2 textureSize(usampler2D _Sampler, int _Lod);
// [Generic] ivec3 textureSize(gsampler3D sampler, int lod);
ivec3 textureSize(sampler3D _Sampler, int _Lod);
ivec3 textureSize(isampler3D _Sampler, int _Lod);
ivec3 textureSize(usampler3D _Sampler, int _Lod);
// [Generic] ivec2 textureSize(gsamplerCube sampler, int lod);
ivec2 textureSize(samplerCube _Sampler, int _Lod);
ivec2 textureSize(isamplerCube _Sampler, int _Lod);
ivec2 textureSize(usamplerCube _Sampler, int _Lod);
// [Generic] int textureSize(sampler1DShadow sampler, int lod);
int textureSize(sampler1DShadow _Sampler, int _Lod);
// [Generic] ivec2 textureSize(sampler2DShadow sampler, int lod);
ivec2 textureSize(sampler2DShadow _Sampler, int _Lod);
// [Generic] ivec2 textureSize(samplerCubeShadow sampler, int lod);
ivec2 textureSize(samplerCubeShadow _Sampler, int _Lod);
// [Generic] ivec3 textureSize(gsamplerCubeArray sampler, int lod);
ivec3 textureSize(samplerCubeArray _Sampler, int _Lod);
ivec3 textureSize(isamplerCubeArray _Sampler, int _Lod);
ivec3 textureSize(usamplerCubeArray _Sampler, int _Lod);
// [Generic] ivec3 textureSize(samplerCubeArrayShadow sampler, int lod);
ivec3 textureSize(samplerCubeArrayShadow _Sampler, int _Lod);
// [Generic] ivec2 textureSize(gsampler1DArray sampler, int lod);
ivec2 textureSize(sampler1DArray _Sampler, int _Lod);
ivec2 textureSize(isampler1DArray _Sampler, int _Lod);
ivec2 textureSize(usampler1DArray _Sampler, int _Lod);
// [Generic] ivec2 textureSize(sampler1DArrayShadow sampler, int lod);
ivec2 textureSize(sampler1DArrayShadow _Sampler, int _Lod);
// [Generic] ivec3 textureSize(gsampler2DArray sampler, int lod);
ivec3 textureSize(sampler2DArray _Sampler, int _Lod);
ivec3 textureSize(isampler2DArray _Sampler, int _Lod);
ivec3 textureSize(usampler2DArray _Sampler, int _Lod);
// [Generic] ivec3 textureSize(sampler2DArrayShadow sampler, int lod);
ivec3 textureSize(sampler2DArrayShadow _Sampler, int _Lod);
// [Generic] ivec2 textureSize(gsampler2DRect sampler);
ivec2 textureSize(sampler2DRect _Sampler);
ivec2 textureSize(isampler2DRect _Sampler);
ivec2 textureSize(usampler2DRect _Sampler);
// [Generic] ivec2 textureSize(sampler2DRectShadow sampler);
ivec2 textureSize(sampler2DRectShadow _Sampler);
// [Generic] int textureSize(gsamplerBuffer sampler);
int textureSize(samplerBuffer _Sampler);
int textureSize(isamplerBuffer _Sampler);
int textureSize(usamplerBuffer _Sampler);
// [Generic] ivec2 textureSize(gsampler2DMS sampler);
ivec2 textureSize(sampler2DMS _Sampler);
ivec2 textureSize(isampler2DMS _Sampler);
ivec2 textureSize(usampler2DMS _Sampler);
// [Generic] ivec3 textureSize(gsampler2DMSArray sampler);
ivec3 textureSize(sampler2DMSArray _Sampler);
ivec3 textureSize(isampler2DMSArray _Sampler);
ivec3 textureSize(usampler2DMSArray _Sampler);

// TextureQueryLod
// Returns the mipmap array(s) that would be accessed in the x component of the return value.
// Returns the computed level-of-detail relative to the base level in the y component of the return
// value.
// If called on an incomplete texture, the results are undefined.
// [Generic] vec2 textureQueryLod(gsampler1D sampler, float P);
vec2 textureQueryLod(sampler1D _Sampler, float _P);
vec2 textureQueryLod(isampler1D _Sampler, float _P);
vec2 textureQueryLod(usampler1D _Sampler, float _P);
// [Generic] vec2 textureQueryLod(gsampler2D sampler, vec2 P);
vec2 textureQueryLod(sampler2D _Sampler, vec2 _P);
vec2 textureQueryLod(isampler2D _Sampler, vec2 _P);
vec2 textureQueryLod(usampler2D _Sampler, vec2 _P);
// [Generic] vec2 textureQueryLod(gsampler3D sampler, vec3 P);
vec2 textureQueryLod(sampler3D _Sampler, vec3 _P);
vec2 textureQueryLod(isampler3D _Sampler, vec3 _P);
vec2 textureQueryLod(usampler3D _Sampler, vec3 _P);
// [Generic] vec2 textureQueryLod(gsamplerCube sampler, vec3 P);
vec2 textureQueryLod(samplerCube _Sampler, vec3 _P);
vec2 textureQueryLod(isamplerCube _Sampler, vec3 _P);
vec2 textureQueryLod(usamplerCube _Sampler, vec3 _P);
// [Generic] vec2 textureQueryLod(gsampler1DArray sampler, float P);
vec2 textureQueryLod(sampler1DArray _Sampler, float _P);
vec2 textureQueryLod(isampler1DArray _Sampler, float _P);
vec2 textureQueryLod(usampler1DArray _Sampler, float _P);
// [Generic] vec2 textureQueryLod(gsampler2DArray sampler, vec2 P);
vec2 textureQueryLod(sampler2DArray _Sampler, vec2 _P);
vec2 textureQueryLod(isampler2DArray _Sampler, vec2 _P);
vec2 textureQueryLod(usampler2DArray _Sampler, vec2 _P);
// [Generic] vec2 textureQueryLod(gsamplerCubeArray sampler, vec3 P);
vec2 textureQueryLod(samplerCubeArray _Sampler, vec3 _P);
vec2 textureQueryLod(isamplerCubeArray _Sampler, vec3 _P);
vec2 textureQueryLod(usamplerCubeArray _Sampler, vec3 _P);
// [Generic] vec2 textureQueryLod(sampler1DShadow sampler, float P);
vec2 textureQueryLod(sampler1DShadow _Sampler, float _P);
// [Generic] vec2 textureQueryLod(sampler2DShadow sampler, vec2 P);
vec2 textureQueryLod(sampler2DShadow _Sampler, vec2 _P);
// [Generic] vec2 textureQueryLod(samplerCubeShadow sampler, vec3 P);
vec2 textureQueryLod(samplerCubeShadow _Sampler, vec3 _P);
// [Generic] vec2 textureQueryLod(sampler1DArrayShadow sampler, float P);
vec2 textureQueryLod(sampler1DArrayShadow _Sampler, float _P);
// [Generic] vec2 textureQueryLod(sampler2DArrayShadow sampler, vec2 P);
vec2 textureQueryLod(sampler2DArrayShadow _Sampler, vec2 _P);
// [Generic] vec2 textureQueryLod(samplerCubeArrayShadow sampler, vec P);
vec2 textureQueryLod(samplerCubeArrayShadow _Sampler, vec2 _P);
vec2 textureQueryLod(samplerCubeArrayShadow _Sampler, vec3 _P);
vec2 textureQueryLod(samplerCubeArrayShadow _Sampler, vec4 _P);
vec2 textureQueryLod(samplerCubeArrayShadow _Sampler, dvec2 _P);
vec2 textureQueryLod(samplerCubeArrayShadow _Sampler, dvec3 _P);
vec2 textureQueryLod(samplerCubeArrayShadow _Sampler, dvec4 _P);

// TextureQueryLevels
// Returns the number of mipmap levels accessible in the texture associated with sampler, as defined
// in the <<references,{apispec}.>>
// The value zero will be returned if no texture or an incomplete texture is associated with sampler.
// Available in all shader stages.
// [Generic] int textureQueryLevels(gsampler1D sampler);
int textureQueryLevels(sampler1D _Sampler);
int textureQueryLevels(isampler1D _Sampler);
int textureQueryLevels(usampler1D _Sampler);
// [Generic] int textureQueryLevels(gsampler2D sampler);
int textureQueryLevels(sampler2D _Sampler);
int textureQueryLevels(isampler2D _Sampler);
int textureQueryLevels(usampler2D _Sampler);
// [Generic] int textureQueryLevels(gsampler3D sampler);
int textureQueryLevels(sampler3D _Sampler);
int textureQueryLevels(isampler3D _Sampler);
int textureQueryLevels(usampler3D _Sampler);
// [Generic] int textureQueryLevels(gsamplerCube sampler);
int textureQueryLevels(samplerCube _Sampler);
int textureQueryLevels(isamplerCube _Sampler);
int textureQueryLevels(usamplerCube _Sampler);
// [Generic] int textureQueryLevels(gsampler1DArray sampler);
int textureQueryLevels(sampler1DArray _Sampler);
int textureQueryLevels(isampler1DArray _Sampler);
int textureQueryLevels(usampler1DArray _Sampler);
// [Generic] int textureQueryLevels(gsampler2DArray sampler);
int textureQueryLevels(sampler2DArray _Sampler);
int textureQueryLevels(isampler2DArray _Sampler);
int textureQueryLevels(usampler2DArray _Sampler);
// [Generic] int textureQueryLevels(gsamplerCubeArray sampler);
int textureQueryLevels(samplerCubeArray _Sampler);
int textureQueryLevels(isamplerCubeArray _Sampler);
int textureQueryLevels(usamplerCubeArray _Sampler);
// [Generic] int textureQueryLevels(sampler1DShadow sampler);
int textureQueryLevels(sampler1DShadow _Sampler);
// [Generic] int textureQueryLevels(sampler2DShadow sampler);
int textureQueryLevels(sampler2DShadow _Sampler);
// [Generic] int textureQueryLevels(samplerCubeShadow sampler);
int textureQueryLevels(samplerCubeShadow _Sampler);
// [Generic] int textureQueryLevels(sampler1DArrayShadow sampler);
int textureQueryLevels(sampler1DArrayShadow _Sampler);
// [Generic] int textureQueryLevels(sampler2DArrayShadow sampler);
int textureQueryLevels(sampler2DArrayShadow _Sampler);
// [Generic] int textureQueryLevels(samplerCubeArrayShadow sampler);
int textureQueryLevels(samplerCubeArrayShadow _Sampler);

// TextureSamples
// Returns the number of samples of the texture bound to sampler.
// [Generic] int textureSamples(gsampler2DMS sampler);
int textureSamples(sampler2DMS _Sampler);
int textureSamples(isampler2DMS _Sampler);
int textureSamples(usampler2DMS _Sampler);
// [Generic] int textureSamples(gsampler2DMSArray sampler);
int textureSamples(sampler2DMSArray _Sampler);
int textureSamples(isampler2DMSArray _Sampler);
int textureSamples(usampler2DMSArray _Sampler);

// Texel Lookup Functions

// Texture
// Use the texture coordinate P to do a texture lookup in the texture currently bound to sampler.
// For shadow forms: When compare is present, it is used as D~ref~ and the array layer comes from
// the last component of P. When compare is not present, the last component of P is used as
// D~ref~ and the array layer comes from the second to last component of P.
// (The second component of P is unused for 1D shadow lookups.)
// For non-shadow forms: the array layer comes from the last component of P.
// [Generic] gvec4 texture(gsampler1D sampler, float P [, float bias]);
vec4 texture(sampler1D _Sampler, float _P);
ivec4 texture(isampler1D _Sampler, float _P);
uvec4 texture(usampler1D _Sampler, float _P);
vec4 texture(sampler1D _Sampler, float _P, float _Bias);
ivec4 texture(isampler1D _Sampler, float _P, float _Bias);
uvec4 texture(usampler1D _Sampler, float _P, float _Bias);
// [Generic] gvec4 texture(gsampler2D sampler, vec2 P [, float bias]);
vec4 texture(sampler2D _Sampler, vec2 _P);
ivec4 texture(isampler2D _Sampler, vec2 _P);
uvec4 texture(usampler2D _Sampler, vec2 _P);
vec4 texture(sampler2D _Sampler, vec2 _P, float _Bias);
ivec4 texture(isampler2D _Sampler, vec2 _P, float _Bias);
uvec4 texture(usampler2D _Sampler, vec2 _P, float _Bias);
// [Generic] gvec4 texture(gsampler3D sampler, vec3 P [, float bias]);
vec4 texture(sampler3D _Sampler, vec3 _P);
ivec4 texture(isampler3D _Sampler, vec3 _P);
uvec4 texture(usampler3D _Sampler, vec3 _P);
vec4 texture(sampler3D _Sampler, vec3 _P, float _Bias);
ivec4 texture(isampler3D _Sampler, vec3 _P, float _Bias);
uvec4 texture(usampler3D _Sampler, vec3 _P, float _Bias);
// [Generic] gvec4 texture(gsamplerCube sampler, vec3 P [, float bias]);
vec4 texture(samplerCube _Sampler, vec3 _P);
ivec4 texture(isamplerCube _Sampler, vec3 _P);
uvec4 texture(usamplerCube _Sampler, vec3 _P);
vec4 texture(samplerCube _Sampler, vec3 _P, float _Bias);
ivec4 texture(isamplerCube _Sampler, vec3 _P, float _Bias);
uvec4 texture(usamplerCube _Sampler, vec3 _P, float _Bias);
// [Generic] float texture(sampler1DShadow sampler, vec3 P [, float bias]);
float texture(sampler1DShadow _Sampler, vec3 _P);
float texture(sampler1DShadow _Sampler, vec3 _P, float _Bias);
// [Generic] float texture(sampler2DShadow sampler, vec3 P [, float bias]);
float texture(sampler2DShadow _Sampler, vec3 _P);
float texture(sampler2DShadow _Sampler, vec3 _P, float _Bias);
// [Generic] float texture(samplerCubeShadow sampler, vec4 P [, float bias]);
float texture(samplerCubeShadow _Sampler, vec4 _P);
float texture(samplerCubeShadow _Sampler, vec4 _P, float _Bias);
// [Generic] gvec4 texture(gsampler2DArray sampler, vec3 P [, float bias]);
vec4 texture(sampler2DArray _Sampler, vec3 _P);
ivec4 texture(isampler2DArray _Sampler, vec3 _P);
uvec4 texture(usampler2DArray _Sampler, vec3 _P);
vec4 texture(sampler2DArray _Sampler, vec3 _P, float _Bias);
ivec4 texture(isampler2DArray _Sampler, vec3 _P, float _Bias);
uvec4 texture(usampler2DArray _Sampler, vec3 _P, float _Bias);
// [Generic] gvec4 texture(gsamplerCubeArray sampler, vec4 P [, float bias]);
vec4 texture(samplerCubeArray _Sampler, vec4 _P);
ivec4 texture(isamplerCubeArray _Sampler, vec4 _P);
uvec4 texture(usamplerCubeArray _Sampler, vec4 _P);
vec4 texture(samplerCubeArray _Sampler, vec4 _P, float _Bias);
ivec4 texture(isamplerCubeArray _Sampler, vec4 _P, float _Bias);
uvec4 texture(usamplerCubeArray _Sampler, vec4 _P, float _Bias);
// [Generic] gvec4 texture(gsampler1DArray sampler, vec2 P [, float bias]);
vec4 texture(sampler1DArray _Sampler, vec2 _P);
ivec4 texture(isampler1DArray _Sampler, vec2 _P);
uvec4 texture(usampler1DArray _Sampler, vec2 _P);
vec4 texture(sampler1DArray _Sampler, vec2 _P, float _Bias);
ivec4 texture(isampler1DArray _Sampler, vec2 _P, float _Bias);
uvec4 texture(usampler1DArray _Sampler, vec2 _P, float _Bias);
// [Generic] float texture(sampler1DArrayShadow sampler, vec3 P [, float bias]);
float texture(sampler1DArrayShadow _Sampler, vec3 _P);
float texture(sampler1DArrayShadow _Sampler, vec3 _P, float _Bias);
// [Generic] float texture(sampler2DArrayShadow sampler, vec4 P);
float texture(sampler2DArrayShadow _Sampler, vec4 _P);
// [Generic] gvec4 texture(gsampler2DRect sampler, vec2 P);
vec4 texture(sampler2DRect _Sampler, vec2 _P);
ivec4 texture(isampler2DRect _Sampler, vec2 _P);
uvec4 texture(usampler2DRect _Sampler, vec2 _P);
// [Generic] float texture(sampler2DRectShadow sampler, vec3 P);
float texture(sampler2DRectShadow _Sampler, vec3 _P);
// [Generic] float texture(samplerCubeArrayShadow sampler, vec4 P, float compare);
float texture(samplerCubeArrayShadow _Sampler, vec4 _P, float _Compare);

// TextureProj
// Do a texture lookup with projection.
// The texture coordinates consumed from P, not including the last component of P, are divided by
// the last component of P to form projected coordinates P'. The resulting third component of P
// in the shadow forms is used as D~ref~. The third component of P is ignored when sampler has
// type gsampler2D and P has type vec4. After these values are computed, texture lookup proceeds
// as in texture.
// [Generic] gvec4 textureProj(gsampler1D sampler, vec2 P [, float bias]);
vec4 textureProj(sampler1D _Sampler, vec2 _P);
ivec4 textureProj(isampler1D _Sampler, vec2 _P);
uvec4 textureProj(usampler1D _Sampler, vec2 _P);
vec4 textureProj(sampler1D _Sampler, vec2 _P, float _Bias);
ivec4 textureProj(isampler1D _Sampler, vec2 _P, float _Bias);
uvec4 textureProj(usampler1D _Sampler, vec2 _P, float _Bias);
// [Generic] gvec4 textureProj(gsampler1D sampler, vec4 P [, float bias]);
vec4 textureProj(sampler1D _Sampler, vec4 _P);
ivec4 textureProj(isampler1D _Sampler, vec4 _P);
uvec4 textureProj(usampler1D _Sampler, vec4 _P);
vec4 textureProj(sampler1D _Sampler, vec4 _P, float _Bias);
ivec4 textureProj(isampler1D _Sampler, vec4 _P, float _Bias);
uvec4 textureProj(usampler1D _Sampler, vec4 _P, float _Bias);
// [Generic] gvec4 textureProj(gsampler2D sampler, vec3 P [, float bias]);
vec4 textureProj(sampler2D _Sampler, vec3 _P);
ivec4 textureProj(isampler2D _Sampler, vec3 _P);
uvec4 textureProj(usampler2D _Sampler, vec3 _P);
vec4 textureProj(sampler2D _Sampler, vec3 _P, float _Bias);
ivec4 textureProj(isampler2D _Sampler, vec3 _P, float _Bias);
uvec4 textureProj(usampler2D _Sampler, vec3 _P, float _Bias);
// [Generic] gvec4 textureProj(gsampler2D sampler, vec4 P [, float bias]);
vec4 textureProj(sampler2D _Sampler, vec4 _P);
ivec4 textureProj(isampler2D _Sampler, vec4 _P);
uvec4 textureProj(usampler2D _Sampler, vec4 _P);
vec4 textureProj(sampler2D _Sampler, vec4 _P, float _Bias);
ivec4 textureProj(isampler2D _Sampler, vec4 _P, float _Bias);
uvec4 textureProj(usampler2D _Sampler, vec4 _P, float _Bias);
// [Generic] gvec4 textureProj(gsampler3D sampler, vec4 P [, float bias]);
vec4 textureProj(sampler3D _Sampler, vec4 _P);
ivec4 textureProj(isampler3D _Sampler, vec4 _P);
uvec4 textureProj(usampler3D _Sampler, vec4 _P);
vec4 textureProj(sampler3D _Sampler, vec4 _P, float _Bias);
ivec4 textureProj(isampler3D _Sampler, vec4 _P, float _Bias);
uvec4 textureProj(usampler3D _Sampler, vec4 _P, float _Bias);
// [Generic] float textureProj(sampler1DShadow sampler, vec4 P [, float bias]);
float textureProj(sampler1DShadow _Sampler, vec4 _P);
float textureProj(sampler1DShadow _Sampler, vec4 _P, float _Bias);
// [Generic] float textureProj(sampler2DShadow sampler, vec4 P [, float bias]);
float textureProj(sampler2DShadow _Sampler, vec4 _P);
float textureProj(sampler2DShadow _Sampler, vec4 _P, float _Bias);
// [Generic] gvec4 textureProj(gsampler2DRect sampler, vec3 P);
vec4 textureProj(sampler2DRect _Sampler, vec3 _P);
ivec4 textureProj(isampler2DRect _Sampler, vec3 _P);
uvec4 textureProj(usampler2DRect _Sampler, vec3 _P);
// [Generic] gvec4 textureProj(gsampler2DRect sampler, vec4 P);
vec4 textureProj(sampler2DRect _Sampler, vec4 _P);
ivec4 textureProj(isampler2DRect _Sampler, vec4 _P);
uvec4 textureProj(usampler2DRect _Sampler, vec4 _P);
// [Generic] float textureProj(sampler2DRectShadow sampler, vec4 P);
float textureProj(sampler2DRectShadow _Sampler, vec4 _P);

// TextureLod
// Do a texture lookup as in texture but with explicit level-of-detail; lod specifies
// [Generic] gvec4 textureLod(gsampler1D sampler, float P, float lod);
vec4 textureLod(sampler1D _Sampler, float _P, float _Lod);
ivec4 textureLod(isampler1D _Sampler, float _P, float _Lod);
uvec4 textureLod(usampler1D _Sampler, float _P, float _Lod);
// [Generic] gvec4 textureLod(gsampler2D sampler, vec2 P, float lod);
vec4 textureLod(sampler2D _Sampler, vec2 _P, float _Lod);
ivec4 textureLod(isampler2D _Sampler, vec2 _P, float _Lod);
uvec4 textureLod(usampler2D _Sampler, vec2 _P, float _Lod);
// [Generic] gvec4 textureLod(gsampler3D sampler, vec3 P, float lod);
vec4 textureLod(sampler3D _Sampler, vec3 _P, float _Lod);
ivec4 textureLod(isampler3D _Sampler, vec3 _P, float _Lod);
uvec4 textureLod(usampler3D _Sampler, vec3 _P, float _Lod);
// [Generic] gvec4 textureLod(gsamplerCube sampler, vec3 P, float lod);
vec4 textureLod(samplerCube _Sampler, vec3 _P, float _Lod);
ivec4 textureLod(isamplerCube _Sampler, vec3 _P, float _Lod);
uvec4 textureLod(usamplerCube _Sampler, vec3 _P, float _Lod);
// [Generic] float textureLod(sampler2DShadow sampler, vec3 P, float lod);
float textureLod(sampler2DShadow _Sampler, vec3 _P, float _Lod);
// [Generic] float textureLod(sampler1DShadow sampler, vec3 P, float lod);
float textureLod(sampler1DShadow _Sampler, vec3 _P, float _Lod);
// [Generic] gvec4 textureLod(gsampler1DArray sampler, vec2 P, float lod);
vec4 textureLod(sampler1DArray _Sampler, vec2 _P, float _Lod);
ivec4 textureLod(isampler1DArray _Sampler, vec2 _P, float _Lod);
uvec4 textureLod(usampler1DArray _Sampler, vec2 _P, float _Lod);
// [Generic] float textureLod(sampler1DArrayShadow sampler, vec3 P, float lod);
float textureLod(sampler1DArrayShadow _Sampler, vec3 _P, float _Lod);
// [Generic] gvec4 textureLod(gsampler2DArray sampler, vec3 P, float lod);
vec4 textureLod(sampler2DArray _Sampler, vec3 _P, float _Lod);
ivec4 textureLod(isampler2DArray _Sampler, vec3 _P, float _Lod);
uvec4 textureLod(usampler2DArray _Sampler, vec3 _P, float _Lod);
// [Generic] gvec4 textureLod(gsamplerCubeArray sampler, vec4 P, float lod);
vec4 textureLod(samplerCubeArray _Sampler, vec4 _P, float _Lod);
ivec4 textureLod(isamplerCubeArray _Sampler, vec4 _P, float _Lod);
uvec4 textureLod(usamplerCubeArray _Sampler, vec4 _P, float _Lod);

// TextureOffset
// Do a texture lookup as in texture but with offset added to the (u,v,w) texel coordinates
// before looking up each texel. The offset value must be a constant expression. A limited range of
// offset values are supported; the minimum and maximum offset values are implementation-dependent and
// given by glMinProgramTexelOffset and glMaxProgramTexelOffset, respectively.
// Note that offset does not apply to the layer coordinate for texture arrays.
// This is explained in detail in section 8.14.2 "Coordinate Wrapping and Texel Selection" of the
// <<references,{apispec}>>, where offset is (δ~u~, δ~v~, δ~w~).
// Note that texel offsets are also not supported for cube maps.
// [Generic] gvec4 textureOffset(gsampler1D sampler, float P, int offset [, float bias]);
vec4 textureOffset(sampler1D _Sampler, float _P, int _Offset);
ivec4 textureOffset(isampler1D _Sampler, float _P, int _Offset);
uvec4 textureOffset(usampler1D _Sampler, float _P, int _Offset);
vec4 textureOffset(sampler1D _Sampler, float _P, int _Offset, float _Bias);
ivec4 textureOffset(isampler1D _Sampler, float _P, int _Offset, float _Bias);
uvec4 textureOffset(usampler1D _Sampler, float _P, int _Offset, float _Bias);
// [Generic] gvec4 textureOffset(gsampler2D sampler, vec2 P, ivec2 offset [, float bias]);
vec4 textureOffset(sampler2D _Sampler, vec2 _P, ivec2 _Offset);
ivec4 textureOffset(isampler2D _Sampler, vec2 _P, ivec2 _Offset);
uvec4 textureOffset(usampler2D _Sampler, vec2 _P, ivec2 _Offset);
vec4 textureOffset(sampler2D _Sampler, vec2 _P, ivec2 _Offset, float _Bias);
ivec4 textureOffset(isampler2D _Sampler, vec2 _P, ivec2 _Offset, float _Bias);
uvec4 textureOffset(usampler2D _Sampler, vec2 _P, ivec2 _Offset, float _Bias);
// [Generic] gvec4 textureOffset(gsampler3D sampler, vec3 P, ivec3 offset [, float bias]);
vec4 textureOffset(sampler3D _Sampler, vec3 _P, ivec3 _Offset);
ivec4 textureOffset(isampler3D _Sampler, vec3 _P, ivec3 _Offset);
uvec4 textureOffset(usampler3D _Sampler, vec3 _P, ivec3 _Offset);
vec4 textureOffset(sampler3D _Sampler, vec3 _P, ivec3 _Offset, float _Bias);
ivec4 textureOffset(isampler3D _Sampler, vec3 _P, ivec3 _Offset, float _Bias);
uvec4 textureOffset(usampler3D _Sampler, vec3 _P, ivec3 _Offset, float _Bias);
// [Generic] float textureOffset(sampler2DShadow sampler, vec3 P, ivec2 offset [, float bias]);
float textureOffset(sampler2DShadow _Sampler, vec3 _P, ivec2 _Offset);
float textureOffset(sampler2DShadow _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
// [Generic] gvec4 textureOffset(gsampler2DRect sampler, vec2 P, ivec2 offset);
vec4 textureOffset(sampler2DRect _Sampler, vec2 _P, ivec2 _Offset);
ivec4 textureOffset(isampler2DRect _Sampler, vec2 _P, ivec2 _Offset);
uvec4 textureOffset(usampler2DRect _Sampler, vec2 _P, ivec2 _Offset);
// [Generic] float textureOffset(sampler2DRectShadow sampler, vec3 P, ivec2 offset);
float textureOffset(sampler2DRectShadow _Sampler, vec3 _P, ivec2 _Offset);
// [Generic] float textureOffset(sampler1DShadow sampler, vec3 P, int offset [, float bias]);
float textureOffset(sampler1DShadow _Sampler, vec3 _P, int _Offset);
float textureOffset(sampler1DShadow _Sampler, vec3 _P, int _Offset, float _Bias);
// [Generic] gvec4 textureOffset(gsampler1DArray sampler, vec2 P, int offset [, float bias]);
vec4 textureOffset(sampler1DArray _Sampler, vec2 _P, int _Offset);
ivec4 textureOffset(isampler1DArray _Sampler, vec2 _P, int _Offset);
uvec4 textureOffset(usampler1DArray _Sampler, vec2 _P, int _Offset);
vec4 textureOffset(sampler1DArray _Sampler, vec2 _P, int _Offset, float _Bias);
ivec4 textureOffset(isampler1DArray _Sampler, vec2 _P, int _Offset, float _Bias);
uvec4 textureOffset(usampler1DArray _Sampler, vec2 _P, int _Offset, float _Bias);
// [Generic] gvec4 textureOffset(gsampler2DArray sampler, vec3 P, ivec2 offset [, float bias]);
vec4 textureOffset(sampler2DArray _Sampler, vec3 _P, ivec2 _Offset);
ivec4 textureOffset(isampler2DArray _Sampler, vec3 _P, ivec2 _Offset);
uvec4 textureOffset(usampler2DArray _Sampler, vec3 _P, ivec2 _Offset);
vec4 textureOffset(sampler2DArray _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
ivec4 textureOffset(isampler2DArray _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
uvec4 textureOffset(usampler2DArray _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
// [Generic] float textureOffset(sampler1DArrayShadow sampler, vec3 P, int offset [, float bias]);
float textureOffset(sampler1DArrayShadow _Sampler, vec3 _P, int _Offset);
float textureOffset(sampler1DArrayShadow _Sampler, vec3 _P, int _Offset, float _Bias);
// [Generic] float textureOffset(sampler2DArrayShadow sampler, vec4 P, ivec2 offset);
float textureOffset(sampler2DArrayShadow _Sampler, vec4 _P, ivec2 _Offset);

// TexelFetch
// Use integer texture coordinate P to lookup a single texel from sampler. The array layer comes
// from the last component of P for the array forms. The level-of-detail lod (if present) is as
// described in sections 11.1.3.2 "Texel Fetches" and 8.14.1 "Scale Factor and Level of Detail" of
// the <<references,{apispec}>>.
// [Generic] gvec4 texelFetch(gsampler1D sampler, int P, int lod);
vec4 texelFetch(sampler1D _Sampler, int _P, int _Lod);
ivec4 texelFetch(isampler1D _Sampler, int _P, int _Lod);
uvec4 texelFetch(usampler1D _Sampler, int _P, int _Lod);
// [Generic] gvec4 texelFetch(gsampler2D sampler, ivec2 P, int lod);
vec4 texelFetch(sampler2D _Sampler, ivec2 _P, int _Lod);
ivec4 texelFetch(isampler2D _Sampler, ivec2 _P, int _Lod);
uvec4 texelFetch(usampler2D _Sampler, ivec2 _P, int _Lod);
// [Generic] gvec4 texelFetch(gsampler3D sampler, ivec3 P, int lod);
vec4 texelFetch(sampler3D _Sampler, ivec3 _P, int _Lod);
ivec4 texelFetch(isampler3D _Sampler, ivec3 _P, int _Lod);
uvec4 texelFetch(usampler3D _Sampler, ivec3 _P, int _Lod);
// [Generic] gvec4 texelFetch(gsampler2DRect sampler, ivec2 P);
vec4 texelFetch(sampler2DRect _Sampler, ivec2 _P);
ivec4 texelFetch(isampler2DRect _Sampler, ivec2 _P);
uvec4 texelFetch(usampler2DRect _Sampler, ivec2 _P);
// [Generic] gvec4 texelFetch(gsampler1DArray sampler, ivec2 P, int lod);
vec4 texelFetch(sampler1DArray _Sampler, ivec2 _P, int _Lod);
ivec4 texelFetch(isampler1DArray _Sampler, ivec2 _P, int _Lod);
uvec4 texelFetch(usampler1DArray _Sampler, ivec2 _P, int _Lod);
// [Generic] gvec4 texelFetch(gsampler2DArray sampler, ivec3 P, int lod);
vec4 texelFetch(sampler2DArray _Sampler, ivec3 _P, int _Lod);
ivec4 texelFetch(isampler2DArray _Sampler, ivec3 _P, int _Lod);
uvec4 texelFetch(usampler2DArray _Sampler, ivec3 _P, int _Lod);
// [Generic] gvec4 texelFetch(gsamplerBuffer sampler, int P);
vec4 texelFetch(samplerBuffer _Sampler, int _P);
ivec4 texelFetch(isamplerBuffer _Sampler, int _P);
uvec4 texelFetch(usamplerBuffer _Sampler, int _P);
// [Generic] gvec4 texelFetch(gsampler2DMS sampler, ivec2 P, int sample);
vec4 texelFetch(sampler2DMS _Sampler, ivec2 _P, int _Sample);
ivec4 texelFetch(isampler2DMS _Sampler, ivec2 _P, int _Sample);
uvec4 texelFetch(usampler2DMS _Sampler, ivec2 _P, int _Sample);
// [Generic] gvec4 texelFetch(gsampler2DMSArray sampler, ivec3 P, int sample);
vec4 texelFetch(sampler2DMSArray _Sampler, ivec3 _P, int _Sample);
ivec4 texelFetch(isampler2DMSArray _Sampler, ivec3 _P, int _Sample);
uvec4 texelFetch(usampler2DMSArray _Sampler, ivec3 _P, int _Sample);

// TexelFetchOffset
// Fetch a single texel as in texelFetch, offset by offset as described in textureOffset.
// [Generic] gvec4 texelFetchOffset(gsampler1D sampler, int P, int lod, int offset);
vec4 texelFetchOffset(sampler1D _Sampler, int _P, int _Lod, int _Offset);
ivec4 texelFetchOffset(isampler1D _Sampler, int _P, int _Lod, int _Offset);
uvec4 texelFetchOffset(usampler1D _Sampler, int _P, int _Lod, int _Offset);
// [Generic] gvec4 texelFetchOffset(gsampler2D sampler, ivec2 P, int lod, ivec2 offset);
vec4 texelFetchOffset(sampler2D _Sampler, ivec2 _P, int _Lod, ivec2 _Offset);
ivec4 texelFetchOffset(isampler2D _Sampler, ivec2 _P, int _Lod, ivec2 _Offset);
uvec4 texelFetchOffset(usampler2D _Sampler, ivec2 _P, int _Lod, ivec2 _Offset);
// [Generic] gvec4 texelFetchOffset(gsampler3D sampler, ivec3 P, int lod, ivec3 offset);
vec4 texelFetchOffset(sampler3D _Sampler, ivec3 _P, int _Lod, ivec3 _Offset);
ivec4 texelFetchOffset(isampler3D _Sampler, ivec3 _P, int _Lod, ivec3 _Offset);
uvec4 texelFetchOffset(usampler3D _Sampler, ivec3 _P, int _Lod, ivec3 _Offset);
// [Generic] gvec4 texelFetchOffset(gsampler2DRect sampler, ivec2 P, ivec2 offset);
vec4 texelFetchOffset(sampler2DRect _Sampler, ivec2 _P, ivec2 _Offset);
ivec4 texelFetchOffset(isampler2DRect _Sampler, ivec2 _P, ivec2 _Offset);
uvec4 texelFetchOffset(usampler2DRect _Sampler, ivec2 _P, ivec2 _Offset);
// [Generic] gvec4 texelFetchOffset(gsampler1DArray sampler, ivec2 P, int lod, int offset);
vec4 texelFetchOffset(sampler1DArray _Sampler, ivec2 _P, int _Lod, int _Offset);
ivec4 texelFetchOffset(isampler1DArray _Sampler, ivec2 _P, int _Lod, int _Offset);
uvec4 texelFetchOffset(usampler1DArray _Sampler, ivec2 _P, int _Lod, int _Offset);
// [Generic] gvec4 texelFetchOffset(gsampler2DArray sampler, ivec3 P, int lod, ivec2 offset);
vec4 texelFetchOffset(sampler2DArray _Sampler, ivec3 _P, int _Lod, ivec2 _Offset);
ivec4 texelFetchOffset(isampler2DArray _Sampler, ivec3 _P, int _Lod, ivec2 _Offset);
uvec4 texelFetchOffset(usampler2DArray _Sampler, ivec3 _P, int _Lod, ivec2 _Offset);

// TextureProjOffset
// Do a projective texture lookup as described in textureProj, offset by offset as described in
// textureOffset.
// [Generic] gvec4 textureProjOffset(gsampler1D sampler, vec2 P, int offset [, float bias]);
vec4 textureProjOffset(sampler1D _Sampler, vec2 _P, int _Offset);
ivec4 textureProjOffset(isampler1D _Sampler, vec2 _P, int _Offset);
uvec4 textureProjOffset(usampler1D _Sampler, vec2 _P, int _Offset);
vec4 textureProjOffset(sampler1D _Sampler, vec2 _P, int _Offset, float _Bias);
ivec4 textureProjOffset(isampler1D _Sampler, vec2 _P, int _Offset, float _Bias);
uvec4 textureProjOffset(usampler1D _Sampler, vec2 _P, int _Offset, float _Bias);
// [Generic] gvec4 textureProjOffset(gsampler1D sampler, vec4 P, int offset [, float bias]);
vec4 textureProjOffset(sampler1D _Sampler, vec4 _P, int _Offset);
ivec4 textureProjOffset(isampler1D _Sampler, vec4 _P, int _Offset);
uvec4 textureProjOffset(usampler1D _Sampler, vec4 _P, int _Offset);
vec4 textureProjOffset(sampler1D _Sampler, vec4 _P, int _Offset, float _Bias);
ivec4 textureProjOffset(isampler1D _Sampler, vec4 _P, int _Offset, float _Bias);
uvec4 textureProjOffset(usampler1D _Sampler, vec4 _P, int _Offset, float _Bias);
// [Generic] gvec4 textureProjOffset(gsampler2D sampler, vec3 P, ivec2 offset [, float bias]);
vec4 textureProjOffset(sampler2D _Sampler, vec3 _P, ivec2 _Offset);
ivec4 textureProjOffset(isampler2D _Sampler, vec3 _P, ivec2 _Offset);
uvec4 textureProjOffset(usampler2D _Sampler, vec3 _P, ivec2 _Offset);
vec4 textureProjOffset(sampler2D _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
ivec4 textureProjOffset(isampler2D _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
uvec4 textureProjOffset(usampler2D _Sampler, vec3 _P, ivec2 _Offset, float _Bias);
// [Generic] gvec4 textureProjOffset(gsampler2D sampler, vec4 P, ivec2 offset [, float bias]);
vec4 textureProjOffset(sampler2D _Sampler, vec4 _P, ivec2 _Offset);
ivec4 textureProjOffset(isampler2D _Sampler, vec4 _P, ivec2 _Offset);
uvec4 textureProjOffset(usampler2D _Sampler, vec4 _P, ivec2 _Offset);
vec4 textureProjOffset(sampler2D _Sampler, vec4 _P, ivec2 _Offset, float _Bias);
ivec4 textureProjOffset(isampler2D _Sampler, vec4 _P, ivec2 _Offset, float _Bias);
uvec4 textureProjOffset(usampler2D _Sampler, vec4 _P, ivec2 _Offset, float _Bias);
// [Generic] gvec4 textureProjOffset(gsampler3D sampler, vec4 P, ivec3 offset [, float bias]);
vec4 textureProjOffset(sampler3D _Sampler, vec4 _P, ivec3 _Offset);
ivec4 textureProjOffset(isampler3D _Sampler, vec4 _P, ivec3 _Offset);
uvec4 textureProjOffset(usampler3D _Sampler, vec4 _P, ivec3 _Offset);
vec4 textureProjOffset(sampler3D _Sampler, vec4 _P, ivec3 _Offset, float _Bias);
ivec4 textureProjOffset(isampler3D _Sampler, vec4 _P, ivec3 _Offset, float _Bias);
uvec4 textureProjOffset(usampler3D _Sampler, vec4 _P, ivec3 _Offset, float _Bias);
// [Generic] gvec4 textureProjOffset(gsampler2DRect sampler, vec3 P, ivec2 offset);
vec4 textureProjOffset(sampler2DRect _Sampler, vec3 _P, ivec2 _Offset);
ivec4 textureProjOffset(isampler2DRect _Sampler, vec3 _P, ivec2 _Offset);
uvec4 textureProjOffset(usampler2DRect _Sampler, vec3 _P, ivec2 _Offset);
// [Generic] gvec4 textureProjOffset(gsampler2DRect sampler, vec4 P, ivec2 offset);
vec4 textureProjOffset(sampler2DRect _Sampler, vec4 _P, ivec2 _Offset);
ivec4 textureProjOffset(isampler2DRect _Sampler, vec4 _P, ivec2 _Offset);
uvec4 textureProjOffset(usampler2DRect _Sampler, vec4 _P, ivec2 _Offset);
// [Generic] float textureProjOffset(sampler2DRectShadow sampler, vec4 P, ivec2 offset);
float textureProjOffset(sampler2DRectShadow _Sampler, vec4 _P, ivec2 _Offset);
// [Generic] float textureProjOffset(sampler1DShadow sampler, vec4 P, int offset [, float bias]);
float textureProjOffset(sampler1DShadow _Sampler, vec4 _P, int _Offset);
float textureProjOffset(sampler1DShadow _Sampler, vec4 _P, int _Offset, float _Bias);
// [Generic] float textureProjOffset(sampler2DShadow sampler, vec4 P, ivec2 offset [, float bias]);
float textureProjOffset(sampler2DShadow _Sampler, vec4 _P, ivec2 _Offset);
float textureProjOffset(sampler2DShadow _Sampler, vec4 _P, ivec2 _Offset, float _Bias);

// TextureLodOffset
// Do an offset texture lookup with explicit level-of-detail. See textureLod and textureOffset.
// [Generic] gvec4 textureLodOffset(gsampler1D sampler, float P, float lod, int offset);
vec4 textureLodOffset(sampler1D _Sampler, float _P, float _Lod, int _Offset);
ivec4 textureLodOffset(isampler1D _Sampler, float _P, float _Lod, int _Offset);
uvec4 textureLodOffset(usampler1D _Sampler, float _P, float _Lod, int _Offset);
// [Generic] gvec4 textureLodOffset(gsampler2D sampler, vec2 P, float lod, ivec2 offset);
vec4 textureLodOffset(sampler2D _Sampler, vec2 _P, float _Lod, ivec2 _Offset);
ivec4 textureLodOffset(isampler2D _Sampler, vec2 _P, float _Lod, ivec2 _Offset);
uvec4 textureLodOffset(usampler2D _Sampler, vec2 _P, float _Lod, ivec2 _Offset);
// [Generic] gvec4 textureLodOffset(gsampler3D sampler, vec3 P, float lod, ivec3 offset);
vec4 textureLodOffset(sampler3D _Sampler, vec3 _P, float _Lod, ivec3 _Offset);
ivec4 textureLodOffset(isampler3D _Sampler, vec3 _P, float _Lod, ivec3 _Offset);
uvec4 textureLodOffset(usampler3D _Sampler, vec3 _P, float _Lod, ivec3 _Offset);
// [Generic] float textureLodOffset(sampler1DShadow sampler, vec3 P, float lod, int offset);
float textureLodOffset(sampler1DShadow _Sampler, vec3 _P, float _Lod, int _Offset);
// [Generic] float textureLodOffset(sampler2DShadow sampler, vec3 P, float lod, ivec2 offset);
float textureLodOffset(sampler2DShadow _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
// [Generic] gvec4 textureLodOffset(gsampler1DArray sampler, vec2 P, float lod, int offset);
vec4 textureLodOffset(sampler1DArray _Sampler, vec2 _P, float _Lod, int _Offset);
ivec4 textureLodOffset(isampler1DArray _Sampler, vec2 _P, float _Lod, int _Offset);
uvec4 textureLodOffset(usampler1DArray _Sampler, vec2 _P, float _Lod, int _Offset);
// [Generic] gvec4 textureLodOffset(gsampler2DArray sampler, vec3 P, float lod, ivec2 offset);
vec4 textureLodOffset(sampler2DArray _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
ivec4 textureLodOffset(isampler2DArray _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
uvec4 textureLodOffset(usampler2DArray _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
// [Generic] float textureLodOffset(sampler1DArrayShadow sampler, vec3 P, float lod, int offset);
float textureLodOffset(sampler1DArrayShadow _Sampler, vec3 _P, float _Lod, int _Offset);

// TextureProjLod
// Do a projective texture lookup with explicit level-of-detail. See textureProj and textureLod.
// [Generic] gvec4 textureProjLod(gsampler1D sampler, vec2 P, float lod);
vec4 textureProjLod(sampler1D _Sampler, vec2 _P, float _Lod);
ivec4 textureProjLod(isampler1D _Sampler, vec2 _P, float _Lod);
uvec4 textureProjLod(usampler1D _Sampler, vec2 _P, float _Lod);
// [Generic] gvec4 textureProjLod(gsampler1D sampler, vec4 P, float lod);
vec4 textureProjLod(sampler1D _Sampler, vec4 _P, float _Lod);
ivec4 textureProjLod(isampler1D _Sampler, vec4 _P, float _Lod);
uvec4 textureProjLod(usampler1D _Sampler, vec4 _P, float _Lod);
// [Generic] gvec4 textureProjLod(gsampler2D sampler, vec3 P, float lod);
vec4 textureProjLod(sampler2D _Sampler, vec3 _P, float _Lod);
ivec4 textureProjLod(isampler2D _Sampler, vec3 _P, float _Lod);
uvec4 textureProjLod(usampler2D _Sampler, vec3 _P, float _Lod);
// [Generic] gvec4 textureProjLod(gsampler2D sampler, vec4 P, float lod);
vec4 textureProjLod(sampler2D _Sampler, vec4 _P, float _Lod);
ivec4 textureProjLod(isampler2D _Sampler, vec4 _P, float _Lod);
uvec4 textureProjLod(usampler2D _Sampler, vec4 _P, float _Lod);
// [Generic] gvec4 textureProjLod(gsampler3D sampler, vec4 P, float lod);
vec4 textureProjLod(sampler3D _Sampler, vec4 _P, float _Lod);
ivec4 textureProjLod(isampler3D _Sampler, vec4 _P, float _Lod);
uvec4 textureProjLod(usampler3D _Sampler, vec4 _P, float _Lod);
// [Generic] float textureProjLod(sampler1DShadow sampler, vec4 P, float lod);
float textureProjLod(sampler1DShadow _Sampler, vec4 _P, float _Lod);
// [Generic] float textureProjLod(sampler2DShadow sampler, vec4 P, float lod);
float textureProjLod(sampler2DShadow _Sampler, vec4 _P, float _Lod);

// TextureProdLodOffset
// Do an offset projective texture lookup with explicit level-of-detail. See textureProj,
// textureLod, and textureOffset.
// [Generic] gvec4 textureProjLodOffset(gsampler1D sampler, vec2 P, float lod, int offset);
vec4 textureProjLodOffset(sampler1D _Sampler, vec2 _P, float _Lod, int _Offset);
ivec4 textureProjLodOffset(isampler1D _Sampler, vec2 _P, float _Lod, int _Offset);
uvec4 textureProjLodOffset(usampler1D _Sampler, vec2 _P, float _Lod, int _Offset);
// [Generic] gvec4 textureProjLodOffset(gsampler1D sampler, vec4 P, float lod, int offset);
vec4 textureProjLodOffset(sampler1D _Sampler, vec4 _P, float _Lod, int _Offset);
ivec4 textureProjLodOffset(isampler1D _Sampler, vec4 _P, float _Lod, int _Offset);
uvec4 textureProjLodOffset(usampler1D _Sampler, vec4 _P, float _Lod, int _Offset);
// [Generic] gvec4 textureProjLodOffset(gsampler2D sampler, vec3 P, float lod, ivec2 offset);
vec4 textureProjLodOffset(sampler2D _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
ivec4 textureProjLodOffset(isampler2D _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
uvec4 textureProjLodOffset(usampler2D _Sampler, vec3 _P, float _Lod, ivec2 _Offset);
// [Generic] gvec4 textureProjLodOffset(gsampler2D sampler, vec4 P, float lod, ivec2 offset);
vec4 textureProjLodOffset(sampler2D _Sampler, vec4 _P, float _Lod, ivec2 _Offset);
ivec4 textureProjLodOffset(isampler2D _Sampler, vec4 _P, float _Lod, ivec2 _Offset);
uvec4 textureProjLodOffset(usampler2D _Sampler, vec4 _P, float _Lod, ivec2 _Offset);
// [Generic] gvec4 textureProjLodOffset(gsampler3D sampler, vec4 P, float lod, ivec3 offset);
vec4 textureProjLodOffset(sampler3D _Sampler, vec4 _P, float _Lod, ivec3 _Offset);
ivec4 textureProjLodOffset(isampler3D _Sampler, vec4 _P, float _Lod, ivec3 _Offset);
uvec4 textureProjLodOffset(usampler3D _Sampler, vec4 _P, float _Lod, ivec3 _Offset);
// [Generic] float textureProjLodOffset(sampler1DShadow sampler, vec4 P, float lod, int offset);
float textureProjLodOffset(sampler1DShadow _Sampler, vec4 _P, float _Lod, int _Offset);
// [Generic] float textureProjLodOffset(sampler2DShadow sampler, vec4 P, float lod, ivec2 offset);
float textureProjLodOffset(sampler2DShadow _Sampler, vec4 _P, float _Lod, ivec2 _Offset);

// TextureGrad
// Do a texture lookup as in texture but with <<explicit-gradients, explicit gradients>> as shown
// below. The partial derivatives of P are with respect to window x and window y.
// For the cube version, the partial derivatives of P are assumed to be in the coordinate system
// used before texture coordinates are projected onto the appropriate cube face.
// [Generic] gvec4 textureGrad(gsampler1D sampler, float P, float dPdx, float dPdy);
vec4 textureGrad(sampler1D _Sampler, float _P, float _dPdx, float _dPdy);
ivec4 textureGrad(isampler1D _Sampler, float _P, float _dPdx, float _dPdy);
uvec4 textureGrad(usampler1D _Sampler, float _P, float _dPdx, float _dPdy);
// [Generic] gvec4 textureGrad(gsampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy);
vec4 textureGrad(sampler2D _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureGrad(isampler2D _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureGrad(usampler2D _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] gvec4 textureGrad(gsampler3D sampler, vec3 P, vec3 dPdx, vec3 dPdy);
vec4 textureGrad(sampler3D _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy);
ivec4 textureGrad(isampler3D _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy);
uvec4 textureGrad(usampler3D _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy);
// [Generic] gvec4 textureGrad(gsamplerCube sampler, vec3 P, vec3 dPdx, vec3 dPdy);
vec4 textureGrad(samplerCube _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy);
ivec4 textureGrad(isamplerCube _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy);
uvec4 textureGrad(usamplerCube _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy);
// [Generic] gvec4 textureGrad(gsampler2DRect sampler, vec2 P, vec2 dPdx, vec2 dPdy);
vec4 textureGrad(sampler2DRect _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureGrad(isampler2DRect _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureGrad(usampler2DRect _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] float textureGrad(sampler2DRectShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy);
float textureGrad(sampler2DRectShadow _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] float textureGrad(sampler1DShadow sampler, vec3 P, float dPdx, float dPdy);
float textureGrad(sampler1DShadow _Sampler, vec3 _P, float _dPdx, float _dPdy);
// [Generic] gvec4 textureGrad(gsampler1DArray sampler, vec2 P, float dPdx, float dPdy);
vec4 textureGrad(sampler1DArray _Sampler, vec2 _P, float _dPdx, float _dPdy);
ivec4 textureGrad(isampler1DArray _Sampler, vec2 _P, float _dPdx, float _dPdy);
uvec4 textureGrad(usampler1DArray _Sampler, vec2 _P, float _dPdx, float _dPdy);
// [Generic] float textureGrad(sampler1DArrayShadow sampler, vec3 P, float dPdx, float dPdy);
float textureGrad(sampler1DArrayShadow _Sampler, vec3 _P, float _dPdx, float _dPdy);
// [Generic] float textureGrad(sampler2DShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy);
float textureGrad(sampler2DShadow _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] float textureGrad(samplerCubeShadow sampler, vec4 P, vec3 dPdx, vec3 dPdy);
float textureGrad(samplerCubeShadow _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);
// [Generic] gvec4 textureGrad(gsampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy);
vec4 textureGrad(sampler2DArray _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureGrad(isampler2DArray _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureGrad(usampler2DArray _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] float textureGrad(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy);
float textureGrad(sampler2DArrayShadow _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] gvec4 textureGrad(gsamplerCubeArray sampler, vec4 P, vec3 dPdx, vec3 dPdy);
vec4 textureGrad(samplerCubeArray _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);
ivec4 textureGrad(isamplerCubeArray _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);
uvec4 textureGrad(usamplerCubeArray _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);

// Explicit Gradients

// TextureGradOffset
// Do a texture lookup with both explicit gradient and offset, as described in textureGrad and
// textureOffset.
// [Generic] gvec4 textureGradOffset(gsampler1D sampler, float P, float dPdx, float dPdy, int offset);
vec4 textureGradOffset(sampler1D _Sampler, float _P, float _dPdx, float _dPdy, int _Offset);
ivec4 textureGradOffset(isampler1D _Sampler, float _P, float _dPdx, float _dPdy, int _Offset);
uvec4 textureGradOffset(usampler1D _Sampler, float _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] gvec4 textureGradOffset(gsampler2D sampler, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureGradOffset(sampler2D _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureGradOffset(isampler2D _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureGradOffset(usampler2D _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] gvec4 textureGradOffset(gsampler3D sampler, vec3 P, vec3 dPdx, vec3 dPdy, ivec3 offset);
vec4 textureGradOffset(sampler3D _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy, ivec3 _Offset);
ivec4 textureGradOffset(isampler3D _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy, ivec3 _Offset);
uvec4 textureGradOffset(usampler3D _Sampler, vec3 _P, vec3 _dPdx, vec3 _dPdy, ivec3 _Offset);
// [Generic] gvec4 textureGradOffset(gsampler2DRect sampler, vec2 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureGradOffset(sampler2DRect _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureGradOffset(isampler2DRect _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureGradOffset(usampler2DRect _Sampler, vec2 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] float textureGradOffset(sampler2DRectShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
float textureGradOffset(sampler2DRectShadow _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] float textureGradOffset(sampler1DShadow sampler, vec3 P, float dPdx, float dPdy, int offset);
float textureGradOffset(sampler1DShadow _Sampler, vec3 _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] float textureGradOffset(sampler2DShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
float textureGradOffset(sampler2DShadow _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] gvec4 textureGradOffset(gsampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureGradOffset(sampler2DArray _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureGradOffset(isampler2DArray _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureGradOffset(usampler2DArray _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] gvec4 textureGradOffset(gsampler1DArray sampler, vec2 P, float dPdx, float dPdy, int offset);
vec4 textureGradOffset(sampler1DArray _Sampler, vec2 _P, float _dPdx, float _dPdy, int _Offset);
ivec4 textureGradOffset(isampler1DArray _Sampler, vec2 _P, float _dPdx, float _dPdy, int _Offset);
uvec4 textureGradOffset(usampler1DArray _Sampler, vec2 _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] float textureGradOffset(sampler1DArrayShadow sampler, vec3 P, float dPdx, float dPdy, int offset);
float textureGradOffset(sampler1DArrayShadow _Sampler, vec3 _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] float textureGradOffset(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
float textureGradOffset(sampler2DArrayShadow _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);

// TextureProjGrad
// Do a texture lookup both projectively, as described in textureProj, and with explicit gradient as
// described in textureGrad. The partial derivatives dPdx and dPdy are assumed to be already
// projected.
// [Generic] gvec4 textureProjGrad(gsampler1D sampler, vec2 P, float dPdx, float dPdy);
vec4 textureProjGrad(sampler1D _Sampler, vec2 _P, float _dPdx, float _dPdy);
ivec4 textureProjGrad(isampler1D _Sampler, vec2 _P, float _dPdx, float _dPdy);
uvec4 textureProjGrad(usampler1D _Sampler, vec2 _P, float _dPdx, float _dPdy);
// [Generic] gvec4 textureProjGrad(gsampler1D sampler, vec4 P, float dPdx, float dPdy);
vec4 textureProjGrad(sampler1D _Sampler, vec4 _P, float _dPdx, float _dPdy);
ivec4 textureProjGrad(isampler1D _Sampler, vec4 _P, float _dPdx, float _dPdy);
uvec4 textureProjGrad(usampler1D _Sampler, vec4 _P, float _dPdx, float _dPdy);
// [Generic] gvec4 textureProjGrad(gsampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy);
vec4 textureProjGrad(sampler2D _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureProjGrad(isampler2D _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureProjGrad(usampler2D _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] gvec4 textureProjGrad(gsampler2D sampler, vec4 P, vec2 dPdx, vec2 dPdy);
vec4 textureProjGrad(sampler2D _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureProjGrad(isampler2D _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureProjGrad(usampler2D _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] gvec4 textureProjGrad(gsampler3D sampler, vec4 P, vec3 dPdx, vec3 dPdy);
vec4 textureProjGrad(sampler3D _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);
ivec4 textureProjGrad(isampler3D _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);
uvec4 textureProjGrad(usampler3D _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy);
// [Generic] gvec4 textureProjGrad(gsampler2DRect sampler, vec3 P, vec2 dPdx, vec2 dPdy);
vec4 textureProjGrad(sampler2DRect _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureProjGrad(isampler2DRect _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureProjGrad(usampler2DRect _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] gvec4 textureProjGrad(gsampler2DRect sampler, vec4 P, vec2 dPdx, vec2 dPdy);
vec4 textureProjGrad(sampler2DRect _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
ivec4 textureProjGrad(isampler2DRect _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
uvec4 textureProjGrad(usampler2DRect _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] float textureProjGrad(sampler2DRectShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy);
float textureProjGrad(sampler2DRectShadow _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);
// [Generic] float textureProjGrad(sampler1DShadow sampler, vec4 P, float dPdx, float dPdy);
float textureProjGrad(sampler1DShadow _Sampler, vec4 _P, float _dPdx, float _dPdy);
// [Generic] float textureProjGrad(sampler2DShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy);
float textureProjGrad(sampler2DShadow _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy);

// TextureProjGradOffset
// Do a texture lookup projectively and with explicit gradient as described in textureProjGrad, as
// well as with offset, as described in textureOffset.
// [Generic] gvec4 textureProjGradOffset(gsampler1D sampler, vec2 P, float dPdx, float dPdy, int offset);
vec4 textureProjGradOffset(sampler1D _Sampler, vec2 _P, float _dPdx, float _dPdy, int _Offset);
ivec4 textureProjGradOffset(isampler1D _Sampler, vec2 _P, float _dPdx, float _dPdy, int _Offset);
uvec4 textureProjGradOffset(usampler1D _Sampler, vec2 _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] gvec4 textureProjGradOffset(gsampler1D sampler, vec4 P, float dPdx, float dPdy, int offset);
vec4 textureProjGradOffset(sampler1D _Sampler, vec4 _P, float _dPdx, float _dPdy, int _Offset);
ivec4 textureProjGradOffset(isampler1D _Sampler, vec4 _P, float _dPdx, float _dPdy, int _Offset);
uvec4 textureProjGradOffset(usampler1D _Sampler, vec4 _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] gvec4 textureProjGradOffset(gsampler2D sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureProjGradOffset(sampler2D _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureProjGradOffset(isampler2D _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureProjGradOffset(usampler2D _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] gvec4 textureProjGradOffset(gsampler2D sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureProjGradOffset(sampler2D _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureProjGradOffset(isampler2D _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureProjGradOffset(usampler2D _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] gvec4 textureProjGradOffset(gsampler3D sampler, vec4 P, vec3 dPdx, vec3 dPdy, ivec3 offset);
vec4 textureProjGradOffset(sampler3D _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy, ivec3 _Offset);
ivec4 textureProjGradOffset(isampler3D _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy, ivec3 _Offset);
uvec4 textureProjGradOffset(usampler3D _Sampler, vec4 _P, vec3 _dPdx, vec3 _dPdy, ivec3 _Offset);
// [Generic] gvec4 textureProjGradOffset(gsampler2DRect sampler, vec3 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureProjGradOffset(sampler2DRect _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureProjGradOffset(isampler2DRect _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureProjGradOffset(usampler2DRect _Sampler, vec3 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] gvec4 textureProjGradOffset(gsampler2DRect sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
vec4 textureProjGradOffset(sampler2DRect _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
ivec4 textureProjGradOffset(isampler2DRect _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
uvec4 textureProjGradOffset(usampler2DRect _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] float textureProjGradOffset(sampler2DRectShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
float textureProjGradOffset(sampler2DRectShadow _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);
// [Generic] float textureProjGradOffset(sampler1DShadow sampler, vec4 P, float dPdx, float dPdy, int offset);
float textureProjGradOffset(sampler1DShadow _Sampler, vec4 _P, float _dPdx, float _dPdy, int _Offset);
// [Generic] float textureProjGradOffset(sampler2DShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy, ivec2 offset);
float textureProjGradOffset(sampler2DShadow _Sampler, vec4 _P, vec2 _dPdx, vec2 _dPdy, ivec2 _Offset);

// Texture Gather Functions

// The texture gather functions take components of a single floating-point
// vector operand as a texture coordinate, determine a set of four texels to
// sample from the base level-of-detail of the specified texture image, and
// return one component from each texel in a four-component result vector.

// When performing a texture gather operation, the minification and
// magnification filters are ignored, and the rules for LINEAR filtering in the
// <<references,{apispec}>> are applied to the base level of the texture image
// to identify the four texels _i~0~ j~1~_, _i~1~ j~1~_, _i~1~ j~0~_, and _i~0~
// j~0~_.
// The texels are then converted to texture base colors (_R~s~_, _G~s~_,
// _B~s~_, _A~s~_) according to table 15.1, followed by application of the
// texture swizzle as described in section 15.2.1 "`Texture Access`" of the
// <<references,{apispec}>>.
// A four-component vector is assembled by taking the selected component from
// each of the post-swizzled texture source colors in the order (_i~0~ j~1~_,
// _i~1~ j~1~_, _i~1~ j~0~_, _i~0~ j~0~_).

// The selected component is identified by the optional _comp_ argument, where
// the values zero, one, two, and three identify the _R~s~_, _G~s~_, _B~s~_, or
// _A~s~_ component, respectively.
// If _comp_ is omitted, it is treated as identifying the _R~s~_ component.

// Incomplete textures (see section 8.16 "`Texture Completeness`" of the
// <<references,{apispec}>>) return a texture source color of (0,0,0,1) for all
// four source texels.

// For texture gather functions using a texture-combined shadow sampler type,
// each of the four
// texel lookups perform a depth comparison against the depth reference value
// passed in (_refZ_), and returns the result of that comparison in the
// appropriate component of the result vector.

// As with other texture lookup functions, the results of a texture gather are
// undefined for shadow samplers if the texture referenced is not a depth
// texture or has depth comparisons disabled; or for non-shadow samplers if the
// texture referenced is a depth texture with depth comparisons enabled.

// The *textureGatherOffset* built-in functions from the {slname} return a vector
// derived from sampling four texels in the image array of level _level~base~_.
// For each of the four texel offsets specified by the _offsets_ argument, the
// rules for the LINEAR minification filter are applied to identify a 2 {times}
// 2 texel footprint, from which the single texel T~i0j0~ is selected.
// A four-component vector is then assembled by taking a single component from
// each of the four T~i0j0~ texels in the same manner as for the
// *textureGather* function.

// TextureGather
// Returns the value
// [Generic] gvec4 textureGather(gsampler2D sampler, vec2 P [, int comp]);
vec4 textureGather(sampler2D _Sampler, vec2 _P);
ivec4 textureGather(isampler2D _Sampler, vec2 _P);
uvec4 textureGather(usampler2D _Sampler, vec2 _P);
vec4 textureGather(sampler2D _Sampler, vec2 _P, int _Comp);
ivec4 textureGather(isampler2D _Sampler, vec2 _P, int _Comp);
uvec4 textureGather(usampler2D _Sampler, vec2 _P, int _Comp);
// [Generic] gvec4 textureGather(gsampler2DArray sampler, vec3 P [, int comp]);
vec4 textureGather(sampler2DArray _Sampler, vec3 _P);
ivec4 textureGather(isampler2DArray _Sampler, vec3 _P);
uvec4 textureGather(usampler2DArray _Sampler, vec3 _P);
vec4 textureGather(sampler2DArray _Sampler, vec3 _P, int _Comp);
ivec4 textureGather(isampler2DArray _Sampler, vec3 _P, int _Comp);
uvec4 textureGather(usampler2DArray _Sampler, vec3 _P, int _Comp);
// [Generic] gvec4 textureGather(gsamplerCube sampler, vec3 P [, int comp]);
vec4 textureGather(samplerCube _Sampler, vec3 _P);
ivec4 textureGather(isamplerCube _Sampler, vec3 _P);
uvec4 textureGather(usamplerCube _Sampler, vec3 _P);
vec4 textureGather(samplerCube _Sampler, vec3 _P, int _Comp);
ivec4 textureGather(isamplerCube _Sampler, vec3 _P, int _Comp);
uvec4 textureGather(usamplerCube _Sampler, vec3 _P, int _Comp);
// [Generic] gvec4 textureGather(gsamplerCubeArray sampler, vec4 P [, int comp]);
vec4 textureGather(samplerCubeArray _Sampler, vec4 _P);
ivec4 textureGather(isamplerCubeArray _Sampler, vec4 _P);
uvec4 textureGather(usamplerCubeArray _Sampler, vec4 _P);
vec4 textureGather(samplerCubeArray _Sampler, vec4 _P, int _Comp);
ivec4 textureGather(isamplerCubeArray _Sampler, vec4 _P, int _Comp);
uvec4 textureGather(usamplerCubeArray _Sampler, vec4 _P, int _Comp);
// [Generic] gvec4 textureGather(gsampler2DRect sampler, vec2 P [, int comp]);
vec4 textureGather(sampler2DRect _Sampler, vec2 _P);
ivec4 textureGather(isampler2DRect _Sampler, vec2 _P);
uvec4 textureGather(usampler2DRect _Sampler, vec2 _P);
vec4 textureGather(sampler2DRect _Sampler, vec2 _P, int _Comp);
ivec4 textureGather(isampler2DRect _Sampler, vec2 _P, int _Comp);
uvec4 textureGather(usampler2DRect _Sampler, vec2 _P, int _Comp);
// [Generic] vec4 textureGather(sampler2DShadow sampler, vec2 P, float refZ);
vec4 textureGather(sampler2DShadow _Sampler, vec2 _P, float _RefZ);
// [Generic] vec4 textureGather(sampler2DArrayShadow sampler, vec3 P, float refZ);
vec4 textureGather(sampler2DArrayShadow _Sampler, vec3 _P, float _RefZ);
// [Generic] vec4 textureGather(samplerCubeShadow sampler, vec3 P, float refZ);
vec4 textureGather(samplerCubeShadow _Sampler, vec3 _P, float _RefZ);
// [Generic] vec4 textureGather(samplerCubeArrayShadow sampler, vec4 P, float refZ);
vec4 textureGather(samplerCubeArrayShadow _Sampler, vec4 _P, float _RefZ);
// [Generic] vec4 textureGather(sampler2DRectShadow sampler, vec2 P, float refZ);
vec4 textureGather(sampler2DRectShadow _Sampler, vec2 _P, float _RefZ);

// TextureGatherOffset
// Perform a texture gather operation as in textureGather by offset as described in
// textureOffset except that the offset can be variable (non constant) and the
// implementation-dependent minimum and maximum offset values are given by
// MINPROGRAMTEXTUREGATHEROFFSET and MAXPROGRAMTEXTUREGATHEROFFSET, respectively.
// [Generic] gvec4 textureGatherOffset(gsampler2D sampler, vec2 P, ivec2 offset [, int comp]);
vec4 textureGatherOffset(sampler2D _Sampler, vec2 _P, ivec2 _Offset);
ivec4 textureGatherOffset(isampler2D _Sampler, vec2 _P, ivec2 _Offset);
uvec4 textureGatherOffset(usampler2D _Sampler, vec2 _P, ivec2 _Offset);
vec4 textureGatherOffset(sampler2D _Sampler, vec2 _P, ivec2 _Offset, int _Comp);
ivec4 textureGatherOffset(isampler2D _Sampler, vec2 _P, ivec2 _Offset, int _Comp);
uvec4 textureGatherOffset(usampler2D _Sampler, vec2 _P, ivec2 _Offset, int _Comp);
// [Generic] gvec4 textureGatherOffset(gsampler2DArray sampler, vec3 P, ivec2 offset [, int comp]);
vec4 textureGatherOffset(sampler2DArray _Sampler, vec3 _P, ivec2 _Offset);
ivec4 textureGatherOffset(isampler2DArray _Sampler, vec3 _P, ivec2 _Offset);
uvec4 textureGatherOffset(usampler2DArray _Sampler, vec3 _P, ivec2 _Offset);
vec4 textureGatherOffset(sampler2DArray _Sampler, vec3 _P, ivec2 _Offset, int _Comp);
ivec4 textureGatherOffset(isampler2DArray _Sampler, vec3 _P, ivec2 _Offset, int _Comp);
uvec4 textureGatherOffset(usampler2DArray _Sampler, vec3 _P, ivec2 _Offset, int _Comp);
// [Generic] vec4 textureGatherOffset(sampler2DShadow sampler, vec2 P, float refZ, ivec2 offset);
vec4 textureGatherOffset(sampler2DShadow _Sampler, vec2 _P, float _RefZ, ivec2 _Offset);
// [Generic] vec4 textureGatherOffset(sampler2DArrayShadow sampler, vec3 P, float refZ, ivec2 offset);
vec4 textureGatherOffset(sampler2DArrayShadow _Sampler, vec3 _P, float _RefZ, ivec2 _Offset);
// [Generic] gvec4 textureGatherOffset(gsampler2DRect sampler, vec2 P, ivec2 offset [, int comp]);
vec4 textureGatherOffset(sampler2DRect _Sampler, vec2 _P, ivec2 _Offset);
ivec4 textureGatherOffset(isampler2DRect _Sampler, vec2 _P, ivec2 _Offset);
uvec4 textureGatherOffset(usampler2DRect _Sampler, vec2 _P, ivec2 _Offset);
vec4 textureGatherOffset(sampler2DRect _Sampler, vec2 _P, ivec2 _Offset, int _Comp);
ivec4 textureGatherOffset(isampler2DRect _Sampler, vec2 _P, ivec2 _Offset, int _Comp);
uvec4 textureGatherOffset(usampler2DRect _Sampler, vec2 _P, ivec2 _Offset, int _Comp);
// [Generic] vec4 textureGatherOffset(sampler2DRectShadow sampler, vec2 P, float refZ, ivec2 offset);
vec4 textureGatherOffset(sampler2DRectShadow _Sampler, vec2 _P, float _RefZ, ivec2 _Offset);

// TextureGatherOffsets
// Operate identically to textureGatherOffset except that offsets is used to determine the
// location of the four texels to sample. Each of the four texels is obtained by applying the
// corresponding offset in offsets as a (u, v) coordinate offset to P, identifying the
// four-texel LINEAR footprint, and then selecting the texel i~0~ j~0~ of that footprint. The
// specified values in offsets must be constant integral expressions.
// [Generic] gvec4 textureGatherOffsets(gsampler2D sampler, vec2 P, ivec2 offsets[4] [, int comp]);
vec4 textureGatherOffsets(sampler2D _Sampler, vec2 _P, ivec2 _Offsets[4]);
ivec4 textureGatherOffsets(isampler2D _Sampler, vec2 _P, ivec2 _Offsets[4]);
uvec4 textureGatherOffsets(usampler2D _Sampler, vec2 _P, ivec2 _Offsets[4]);
vec4 textureGatherOffsets(sampler2D _Sampler, vec2 _P, ivec2 _Offsets[4], int _Comp);
ivec4 textureGatherOffsets(isampler2D _Sampler, vec2 _P, ivec2 _Offsets[4], int _Comp);
uvec4 textureGatherOffsets(usampler2D _Sampler, vec2 _P, ivec2 _Offsets[4], int _Comp);
// [Generic] gvec4 textureGatherOffsets(gsampler2DArray sampler, vec3 P, ivec2 offsets[4] [, int comp]);
vec4 textureGatherOffsets(sampler2DArray _Sampler, vec3 _P, ivec2 _Offsets[4]);
ivec4 textureGatherOffsets(isampler2DArray _Sampler, vec3 _P, ivec2 _Offsets[4]);
uvec4 textureGatherOffsets(usampler2DArray _Sampler, vec3 _P, ivec2 _Offsets[4]);
vec4 textureGatherOffsets(sampler2DArray _Sampler, vec3 _P, ivec2 _Offsets[4], int _Comp);
ivec4 textureGatherOffsets(isampler2DArray _Sampler, vec3 _P, ivec2 _Offsets[4], int _Comp);
uvec4 textureGatherOffsets(usampler2DArray _Sampler, vec3 _P, ivec2 _Offsets[4], int _Comp);
// [Generic] vec4 textureGatherOffsets(sampler2DShadow sampler, vec2 P, float refZ, ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DShadow _Sampler, vec2 _P, float _RefZ, ivec2 _Offsets[4]);
// [Generic] vec4 textureGatherOffsets(sampler2DArrayShadow sampler, vec3 P, float refZ, ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DArrayShadow _Sampler, vec3 _P, float _RefZ, ivec2 _Offsets[4]);
// [Generic] gvec4 textureGatherOffsets(gsampler2DRect sampler, vec2 P, ivec2 offsets[4] [, int comp]);
vec4 textureGatherOffsets(sampler2DRect _Sampler, vec2 _P, ivec2 _Offsets[4]);
ivec4 textureGatherOffsets(isampler2DRect _Sampler, vec2 _P, ivec2 _Offsets[4]);
uvec4 textureGatherOffsets(usampler2DRect _Sampler, vec2 _P, ivec2 _Offsets[4]);
vec4 textureGatherOffsets(sampler2DRect _Sampler, vec2 _P, ivec2 _Offsets[4], int _Comp);
ivec4 textureGatherOffsets(isampler2DRect _Sampler, vec2 _P, ivec2 _Offsets[4], int _Comp);
uvec4 textureGatherOffsets(usampler2DRect _Sampler, vec2 _P, ivec2 _Offsets[4], int _Comp);
// [Generic] vec4 textureGatherOffsets(sampler2DRectShadow sampler, vec2 P, float refZ, ivec2 offsets[4]);
vec4 textureGatherOffsets(sampler2DRectShadow _Sampler, vec2 _P, float _RefZ, ivec2 _Offsets[4]);

// Compatibility Profile Texture Functions

// The following texture functions are only in the compatibility profile.

// Texture1D
// See corresponding signature above without "1D" in the name.
// [Generic] vec4 texture1D(sampler1D sampler, float coord [, float bias]);
vec4 texture1D(sampler1D _Sampler, float _Coord);
vec4 texture1D(sampler1D _Sampler, float _Coord, float _Bias);
// [Generic] vec4 texture1DProj(sampler1D sampler, vec2 coord [, float bias]);
vec4 texture1DProj(sampler1D _Sampler, vec2 _Coord);
vec4 texture1DProj(sampler1D _Sampler, vec2 _Coord, float _Bias);
// [Generic] vec4 texture1DProj(sampler1D sampler, vec4 coord [, float bias]);
vec4 texture1DProj(sampler1D _Sampler, vec4 _Coord);
vec4 texture1DProj(sampler1D _Sampler, vec4 _Coord, float _Bias);
// [Generic] vec4 texture1DLod(sampler1D sampler, float coord, float lod);
vec4 texture1DLod(sampler1D _Sampler, float _Coord, float _Lod);
// [Generic] vec4 texture1DProjLod(sampler1D sampler, vec2 coord, float lod);
vec4 texture1DProjLod(sampler1D _Sampler, vec2 _Coord, float _Lod);
// [Generic] vec4 texture1DProjLod(sampler1D sampler, vec4 coord, float lod);
vec4 texture1DProjLod(sampler1D _Sampler, vec4 _Coord, float _Lod);

// Texture2D
// See corresponding signature above without "2D" in the name.
// [Generic] vec4 texture2D(sampler2D sampler, vec2 coord [, float bias]);
vec4 texture2D(sampler2D _Sampler, vec2 _Coord);
vec4 texture2D(sampler2D _Sampler, vec2 _Coord, float _Bias);
// [Generic] vec4 texture2DProj(sampler2D sampler, vec3 coord [, float bias]);
vec4 texture2DProj(sampler2D _Sampler, vec3 _Coord);
vec4 texture2DProj(sampler2D _Sampler, vec3 _Coord, float _Bias);
// [Generic] vec4 texture2DProj(sampler2D sampler, vec4 coord [, float bias]);
vec4 texture2DProj(sampler2D _Sampler, vec4 _Coord);
vec4 texture2DProj(sampler2D _Sampler, vec4 _Coord, float _Bias);
// [Generic] vec4 texture2DLod(sampler2D sampler, vec2 coord, float lod);
vec4 texture2DLod(sampler2D _Sampler, vec2 _Coord, float _Lod);
// [Generic] vec4 texture2DProjLod(sampler2D sampler, vec3 coord, float lod);
vec4 texture2DProjLod(sampler2D _Sampler, vec3 _Coord, float _Lod);
// [Generic] vec4 texture2DProjLod(sampler2D sampler, vec4 coord, float lod);
vec4 texture2DProjLod(sampler2D _Sampler, vec4 _Coord, float _Lod);

// Texture3D
// See corresponding signature above without "3D" in the name. Use the texture coordinate coord
// to do a texture lookup in the 3D texture currently bound to sampler. For the projective
// ("Proj") versions, the texture coordinate is divided by coord.q.
// [Generic] vec4 texture3D(sampler3D sampler, vec3 coord [, float bias]);
vec4 texture3D(sampler3D _Sampler, vec3 _Coord);
vec4 texture3D(sampler3D _Sampler, vec3 _Coord, float _Bias);
// [Generic] vec4 texture3DProj(sampler3D sampler, vec4 coord [, float bias]);
vec4 texture3DProj(sampler3D _Sampler, vec4 _Coord);
vec4 texture3DProj(sampler3D _Sampler, vec4 _Coord, float _Bias);
// [Generic] vec4 texture3DLod(sampler3D sampler, vec3 coord, float lod);
vec4 texture3DLod(sampler3D _Sampler, vec3 _Coord, float _Lod);
// [Generic] vec4 texture3DProjLod(sampler3D sampler, vec4 coord, float lod);
vec4 texture3DProjLod(sampler3D _Sampler, vec4 _Coord, float _Lod);

// TextureCube
// See corresponding signature above without "Cube" in the name.
// [Generic] vec4 textureCube(samplerCube sampler, vec3 coord [, float bias]);
vec4 textureCube(samplerCube _Sampler, vec3 _Coord);
vec4 textureCube(samplerCube _Sampler, vec3 _Coord, float _Bias);
// [Generic] vec4 textureCubeLod(samplerCube sampler, vec3 coord, float lod);
vec4 textureCubeLod(samplerCube _Sampler, vec3 _Coord, float _Lod);

// Shadow
// Same functionality as the "texture" based names above with the same signature.
// [Generic] vec4 shadow1D(sampler1DShadow sampler, vec3 coord [, float bias]);
vec4 shadow1D(sampler1DShadow _Sampler, vec3 _Coord);
vec4 shadow1D(sampler1DShadow _Sampler, vec3 _Coord, float _Bias);
// [Generic] vec4 shadow2D(sampler2DShadow sampler, vec3 coord [, float bias]);
vec4 shadow2D(sampler2DShadow _Sampler, vec3 _Coord);
vec4 shadow2D(sampler2DShadow _Sampler, vec3 _Coord, float _Bias);
// [Generic] vec4 shadow1DProj(sampler1DShadow sampler, vec4 coord [, float bias]);
vec4 shadow1DProj(sampler1DShadow _Sampler, vec4 _Coord);
vec4 shadow1DProj(sampler1DShadow _Sampler, vec4 _Coord, float _Bias);
// [Generic] vec4 shadow2DProj(sampler2DShadow sampler, vec4 coord [, float bias]);
vec4 shadow2DProj(sampler2DShadow _Sampler, vec4 _Coord);
vec4 shadow2DProj(sampler2DShadow _Sampler, vec4 _Coord, float _Bias);
// [Generic] vec4 shadow1DLod(sampler1DShadow sampler, vec3 coord, float lod);
vec4 shadow1DLod(sampler1DShadow _Sampler, vec3 _Coord, float _Lod);
// [Generic] vec4 shadow2DLod(sampler2DShadow sampler, vec3 coord, float lod);
vec4 shadow2DLod(sampler2DShadow _Sampler, vec3 _Coord, float _Lod);
// [Generic] vec4 shadow1DProjLod(sampler1DShadow sampler, vec4 coord, float lod);
vec4 shadow1DProjLod(sampler1DShadow _Sampler, vec4 _Coord, float _Lod);
// [Generic] vec4 shadow2DProjLod(sampler2DShadow sampler, vec4 coord, float lod);
vec4 shadow2DProjLod(sampler2DShadow _Sampler, vec4 _Coord, float _Lod);

// AtomicCounterIncrement
// Atomically
// . increments the counter for c, and
// . returns its value prior to the increment operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterIncrement(atomic_uint c);
uint atomicCounterIncrement(atomic_uint _C);

// AtomicCounterDecrement
// Atomically
// . decrements the counter for c, and
// . returns the value resulting from the decrement operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterDecrement(atomic_uint c);
uint atomicCounterDecrement(atomic_uint _C);

// AtomicCounter
// Returns the counter value for c.
// [Generic] uint atomicCounter(atomic_uint c);
uint atomicCounter(atomic_uint _C);

// AtomicCounterAdd
// Atomically
// . adds the value of data to the counter for c, and
// . returns its value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterAdd(atomic_uint c, uint data);
uint atomicCounterAdd(atomic_uint _C, uint _Data);

// AtomicCounterSubtract
// Atomically
// . subtracts the value of data from the counter for c, and
// . returns its value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterSubtract(atomic_uint c, uint data);
uint atomicCounterSubtract(atomic_uint _C, uint _Data);

// AtomicCounterMin
// Atomically
// . sets the counter for c to the minimum of the value of the counter and
// the value of data, and
// . returns the value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterMin(atomic_uint c, uint data);
uint atomicCounterMin(atomic_uint _C, uint _Data);

// AtomicCounterMax
// Atomically
// . sets the counter for c to the maximum of the value of the counter and
// the value of data, and
// . returns the value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterMax(atomic_uint c, uint data);
uint atomicCounterMax(atomic_uint _C, uint _Data);

// AtomicCounterAnd
// Atomically
// . sets the counter for c to the bitwise AND of the value of the counter
// and the value of data, and
// . returns the value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterAnd(atomic_uint c, uint data);
uint atomicCounterAnd(atomic_uint _C, uint _Data);

// AtomicCounterOr
// Atomically
// . sets the counter for c to the bitwise OR of the value of the counter
// and the value of data, and
// . returns the value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterOr(atomic_uint c, uint data);
uint atomicCounterOr(atomic_uint _C, uint _Data);

// AtomicCounterXor
// Atomically
// . sets the counter for c to the bitwise XOR of the value of the counter
// and the value of data, and
// . returns the value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterXor(atomic_uint c, uint data);
uint atomicCounterXor(atomic_uint _C, uint _Data);

// AtomicCounterExchange
// Atomically
// . sets the counter value for c to the value of data, and
// . returns its value prior to the operation.
// These two steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterExchange(atomic_uint c, uint data);
uint atomicCounterExchange(atomic_uint _C, uint _Data);

// AtomicCounterCompSwap
// Atomically
// . compares the value of compare and the counter value for c
// . if the values are equal, sets the counter value for c to the value of
// data, and
// . returns its value prior to the operation.
// These three steps are done atomically with respect to the atomic counter
// functions in this table.
// [Generic] uint atomicCounterCompSwap(atomic_uint c, uint compare, uint data);
uint atomicCounterCompSwap(atomic_uint _C, uint _Compare, uint _Data);

// AtomicAdd
// Computes a new value by adding the value of data to the contents mem.
// [Generic] uint atomicAdd(inout uint mem, uint data);
uint atomicAdd(inout uint _Mem, uint _Data);
// [Generic] int atomicAdd(inout int mem, int data);
int atomicAdd(inout int _Mem, int _Data);

// AtomicMin
// Computes a new value by taking the minimum of the value of data and the contents of mem.
// [Generic] uint atomicMin(inout uint mem, uint data);
uint atomicMin(inout uint _Mem, uint _Data);
// [Generic] int atomicMin(inout int mem, int data);
int atomicMin(inout int _Mem, int _Data);

// AtomicMax
// Computes a new value by taking the maximum of the value of data and the contents of mem.
// [Generic] uint atomicMax(inout uint mem, uint data);
uint atomicMax(inout uint _Mem, uint _Data);
// [Generic] int atomicMax(inout int mem, int data);
int atomicMax(inout int _Mem, int _Data);

// AtomicAnd
// Computes a new value by performing a bit-wise AND of the value of data and the contents of mem.
// [Generic] uint atomicAnd(inout uint mem, uint data);
uint atomicAnd(inout uint _Mem, uint _Data);
// [Generic] int atomicAnd(inout int mem, int data);
int atomicAnd(inout int _Mem, int _Data);

// AtomicOr
// Computes a new value by performing a bit-wise OR of the value of data and the contents of mem.
// [Generic] uint atomicOr(inout uint mem, uint data);
uint atomicOr(inout uint _Mem, uint _Data);
// [Generic] int atomicOr(inout int mem, int data);
int atomicOr(inout int _Mem, int _Data);

// AtomicXor
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of data and the contents
// of mem.
// [Generic] uint atomicXor(inout uint mem, uint data);
uint atomicXor(inout uint _Mem, uint _Data);
// [Generic] int atomicXor(inout int mem, int data);
int atomicXor(inout int _Mem, int _Data);

// AtomicExchange
// Computes a new value by simply copying the value of data.
// [Generic] uint atomicExchange(inout uint mem, uint data);
uint atomicExchange(inout uint _Mem, uint _Data);
// [Generic] int atomicExchange(inout int mem, int data);
int atomicExchange(inout int _Mem, int _Data);

// AtomicCompSwap
// Compares the value of compare and the contents of mem. If the values are equal, the new value
// is given by data; otherwise, it is taken from the original contents of mem.
// [Generic] uint atomicCompSwap(inout uint mem, uint compare, uint data);
uint atomicCompSwap(inout uint _Mem, uint _Compare, uint _Data);
// [Generic] int atomicCompSwap(inout int mem, int compare, int data);
int atomicCompSwap(inout int _Mem, int _Compare, int _Data);

// ImageSize
// Returns the dimensions of the image bound to image. For arrayed images, the last component of the
// return value will hold the size of the array. Cube images only return the dimensions of one face,
// and the number of cubes in the cube map array, if arrayed.
// Note: The qualification readonly writeonly accepts a variable qualified with readonly,
// writeonly, both, or neither. It means the formal argument will be used for neither reading
// nor writing to the underlying memory.
// [Generic] int imageSize(readonly writeonly gimage1D image);
int imageSize(readonly image1D _Image);
int imageSize(readonly iimage1D _Image);
int imageSize(readonly uimage1D _Image);
// [Generic] ivec2 imageSize(readonly writeonly gimage2D image);
ivec2 imageSize(readonly image2D _Image);
ivec2 imageSize(readonly iimage2D _Image);
ivec2 imageSize(readonly uimage2D _Image);
// [Generic] ivec3 imageSize(readonly writeonly gimage3D image);
ivec3 imageSize(readonly image3D _Image);
ivec3 imageSize(readonly iimage3D _Image);
ivec3 imageSize(readonly uimage3D _Image);
// [Generic] ivec2 imageSize(readonly writeonly gimageCube image);
ivec2 imageSize(readonly imageCube _Image);
ivec2 imageSize(readonly iimageCube _Image);
ivec2 imageSize(readonly uimageCube _Image);
// [Generic] ivec3 imageSize(readonly writeonly gimageCubeArray image);
ivec3 imageSize(readonly imageCubeArray _Image);
ivec3 imageSize(readonly iimageCubeArray _Image);
ivec3 imageSize(readonly uimageCubeArray _Image);
// [Generic] ivec3 imageSize(readonly writeonly gimage2DArray image);
ivec3 imageSize(readonly image2DArray _Image);
ivec3 imageSize(readonly iimage2DArray _Image);
ivec3 imageSize(readonly uimage2DArray _Image);
// [Generic] ivec2 imageSize(readonly writeonly gimage2DRect image);
ivec2 imageSize(readonly image2DRect _Image);
ivec2 imageSize(readonly iimage2DRect _Image);
ivec2 imageSize(readonly uimage2DRect _Image);
// [Generic] ivec2 imageSize(readonly writeonly gimage1DArray image);
ivec2 imageSize(readonly image1DArray _Image);
ivec2 imageSize(readonly iimage1DArray _Image);
ivec2 imageSize(readonly uimage1DArray _Image);
// [Generic] ivec2 imageSize(readonly writeonly gimage2DMS image);
ivec2 imageSize(readonly image2DMS _Image);
ivec2 imageSize(readonly iimage2DMS _Image);
ivec2 imageSize(readonly uimage2DMS _Image);
// [Generic] ivec3 imageSize(readonly writeonly gimage2DMSArray image);
ivec3 imageSize(readonly image2DMSArray _Image);
ivec3 imageSize(readonly iimage2DMSArray _Image);
ivec3 imageSize(readonly uimage2DMSArray _Image);
// [Generic] int imageSize(readonly writeonly gimageBuffer image);
int imageSize(readonly imageBuffer _Image);
int imageSize(readonly iimageBuffer _Image);
int imageSize(readonly uimageBuffer _Image);

// ImageSamples
// Returns the number of samples of the image bound to image.
// [Generic] int imageSamples(readonly writeonly gimage2DMS image);
int imageSamples(readonly image2DMS _Image);
int imageSamples(readonly iimage2DMS _Image);
int imageSamples(readonly uimage2DMS _Image);
// [Generic] int imageSamples(readonly writeonly gimage2DMSArray image);
int imageSamples(readonly image2DMSArray _Image);
int imageSamples(readonly iimage2DMSArray _Image);
int imageSamples(readonly uimage2DMSArray _Image);

// ImageLoad
// Loads the texel at the coordinate P from the image unit image (in IMAGEPARAMS_).
// For multisample loads, the sample number is given by sample. When image, P, and sample
// When image and P
// identify a valid texel, the bits used to represent the selected texel in memory are converted to a
// vec4, ivec4, or uvec4 in the manner described in section 8.26
// "Texture Image Loads and Stores" of the <<references,{apispec}>> and returned.
// [Generic] gvec4 imageLoad(readonly IMAGE_PARAMS);
vec4 imageLoad(readonly image2D _Image, ivec2 _P);
ivec4 imageLoad(readonly iimage2D _Image, ivec2 _P);
uvec4 imageLoad(readonly uimage2D _Image, ivec2 _P);
vec4 imageLoad(readonly image3D _Image, ivec3 _P);
ivec4 imageLoad(readonly iimage3D _Image, ivec3 _P);
uvec4 imageLoad(readonly uimage3D _Image, ivec3 _P);
vec4 imageLoad(readonly imageCube _Image, ivec3 _P);
ivec4 imageLoad(readonly iimageCube _Image, ivec3 _P);
uvec4 imageLoad(readonly uimageCube _Image, ivec3 _P);
vec4 imageLoad(readonly imageBuffer _Image, int _P);
ivec4 imageLoad(readonly iimageBuffer _Image, int _P);
uvec4 imageLoad(readonly uimageBuffer _Image, int _P);
vec4 imageLoad(readonly image2DArray _Image, ivec3 _P);
ivec4 imageLoad(readonly iimage2DArray _Image, ivec3 _P);
uvec4 imageLoad(readonly uimage2DArray _Image, ivec3 _P);
vec4 imageLoad(readonly imageCubeArray _Image, ivec3 _P);
ivec4 imageLoad(readonly iimageCubeArray _Image, ivec3 _P);
uvec4 imageLoad(readonly uimageCubeArray _Image, ivec3 _P);
vec4 imageLoad(readonly image1D _Image, int _P);
ivec4 imageLoad(readonly iimage1D _Image, int _P);
uvec4 imageLoad(readonly uimage1D _Image, int _P);
vec4 imageLoad(readonly image1DArray _Image, ivec2 _P);
ivec4 imageLoad(readonly iimage1DArray _Image, ivec2 _P);
uvec4 imageLoad(readonly uimage1DArray _Image, ivec2 _P);
vec4 imageLoad(readonly image2DRect _Image, ivec2 _P);
ivec4 imageLoad(readonly iimage2DRect _Image, ivec2 _P);
uvec4 imageLoad(readonly uimage2DRect _Image, ivec2 _P);
vec4 imageLoad(readonly image2DMS _Image, ivec2 _P, int _Sample);
ivec4 imageLoad(readonly iimage2DMS _Image, ivec2 _P, int _Sample);
uvec4 imageLoad(readonly uimage2DMS _Image, ivec2 _P, int _Sample);
vec4 imageLoad(readonly image2DMSArray _Image, ivec3 _P, int _Sample);
ivec4 imageLoad(readonly iimage2DMSArray _Image, ivec3 _P, int _Sample);
uvec4 imageLoad(readonly uimage2DMSArray _Image, ivec3 _P, int _Sample);

// ImageStore
// Stores data into the texel at the coordinate P from the image specified by image.
// For multisample stores, the sample number is given by sample. When image, P, and sample
// When image and P
// identify a valid texel, the bits used to represent data are converted to the format of the image
// unit in the manner described in section 8.26
// "Texture Image Loads and Stores" of the <<references,{apispec}>> and stored to the specified texel.
// [Generic] void imageStore(writeonly IMAGE_PARAMS, gvec4 data);
void imageStore(writeonly image2D _Image, ivec2 _P, vec4 _Data);
void imageStore(writeonly iimage2D _Image, ivec2 _P, ivec4 _Data);
void imageStore(writeonly uimage2D _Image, ivec2 _P, uvec4 _Data);
void imageStore(writeonly image3D _Image, ivec3 _P, vec4 _Data);
void imageStore(writeonly iimage3D _Image, ivec3 _P, ivec4 _Data);
void imageStore(writeonly uimage3D _Image, ivec3 _P, uvec4 _Data);
void imageStore(writeonly imageCube _Image, ivec3 _P, vec4 _Data);
void imageStore(writeonly iimageCube _Image, ivec3 _P, ivec4 _Data);
void imageStore(writeonly uimageCube _Image, ivec3 _P, uvec4 _Data);
void imageStore(writeonly imageBuffer _Image, int _P, vec4 _Data);
void imageStore(writeonly iimageBuffer _Image, int _P, ivec4 _Data);
void imageStore(writeonly uimageBuffer _Image, int _P, uvec4 _Data);
void imageStore(writeonly image2DArray _Image, ivec3 _P, vec4 _Data);
void imageStore(writeonly iimage2DArray _Image, ivec3 _P, ivec4 _Data);
void imageStore(writeonly uimage2DArray _Image, ivec3 _P, uvec4 _Data);
void imageStore(writeonly imageCubeArray _Image, ivec3 _P, vec4 _Data);
void imageStore(writeonly iimageCubeArray _Image, ivec3 _P, ivec4 _Data);
void imageStore(writeonly uimageCubeArray _Image, ivec3 _P, uvec4 _Data);
void imageStore(writeonly image1D _Image, int _P, vec4 _Data);
void imageStore(writeonly iimage1D _Image, int _P, ivec4 _Data);
void imageStore(writeonly uimage1D _Image, int _P, uvec4 _Data);
void imageStore(writeonly image1DArray _Image, ivec2 _P, vec4 _Data);
void imageStore(writeonly iimage1DArray _Image, ivec2 _P, ivec4 _Data);
void imageStore(writeonly uimage1DArray _Image, ivec2 _P, uvec4 _Data);
void imageStore(writeonly image2DRect _Image, ivec2 _P, vec4 _Data);
void imageStore(writeonly iimage2DRect _Image, ivec2 _P, ivec4 _Data);
void imageStore(writeonly uimage2DRect _Image, ivec2 _P, uvec4 _Data);
void imageStore(writeonly image2DMS _Image, ivec2 _P, int _Sample, vec4 _Data);
void imageStore(writeonly iimage2DMS _Image, ivec2 _P, int _Sample, ivec4 _Data);
void imageStore(writeonly uimage2DMS _Image, ivec2 _P, int _Sample, uvec4 _Data);
void imageStore(writeonly image2DMSArray _Image, ivec3 _P, int _Sample, vec4 _Data);
void imageStore(writeonly iimage2DMSArray _Image, ivec3 _P, int _Sample, ivec4 _Data);
void imageStore(writeonly uimage2DMSArray _Image, ivec3 _P, int _Sample, uvec4 _Data);

// ImageAtomicAdd
// Computes a new value by adding the value of data to the contents of the selected texel.
// [Generic] uint imageAtomicAdd(IMAGE_PARAMS, uint data);
uint imageAtomicAdd(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicAdd(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicAdd(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicAdd(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAdd(image1D _Image, int _P, uint _Data);
uint imageAtomicAdd(iimage1D _Image, int _P, uint _Data);
uint imageAtomicAdd(uimage1D _Image, int _P, uint _Data);
uint imageAtomicAdd(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicAdd(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicAdd(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicAdd(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicAdd(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicAdd(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicAdd(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicAdd(IMAGE_PARAMS, int data);
int imageAtomicAdd(image2D _Image, ivec2 _P, int _Data);
int imageAtomicAdd(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicAdd(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicAdd(image3D _Image, ivec3 _P, int _Data);
int imageAtomicAdd(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicAdd(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicAdd(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicAdd(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicAdd(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicAdd(imageBuffer _Image, int _P, int _Data);
int imageAtomicAdd(iimageBuffer _Image, int _P, int _Data);
int imageAtomicAdd(uimageBuffer _Image, int _P, int _Data);
int imageAtomicAdd(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicAdd(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicAdd(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicAdd(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicAdd(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicAdd(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicAdd(image1D _Image, int _P, int _Data);
int imageAtomicAdd(iimage1D _Image, int _P, int _Data);
int imageAtomicAdd(uimage1D _Image, int _P, int _Data);
int imageAtomicAdd(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicAdd(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicAdd(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicAdd(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicAdd(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicAdd(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicAdd(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicAdd(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicAdd(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicAdd(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicAdd(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicAdd(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);

// ImageAtomicMin
// Computes a new value by taking the minimum of the value of data and the contents of the selected texel.
// [Generic] uint imageAtomicMin(IMAGE_PARAMS, uint data);
uint imageAtomicMin(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicMin(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicMin(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicMin(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMin(image1D _Image, int _P, uint _Data);
uint imageAtomicMin(iimage1D _Image, int _P, uint _Data);
uint imageAtomicMin(uimage1D _Image, int _P, uint _Data);
uint imageAtomicMin(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicMin(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicMin(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicMin(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicMin(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicMin(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicMin(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicMin(IMAGE_PARAMS, int data);
int imageAtomicMin(image2D _Image, ivec2 _P, int _Data);
int imageAtomicMin(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicMin(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicMin(image3D _Image, ivec3 _P, int _Data);
int imageAtomicMin(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicMin(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicMin(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicMin(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicMin(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicMin(imageBuffer _Image, int _P, int _Data);
int imageAtomicMin(iimageBuffer _Image, int _P, int _Data);
int imageAtomicMin(uimageBuffer _Image, int _P, int _Data);
int imageAtomicMin(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicMin(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicMin(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicMin(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicMin(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicMin(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicMin(image1D _Image, int _P, int _Data);
int imageAtomicMin(iimage1D _Image, int _P, int _Data);
int imageAtomicMin(uimage1D _Image, int _P, int _Data);
int imageAtomicMin(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicMin(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicMin(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicMin(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicMin(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicMin(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicMin(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicMin(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicMin(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicMin(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicMin(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicMin(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);

// ImageAtomicMax
// Computes a new value by taking the maximum of the value data and the contents of the selected texel.
// [Generic] uint imageAtomicMax(IMAGE_PARAMS, uint data);
uint imageAtomicMax(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicMax(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicMax(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicMax(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicMax(image1D _Image, int _P, uint _Data);
uint imageAtomicMax(iimage1D _Image, int _P, uint _Data);
uint imageAtomicMax(uimage1D _Image, int _P, uint _Data);
uint imageAtomicMax(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicMax(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicMax(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicMax(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicMax(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicMax(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicMax(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicMax(IMAGE_PARAMS, int data);
int imageAtomicMax(image2D _Image, ivec2 _P, int _Data);
int imageAtomicMax(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicMax(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicMax(image3D _Image, ivec3 _P, int _Data);
int imageAtomicMax(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicMax(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicMax(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicMax(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicMax(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicMax(imageBuffer _Image, int _P, int _Data);
int imageAtomicMax(iimageBuffer _Image, int _P, int _Data);
int imageAtomicMax(uimageBuffer _Image, int _P, int _Data);
int imageAtomicMax(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicMax(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicMax(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicMax(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicMax(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicMax(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicMax(image1D _Image, int _P, int _Data);
int imageAtomicMax(iimage1D _Image, int _P, int _Data);
int imageAtomicMax(uimage1D _Image, int _P, int _Data);
int imageAtomicMax(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicMax(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicMax(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicMax(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicMax(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicMax(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicMax(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicMax(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicMax(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicMax(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicMax(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicMax(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);

// ImageAtomicAnd
// Computes a new value by performing a bit-wise AND of the value of data and the contents of the selected texel.
// [Generic] uint imageAtomicAnd(IMAGE_PARAMS, uint data);
uint imageAtomicAnd(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicAnd(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicAnd(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicAnd(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicAnd(image1D _Image, int _P, uint _Data);
uint imageAtomicAnd(iimage1D _Image, int _P, uint _Data);
uint imageAtomicAnd(uimage1D _Image, int _P, uint _Data);
uint imageAtomicAnd(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicAnd(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicAnd(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicAnd(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicAnd(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicAnd(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicAnd(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicAnd(IMAGE_PARAMS, int data);
int imageAtomicAnd(image2D _Image, ivec2 _P, int _Data);
int imageAtomicAnd(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicAnd(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicAnd(image3D _Image, ivec3 _P, int _Data);
int imageAtomicAnd(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicAnd(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicAnd(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicAnd(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicAnd(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicAnd(imageBuffer _Image, int _P, int _Data);
int imageAtomicAnd(iimageBuffer _Image, int _P, int _Data);
int imageAtomicAnd(uimageBuffer _Image, int _P, int _Data);
int imageAtomicAnd(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicAnd(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicAnd(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicAnd(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicAnd(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicAnd(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicAnd(image1D _Image, int _P, int _Data);
int imageAtomicAnd(iimage1D _Image, int _P, int _Data);
int imageAtomicAnd(uimage1D _Image, int _P, int _Data);
int imageAtomicAnd(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicAnd(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicAnd(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicAnd(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicAnd(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicAnd(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicAnd(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicAnd(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicAnd(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicAnd(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicAnd(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicAnd(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);

// ImageAtomicOr
// Computes a new value by performing a bit-wise OR of the value of data and the contents of the selected texel.
// [Generic] uint imageAtomicOr(IMAGE_PARAMS, uint data);
uint imageAtomicOr(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicOr(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicOr(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicOr(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicOr(image1D _Image, int _P, uint _Data);
uint imageAtomicOr(iimage1D _Image, int _P, uint _Data);
uint imageAtomicOr(uimage1D _Image, int _P, uint _Data);
uint imageAtomicOr(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicOr(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicOr(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicOr(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicOr(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicOr(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicOr(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicOr(IMAGE_PARAMS, int data);
int imageAtomicOr(image2D _Image, ivec2 _P, int _Data);
int imageAtomicOr(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicOr(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicOr(image3D _Image, ivec3 _P, int _Data);
int imageAtomicOr(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicOr(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicOr(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicOr(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicOr(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicOr(imageBuffer _Image, int _P, int _Data);
int imageAtomicOr(iimageBuffer _Image, int _P, int _Data);
int imageAtomicOr(uimageBuffer _Image, int _P, int _Data);
int imageAtomicOr(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicOr(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicOr(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicOr(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicOr(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicOr(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicOr(image1D _Image, int _P, int _Data);
int imageAtomicOr(iimage1D _Image, int _P, int _Data);
int imageAtomicOr(uimage1D _Image, int _P, int _Data);
int imageAtomicOr(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicOr(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicOr(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicOr(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicOr(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicOr(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicOr(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicOr(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicOr(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicOr(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicOr(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicOr(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);

// ImageAtomicXor
// Computes a new value by performing a bit-wise EXCLUSIVE OR of the value of data and the contents
// of the selected texel.
// [Generic] uint imageAtomicXor(IMAGE_PARAMS, uint data);
uint imageAtomicXor(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicXor(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicXor(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicXor(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicXor(image1D _Image, int _P, uint _Data);
uint imageAtomicXor(iimage1D _Image, int _P, uint _Data);
uint imageAtomicXor(uimage1D _Image, int _P, uint _Data);
uint imageAtomicXor(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicXor(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicXor(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicXor(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicXor(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicXor(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicXor(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicXor(IMAGE_PARAMS, int data);
int imageAtomicXor(image2D _Image, ivec2 _P, int _Data);
int imageAtomicXor(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicXor(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicXor(image3D _Image, ivec3 _P, int _Data);
int imageAtomicXor(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicXor(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicXor(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicXor(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicXor(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicXor(imageBuffer _Image, int _P, int _Data);
int imageAtomicXor(iimageBuffer _Image, int _P, int _Data);
int imageAtomicXor(uimageBuffer _Image, int _P, int _Data);
int imageAtomicXor(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicXor(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicXor(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicXor(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicXor(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicXor(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicXor(image1D _Image, int _P, int _Data);
int imageAtomicXor(iimage1D _Image, int _P, int _Data);
int imageAtomicXor(uimage1D _Image, int _P, int _Data);
int imageAtomicXor(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicXor(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicXor(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicXor(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicXor(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicXor(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicXor(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicXor(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicXor(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicXor(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicXor(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicXor(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);

// ImageAtomicExchange
// Computes a new value by simply copying the value of data.
// [Generic] uint imageAtomicExchange(IMAGE_PARAMS, uint data);
uint imageAtomicExchange(image2D _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(iimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(uimage2D _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(image3D _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(iimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(uimage3D _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(imageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(iimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(uimageCube _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(imageBuffer _Image, int _P, uint _Data);
uint imageAtomicExchange(iimageBuffer _Image, int _P, uint _Data);
uint imageAtomicExchange(uimageBuffer _Image, int _P, uint _Data);
uint imageAtomicExchange(image2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(iimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(uimage2DArray _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(imageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(iimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(uimageCubeArray _Image, ivec3 _P, uint _Data);
uint imageAtomicExchange(image1D _Image, int _P, uint _Data);
uint imageAtomicExchange(iimage1D _Image, int _P, uint _Data);
uint imageAtomicExchange(uimage1D _Image, int _P, uint _Data);
uint imageAtomicExchange(image1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(iimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(uimage1DArray _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(image2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(iimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(uimage2DRect _Image, ivec2 _P, uint _Data);
uint imageAtomicExchange(image2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicExchange(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicExchange(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Data);
uint imageAtomicExchange(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicExchange(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
uint imageAtomicExchange(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Data);
// [Generic] int imageAtomicExchange(IMAGE_PARAMS, int data);
int imageAtomicExchange(image2D _Image, ivec2 _P, int _Data);
int imageAtomicExchange(iimage2D _Image, ivec2 _P, int _Data);
int imageAtomicExchange(uimage2D _Image, ivec2 _P, int _Data);
int imageAtomicExchange(image3D _Image, ivec3 _P, int _Data);
int imageAtomicExchange(iimage3D _Image, ivec3 _P, int _Data);
int imageAtomicExchange(uimage3D _Image, ivec3 _P, int _Data);
int imageAtomicExchange(imageCube _Image, ivec3 _P, int _Data);
int imageAtomicExchange(iimageCube _Image, ivec3 _P, int _Data);
int imageAtomicExchange(uimageCube _Image, ivec3 _P, int _Data);
int imageAtomicExchange(imageBuffer _Image, int _P, int _Data);
int imageAtomicExchange(iimageBuffer _Image, int _P, int _Data);
int imageAtomicExchange(uimageBuffer _Image, int _P, int _Data);
int imageAtomicExchange(image2DArray _Image, ivec3 _P, int _Data);
int imageAtomicExchange(iimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicExchange(uimage2DArray _Image, ivec3 _P, int _Data);
int imageAtomicExchange(imageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicExchange(iimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicExchange(uimageCubeArray _Image, ivec3 _P, int _Data);
int imageAtomicExchange(image1D _Image, int _P, int _Data);
int imageAtomicExchange(iimage1D _Image, int _P, int _Data);
int imageAtomicExchange(uimage1D _Image, int _P, int _Data);
int imageAtomicExchange(image1DArray _Image, ivec2 _P, int _Data);
int imageAtomicExchange(iimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicExchange(uimage1DArray _Image, ivec2 _P, int _Data);
int imageAtomicExchange(image2DRect _Image, ivec2 _P, int _Data);
int imageAtomicExchange(iimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicExchange(uimage2DRect _Image, ivec2 _P, int _Data);
int imageAtomicExchange(image2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicExchange(iimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicExchange(uimage2DMS _Image, ivec2 _P, int _Sample, int _Data);
int imageAtomicExchange(image2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicExchange(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
int imageAtomicExchange(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Data);
// [Generic] float imageAtomicExchange(IMAGE_PARAMS, float data);
float imageAtomicExchange(image2D _Image, ivec2 _P, float _Data);
float imageAtomicExchange(iimage2D _Image, ivec2 _P, float _Data);
float imageAtomicExchange(uimage2D _Image, ivec2 _P, float _Data);
float imageAtomicExchange(image3D _Image, ivec3 _P, float _Data);
float imageAtomicExchange(iimage3D _Image, ivec3 _P, float _Data);
float imageAtomicExchange(uimage3D _Image, ivec3 _P, float _Data);
float imageAtomicExchange(imageCube _Image, ivec3 _P, float _Data);
float imageAtomicExchange(iimageCube _Image, ivec3 _P, float _Data);
float imageAtomicExchange(uimageCube _Image, ivec3 _P, float _Data);
float imageAtomicExchange(imageBuffer _Image, int _P, float _Data);
float imageAtomicExchange(iimageBuffer _Image, int _P, float _Data);
float imageAtomicExchange(uimageBuffer _Image, int _P, float _Data);
float imageAtomicExchange(image2DArray _Image, ivec3 _P, float _Data);
float imageAtomicExchange(iimage2DArray _Image, ivec3 _P, float _Data);
float imageAtomicExchange(uimage2DArray _Image, ivec3 _P, float _Data);
float imageAtomicExchange(imageCubeArray _Image, ivec3 _P, float _Data);
float imageAtomicExchange(iimageCubeArray _Image, ivec3 _P, float _Data);
float imageAtomicExchange(uimageCubeArray _Image, ivec3 _P, float _Data);
float imageAtomicExchange(image1D _Image, int _P, float _Data);
float imageAtomicExchange(iimage1D _Image, int _P, float _Data);
float imageAtomicExchange(uimage1D _Image, int _P, float _Data);
float imageAtomicExchange(image1DArray _Image, ivec2 _P, float _Data);
float imageAtomicExchange(iimage1DArray _Image, ivec2 _P, float _Data);
float imageAtomicExchange(uimage1DArray _Image, ivec2 _P, float _Data);
float imageAtomicExchange(image2DRect _Image, ivec2 _P, float _Data);
float imageAtomicExchange(iimage2DRect _Image, ivec2 _P, float _Data);
float imageAtomicExchange(uimage2DRect _Image, ivec2 _P, float _Data);
float imageAtomicExchange(image2DMS _Image, ivec2 _P, int _Sample, float _Data);
float imageAtomicExchange(iimage2DMS _Image, ivec2 _P, int _Sample, float _Data);
float imageAtomicExchange(uimage2DMS _Image, ivec2 _P, int _Sample, float _Data);
float imageAtomicExchange(image2DMSArray _Image, ivec3 _P, int _Sample, float _Data);
float imageAtomicExchange(iimage2DMSArray _Image, ivec3 _P, int _Sample, float _Data);
float imageAtomicExchange(uimage2DMSArray _Image, ivec3 _P, int _Sample, float _Data);

// ImageAtomicCompSwap
// Compares the value of compare and the contents of the selected texel. If the values are equal,
// the new value is given by data; otherwise, it is taken from the original value loaded from the texel.
// [Generic] uint imageAtomicCompSwap(IMAGE_PARAMS, uint compare, uint data);
uint imageAtomicCompSwap(image2D _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage2D _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage2D _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image3D _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage3D _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage3D _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(imageCube _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimageCube _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimageCube _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(imageBuffer _Image, int _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimageBuffer _Image, int _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimageBuffer _Image, int _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image2DArray _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage2DArray _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage2DArray _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(imageCubeArray _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimageCubeArray _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimageCubeArray _Image, ivec3 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image1D _Image, int _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage1D _Image, int _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage1D _Image, int _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image1DArray _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage1DArray _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage1DArray _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image2DRect _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage2DRect _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage2DRect _Image, ivec2 _P, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image2DMS _Image, ivec2 _P, int _Sample, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage2DMS _Image, ivec2 _P, int _Sample, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage2DMS _Image, ivec2 _P, int _Sample, uint _Compare, uint _Data);
uint imageAtomicCompSwap(image2DMSArray _Image, ivec3 _P, int _Sample, uint _Compare, uint _Data);
uint imageAtomicCompSwap(iimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Compare, uint _Data);
uint imageAtomicCompSwap(uimage2DMSArray _Image, ivec3 _P, int _Sample, uint _Compare, uint _Data);
// [Generic] int imageAtomicCompSwap(IMAGE_PARAMS, int compare, int data);
int imageAtomicCompSwap(image2D _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimage2D _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimage2D _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(image3D _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimage3D _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimage3D _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(imageCube _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimageCube _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimageCube _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(imageBuffer _Image, int _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimageBuffer _Image, int _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimageBuffer _Image, int _P, int _Compare, int _Data);
int imageAtomicCompSwap(image2DArray _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimage2DArray _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimage2DArray _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(imageCubeArray _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimageCubeArray _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimageCubeArray _Image, ivec3 _P, int _Compare, int _Data);
int imageAtomicCompSwap(image1D _Image, int _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimage1D _Image, int _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimage1D _Image, int _P, int _Compare, int _Data);
int imageAtomicCompSwap(image1DArray _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimage1DArray _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimage1DArray _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(image2DRect _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(iimage2DRect _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(uimage2DRect _Image, ivec2 _P, int _Compare, int _Data);
int imageAtomicCompSwap(image2DMS _Image, ivec2 _P, int _Sample, int _Compare, int _Data);
int imageAtomicCompSwap(iimage2DMS _Image, ivec2 _P, int _Sample, int _Compare, int _Data);
int imageAtomicCompSwap(uimage2DMS _Image, ivec2 _P, int _Sample, int _Compare, int _Data);
int imageAtomicCompSwap(image2DMSArray _Image, ivec3 _P, int _Sample, int _Compare, int _Data);
int imageAtomicCompSwap(iimage2DMSArray _Image, ivec3 _P, int _Sample, int _Compare, int _Data);
int imageAtomicCompSwap(uimage2DMSArray _Image, ivec3 _P, int _Sample, int _Compare, int _Data);

// EmitStreamVertex
// Emits the current values of output variables to the current output primitive on stream stream.
// The argument to stream must be a constant integral expression. On return from this call, the
// values of all output variables are undefined.
// Can only be used if multiple output streams are supported.
// [Generic] void EmitStreamVertex(int stream);
void EmitStreamVertex(int _Stream);

// EndStreamPrimitive
// Completes the current output primitive on stream stream and starts a new one. The argument to
// stream must be a constant integral expression. No vertex is emitted.
// Can only be used if multiple output streams are supported.
// [Generic] void EndStreamPrimitive(int stream);
void EndStreamPrimitive(int _Stream);

// EmitVertex
// Emits the current values of output variables to the current output primitive.
// When multiple output streams are supported, this is equivalent to calling EmitStreamVertex(0).
// On return from this call, the values of output variables are undefined.
void EmitVertex();

// EndPrimitive
// Completes the current output primitive and starts a new one.
// When multiple output streams are supported, this is equivalent to calling EndStreamPrimitive(0).
// No vertex is emitted.
void EndPrimitive();

// Derivative Functions

// Derivatives may be computationally expensive and/or numerically unstable.
// Therefore, an implementation may approximate the true derivatives
// by using a fast but not entirely accurate derivative computation.
// Derivatives are undefined within non-uniform control flow.

// The expected behavior of a derivative is specified using forward/backward
// differencing.

// Forward differencing:

// latexmath:[F(x+dx) - F(x) \sim dFdx(x) \cdot dx (1a)]
// latexmath:[dFdx(x) \sim \frac{F(x+dx) - F(x)}{dx} (1b)]

// Backward differencing:

// latexmath:[F(x-dx) - F(x) \sim -dFdx(x) \cdot dx (2a)]
// latexmath:[dFdx(x) \sim \frac{F(x) - F(x-dx)}{dx} (2b)]

// With single-sample rasterization, latexmath:[dx \leq 1.0] in equations 1b and 2b.
// For multisample rasterization, latexmath:[dx < 2.0] in equations 1b and 2b.

// latexmath:[dFdy] is approximated similarly, with _y_ replacing _x_.

// With multisample rasterization, for any given fragment or sample, either
// neighboring fragments or samples may be considered.

// It is typical to consider a 2x2 square of fragments or samples, and compute
// independent *dFdxFine* per row and independent *dFdyFine* per column, while
// computing only a single *dFdxCoarse* and a single *dFdyCoarse* for the
// entire 2x2 square.
// Thus, all second-order coarse derivatives, e.g.
// *dFdxCoarse*(*dFdxCoarse*(_x_)), may be 0, even for non-linear arguments.
// However, second-order fine derivatives, e.g. *dFdxFine*(*dFdyFine*(_x_))
// will properly reflect the difference between the independent fine
// derivatives computed within the 2x2 square.

// The method may differ per fragment, subject to the constraint that the
// method may vary by window coordinates, not screen coordinates.
// The invariance requirement described in section 14.2 "`Invariance`" of the
// <<references,{apispec}>>, is relaxed for derivative calculations, because
// the method may be a function of fragment location.

// In some implementations, varying degrees of derivative accuracy for *dFdx*
// and *dFdy* may be obtained by providing GL hints (see section 21.4 "`Hints`"
// of the <<references,{apispec}>>), allowing a user to make an image quality
// versus speed trade off.
// These hints have no effect on *dFdxCoarse*, *dFdyCoarse*, *dFdxFine* and
// *dFdyFine*.

// DFdx
// Returns either dFdxFine(p) or dFdxCoarse(p), based on implementation choice, presumably
// whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
// [Generic] genFType dFdx(genFType p);
float dFdx(float _P);
vec2 dFdx(vec2 _P);
vec3 dFdx(vec3 _P);
vec4 dFdx(vec4 _P);

// DFdy
// Returns either dFdyFine(p) or dFdyCoarse(p), based on implementation choice, presumably
// whichever is the faster, or by whichever is selected in the API through quality-versus-speed hints.
// [Generic] genFType dFdy(genFType p);
float dFdy(float _P);
vec2 dFdy(vec2 _P);
vec3 dFdy(vec3 _P);
vec4 dFdy(vec4 _P);

// DFdxFine
// Returns the partial derivative of p with respect to the window x coordinate. Will use local
// differencing based on the value of p for the current fragment and its immediate neighbor(s).
// [Generic] genFType dFdxFine(genFType p);
float dFdxFine(float _P);
vec2 dFdxFine(vec2 _P);
vec3 dFdxFine(vec3 _P);
vec4 dFdxFine(vec4 _P);

// DFdyFine
// Returns the partial derivative of p with respect to the window y coordinate. Will use local
// differencing based on the value of p for the current fragment and its immediate neighbor(s).
// [Generic] genFType dFdyFine(genFType p);
float dFdyFine(float _P);
vec2 dFdyFine(vec2 _P);
vec3 dFdyFine(vec3 _P);
vec4 dFdyFine(vec4 _P);

// DFdxCoarse
// Returns the partial derivative of p with respect to the window x coordinate. Will use local
// differencing based on the value of p for the current fragment's neighbors, and will possibly, but
// not necessarily, include the value of p for the current fragment.
// That is, over a given area, the implementation can x compute derivatives in fewer unique locations
// than would be allowed for dFdxFine(p).
// [Generic] genFType dFdxCoarse(genFType p);
float dFdxCoarse(float _P);
vec2 dFdxCoarse(vec2 _P);
vec3 dFdxCoarse(vec3 _P);
vec4 dFdxCoarse(vec4 _P);

// DFdyCoarse
// Returns the partial derivative of p with respect to the window y coordinate. Will use local
// differencing based on the value of p for the current fragment's neighbors, and will possibly, but
// not necessarily, include the value of p for the current fragment.
// That is, over a given area, the implementation can compute y derivatives in fewer unique locations
// than would be allowed for dFdyFine(p).
// [Generic] genFType dFdyCoarse(genFType p);
float dFdyCoarse(float _P);
vec2 dFdyCoarse(vec2 _P);
vec3 dFdyCoarse(vec3 _P);
vec4 dFdyCoarse(vec4 _P);

// Fwidth
// Returns abs(dFdx(p)) + abs(dFdy(p)).
// [Generic] genFType fwidth(genFType p);
float fwidth(float _P);
vec2 fwidth(vec2 _P);
vec3 fwidth(vec3 _P);
vec4 fwidth(vec4 _P);

// FwidthFine
// Returns abs(dFdxFine(p)) + abs(dFdyFine(p)).
// [Generic] genFType fwidthFine(genFType p);
float fwidthFine(float _P);
vec2 fwidthFine(vec2 _P);
vec3 fwidthFine(vec3 _P);
vec4 fwidthFine(vec4 _P);

// FwidthCoarse
// Returns abs(dFdxCoarse(p)) + abs(dFdyCoarse(p)).
// An implementation may use the above or other methods to perform
// the calculation, subject to the following conditions:
// . The method may use piecewise linear approximations.
// Such linear approximations imply that higher order derivatives,
// dFdx(dFdx(x)) and above, are undefined.
// . The method may assume that the function evaluated is continuous.
// Therefore derivatives within the body of a non-uniform conditional are
// undefined.
// . The method may differ per fragment, subject to the constraint that the
// method may vary by window coordinates, not screen coordinates.
// The invariance requirement described in section 13.2 "Invariance" of
// the <<references,{apispec}>>, is relaxed for derivative calculations,
// because the method may be a function of fragment location.
// Other properties that are desirable, but not required, are:
// . Functions should be evaluated within the interior of a primitive
// (interpolated, not extrapolated).
// . Functions for dFdx should be evaluated while holding y constant.
// Functions for dFdy should be evaluated while holding x constant.
// However, mixed higher order derivatives, like dFdx(dFdy(y)) and
// dFdy(dFdx(x)) are undefined.
// . Derivatives of constant arguments should be 0.
// In some implementations, varying degrees of derivative accuracy may be
// obtained by providing GL hints (see section 19.1 "Hints" of the
// <<references,{apispec}>>), allowing a user to make an image quality versus
// speed trade off.
// [Generic] genFType fwidthCoarse(genFType p);
float fwidthCoarse(float _P);
vec2 fwidthCoarse(vec2 _P);
vec3 fwidthCoarse(vec3 _P);
vec4 fwidthCoarse(vec4 _P);

// DFdx
// Returns the derivative in x using local differencing for the input argument p.

// DFdy
// Returns the derivative in y using local differencing for the input argument p.
// These two functions are commonly used to estimate the filter width used to anti-alias procedural
// textures. We are assuming that the expression is being evaluated in parallel on a SIMD array so
// that at any given point in time the value of the function is known at the grid points represented
// by the SIMD array. Local differencing between SIMD array elements can therefore be used to derive
// dFdx, dFdy, etc.

// Fwidth
// Returns the sum of the absolute derivative in x and y using local differencing for the input
// argument p, i.e., abs(dFdx(p)) + abs(dFdy(p));

// Interpolation Functions

// Built-in interpolation functions are available to compute an interpolated
// value of a fragment shader input variable at a shader-specified (_x_, _y_)
// location.
// A separate (_x_, _y_) location may be used for each invocation of the
// built-in function, and those locations may differ from the default (_x_, _y_)
// location used to produce the default value of the input.

// For all of the interpolation functions, _interpolant_ must be an l-value
// from an *in* declaration;
// this can include a variable,
// a block or structure member,

// an array element, or some combination of these.

// Additionally, component selection operators (e.g. *.xy*, *.xxz*) may be applied
// to _interpolant_, in which case the interpolation function will return the
// result of applying the component selection operator to the interpolated value
// of _interpolant_ (for example, interpolateAt(v.xxz) is defined to return
// interpolateAt(v).xxz).


// Component selection operators (e.g. *.xy*), and field selection operators may
// not be used when specifying _interpolant_.

// Arrayed inputs can be indexed with general (nonuniform) integer expressions.

// If _interpolant_ is declared with the *flat* qualifier, the interpolated
// value will have the same value everywhere for a single primitive, so the
// location used for interpolation has no effect and the functions just return
// that same value.
// If _interpolant_ is declared with the *centroid* qualifier, the value
// returned by *interpolateAtSample*() and *interpolateAtOffset*() will be
// evaluated at the specified location, ignoring the location normally used
// with the *centroid* qualifier.

// If _interpolant_ is declared with the *noperspective* qualifier, the
// interpolated value will be computed without perspective correction.


// InterpolateAtCentroid
// Returns the value of the input interpolant sampled at a location inside both the pixel and the
// primitive being processed. The value obtained would be the same value assigned to the input
// variable if declared with the centroid qualifier.
// [Generic] float interpolateAtCentroid(float interpolant);
float interpolateAtCentroid(float _Interpolant);
// [Generic] vec2 interpolateAtCentroid(vec2 interpolant);
vec2 interpolateAtCentroid(vec2 _Interpolant);
// [Generic] vec3 interpolateAtCentroid(vec3 interpolant);
vec3 interpolateAtCentroid(vec3 _Interpolant);
// [Generic] vec4 interpolateAtCentroid(vec4 interpolant);
vec4 interpolateAtCentroid(vec4 _Interpolant);

// InterpolateAtSample
// Returns the value of the input interpolant variable at the location of sample number sample.
// If multisample buffers are not available, the input variable will be evaluated at the center of the
// pixel. If sample sample does not exist, the position used to interpolate the input variable is undefined.
// [Generic] float interpolateAtSample(float interpolant, int sample);
float interpolateAtSample(float _Interpolant, int _Sample);
// [Generic] vec2 interpolateAtSample(vec2 interpolant, int sample);
vec2 interpolateAtSample(vec2 _Interpolant, int _Sample);
// [Generic] vec3 interpolateAtSample(vec3 interpolant, int sample);
vec3 interpolateAtSample(vec3 _Interpolant, int _Sample);
// [Generic] vec4 interpolateAtSample(vec4 interpolant, int sample);
vec4 interpolateAtSample(vec4 _Interpolant, int _Sample);

// InterpolateAtOffset
// Returns the value of the input interpolant variable sampled at an offset from the center of the
// pixel specified by offset. The two floating-point components of offset, give the offset in
// pixels in the x and y directions, respectively.
// An offset of (0, 0) identifies the center of the pixel. The range and granularity of offsets
// supported by this function is implementation-dependent.
// [Generic] float interpolateAtOffset(float interpolant, vec2 offset);
float interpolateAtOffset(float _Interpolant, vec2 _Offset);
// [Generic] vec2 interpolateAtOffset(vec2 interpolant, vec2 offset);
vec2 interpolateAtOffset(vec2 _Interpolant, vec2 _Offset);
// [Generic] vec3 interpolateAtOffset(vec3 interpolant, vec2 offset);
vec3 interpolateAtOffset(vec3 _Interpolant, vec2 _Offset);
// [Generic] vec4 interpolateAtOffset(vec4 interpolant, vec2 offset);
vec4 interpolateAtOffset(vec4 _Interpolant, vec2 _Offset);

// Noise1
// Returns a 1D noise value based on the input value x.
// [Generic] float noise1(genFType x);
float noise1(float _X);
float noise1(vec2 _X);
float noise1(vec3 _X);
float noise1(vec4 _X);

// Noise2
// Returns a 2D noise value based on the input value x.
// [Generic] vec2 noise2(genFType x);
vec2 noise2(float _X);
vec2 noise2(vec2 _X);
vec2 noise2(vec3 _X);
vec2 noise2(vec4 _X);

// Noise3
// Returns a 3D noise value based on the input value x.
// [Generic] vec3 noise3(genFType x);
vec3 noise3(float _X);
vec3 noise3(vec2 _X);
vec3 noise3(vec3 _X);
vec3 noise3(vec4 _X);

// Noise4
// Returns a 4D noise value based on the input value x.
// [Generic] vec4 noise4(genFType x);
vec4 noise4(float _X);
vec4 noise4(vec2 _X);
vec4 noise4(vec3 _X);
vec4 noise4(vec4 _X);

// Barrier
// For any given static instance of barrier(), all tessellation control shader invocations for a
// single input patch must enter it before any will be allowed to continue beyond it, or all compute
// shader invocations for a single workgroup must enter it before any will continue beyond it.
// The function barrier() provides a partially defined order of execution
// between shader invocations.
// The ensures that, for some types of memory accesses, values written by one
// invocation prior to a given static instance of barrier() can be safely read
// by other invocations after their call to the same static instance barrier().
// Because invocations may execute in an undefined order between these barrier
// calls, the values of a per-vertex or per-patch output variable for tessellation
// control shaders, or the values of shared variables for compute shaders will be
// undefined in a number of cases enumerated in
// "<<output-variables,Output Variables>>" (for tessellation control shaders)
// and "<<shared-variables,Shared Variables>>" (for compute shaders).
// For tessellation control shaders, the barrier() function may only be
// placed inside the function main() of the shader and may not be called
// within any control flow.
// Barriers are also disallowed after a return statement in the function main().
// Any such misplaced barriers result in a compile-time error.
// A barrier() affects control flow but only synchronizes memory accesses
// to shared variables and tessellation control output variables.
// For other memory accesses, it does not ensure that values written by one invocation
// prior to a given static instance of barrier() can be safely read by other
// invocations after their call to the same static instance of barrier().
// To achieve this requires the use of both barrier() and a memory barrier.
// For compute shaders, the barrier() function may be placed within control
// flow, but that control flow must be uniform control flow.
// That is, all the controlling expressions that lead to execution of the
// barrier must be dynamically uniform expressions.
// This ensures that if any shader invocation enters a conditional statement,
// then all invocations will enter it.
// While compilers are encouraged to give warnings if they can detect this
// might not happen, compilers cannot completely determine this.
// Hence, it is the author's responsibility to ensure barrier() only exists
// inside uniform control flow.
// Otherwise, some shader invocations will stall indefinitely, waiting for a
// barrier that is never reached by other invocations.
void barrier();

// MemoryBarrier
// Control the ordering of memory transactions issued by a single shader invocation.
void memoryBarrier();

// MemoryBarrierAtomicCounter
// Control the ordering of accesses to atomic-counter variables issued by a single shader invocation.
void memoryBarrierAtomicCounter();

// MemoryBarrierBuffer
// Control the ordering of memory transactions to buffer variables issued within a single shader invocation.
void memoryBarrierBuffer();

// MemoryBarrierShared
// Control the ordering of memory transactions to shared variables issued within a single shader
// invocation, as viewed by other invocations in the same workgroup.
// Only available in compute shaders.
void memoryBarrierShared();

// MemoryBarrierImage
// Control the ordering of memory transactions to images issued within a single shader invocation.
void memoryBarrierImage();

// GroupMemoryBarrier
// Control the ordering of all memory transactions issued within a single shader invocation, as viewed
// by other invocations in the same workgroup.
// Only available in compute shaders.
void groupMemoryBarrier();

// SubpassLoad
// Read from a subpass input, from the implicit location (x, y, layer) of the current fragment coordinate.
// [Generic] gvec4 subpassLoad(gsubpassInput subpass);
vec4 subpassLoad(subpassInput _Subpass);
ivec4 subpassLoad(isubpassInput _Subpass);
uvec4 subpassLoad(usubpassInput _Subpass);
// [Generic] gvec4 subpassLoad(gsubpassInputMS subpass, int sample);
vec4 subpassLoad(subpassInputMS _Subpass, int _Sample);
ivec4 subpassLoad(isubpassInputMS _Subpass, int _Sample);
uvec4 subpassLoad(usubpassInputMS _Subpass, int _Sample);

// AnyInvocation
// Returns true if and only if value is true for at least one active invocation in the group.
// [Generic] bool anyInvocation(bool value);
bool anyInvocation(bool _Value);

// AllInvocations
// Returns true if and only if value is true for all active invocations in the group.
// [Generic] bool allInvocations(bool value);
bool allInvocations(bool _Value);

// AllInvocationsEqual
// Returns true if value is the same for all active invocations in the group.
// [Generic] bool allInvocationsEqual(bool value);
bool allInvocationsEqual(bool _Value);
