// GL_EXT_bfloat16

// Types
// bfloat16_t, bf16vec2, bf16vec3, bf16vec4

// Type aliases
// genBFType = bfloat16_t, bf16vec2, bf16vec3, bf16vec4
// genI16Type = int16_t, i16vec2, i16vec3, i16vec4
// genU16Type = uint16_t, u16vec2, u16vec3, u16vec4

bfloat16_t dot(bfloat16_t _X, bfloat16_t _Y);
bfloat16_t dot(bf16vec2 _X, bf16vec2 _Y);
bfloat16_t dot(bf16vec3 _X, bf16vec3 _Y);
bfloat16_t dot(bf16vec4 _X, bf16vec4 _Y);

int16_t bfloat16BitsToIntEXT(bfloat16_t _Value);
i16vec2 bfloat16BitsToIntEXT(bf16vec2 _Value);
i16vec3 bfloat16BitsToIntEXT(bf16vec3 _Value);
i16vec4 bfloat16BitsToIntEXT(bf16vec4 _Value);

uint16_t bfloat16BitsToUintEXT(bfloat16_t _Value);
u16vec2 bfloat16BitsToUintEXT(bf16vec2 _Value);
u16vec3 bfloat16BitsToUintEXT(bf16vec3 _Value);
u16vec4 bfloat16BitsToUintEXT(bf16vec4 _Value);

bfloat16_t intBitsToBFloat16EXT(int16_t _Value);
bf16vec2 intBitsToBFloat16EXT(i16vec2 _Value);
bf16vec3 intBitsToBFloat16EXT(i16vec3 _Value);
bf16vec4 intBitsToBFloat16EXT(i16vec4 _Value);

bfloat16_t uintBitsToBFloat16EXT(uint16_t _Value);
bf16vec2 uintBitsToBFloat16EXT(u16vec2 _Value);
bf16vec3 uintBitsToBFloat16EXT(u16vec3 _Value);
bf16vec4 uintBitsToBFloat16EXT(u16vec4 _Value);

// Cooperative matrix load/store overloads for bfloat16 types
void coopMatLoad(out coopmat _M, volatile coherent bfloat16_t[] _Buf, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _M, volatile coherent bf16vec2[] _Buf, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatLoad(out coopmat _M, volatile coherent bf16vec4[] _Buf, uint _Element, uint _Stride, int _MatrixLayout);

void coopMatStore(coopmat _M, volatile coherent out bfloat16_t[] _Buf, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _M, volatile coherent out bf16vec2[] _Buf, uint _Element, uint _Stride, int _MatrixLayout);
void coopMatStore(coopmat _M, volatile coherent out bf16vec4[] _Buf, uint _Element, uint _Stride, int _MatrixLayout);
