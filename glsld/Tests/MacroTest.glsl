// **基础用例**

// 1) 对象宏最小用例
#define T int
T a;

// 2) 对象宏链式展开
#define A B
#define B C
#define C float
C x;
A y;

// 3) 函数宏（1参）
#define SQRT(x) (x * x)
SQRT(a);

// 4) 函数宏（2参）
#define MUL(x, y) (x * y)
MUL(a, b);

// 5) 0参数函数宏
#define ONE() 1
ONE();

// 6) 函数宏调用（有空白）
#define F1(x, y) (x + y)
F1 (a, b);

// **进阶/嵌套用例**

// 7) 实参是表达式
#define F2(x, y) (x + y)
F2(a + b, c * d);

// 8) 函数宏嵌套调用
#define F3(x, y) (x * y)
#define G1(t) F3(t, t)
G1(a);

// 9) 深层嵌套调用
#define F4(x, y) (x + y)
#define G2(x) F4(x, x)
#define H1(x) G2(G1(x))
H1(a);

// 10) 实参里带括号与逗号运算
#define F5(x, y) (x + y)
F5((a, b), c);

// 11) 对象宏 + 函数宏混合
#define U int
#define CAST(x) (U)(x)
CAST(a);

// 12) 自递归宏（防死循环）
#define A A
A;

// 13) 互相递归宏（防死循环）
#define M N
#define N M
M;

// 14) token paste 基础
#define BAT(x, y) x##y
BAT(ab, cd);

// 15) token paste + 宏参数
#define X ab
#define Y cd
#define CAT(x, y) x##y
CAT(X, Y);

// 16) token paste 生成数字/标识符
#define DAT(x, y) x##y
DAT(12, 34);
DAT(v, 2);

// **不完整输入（模拟用户打到一半）**

// 17) 调用少右括号
#define F5(x, y) (x * y)
F5(a, b

// 18) 调用尾逗号
#define F6(x, y) (x * y)
F6(a, );

// 19) 调用缺第二参数
#define F7(x, y) (x * y)
F7(a

// 20) 函数宏定义未闭合参数列表
#define F8(x, y
F8(a, b);

// 21) 函数宏定义参数后无 `)`
#define F9(x, y (x + y)
F9(a, b);

// 22) 对象宏定义写到一半
#define F10
F10 a;

// 23) token paste 右侧缺失
#define EAT(x, y) x##
EAT(a, b);

// 24) 函数宏展开体中括号不完整
#define F11(x, y) (x * (y)
F11(a, b);

// 25) 调用中嵌套不完整
#define F12(x, y) (x + y)
F12(G1(a, b), c

#define ENUMERATE_GLM_VECTOR(prefix) \
    glm::prefix##vec2, glm::prefix##vec3, glm::prefix##vec4

#define ENUMERATE_GLM_MATRIX(prefix) \
    glm::prefix##mat2x2, glm::prefix##mat2x3, glm::prefix##mat2x4, \
    glm::prefix##mat3x2, glm::prefix##mat3x3, glm::prefix##mat3x4, \
    glm::prefix##mat4x2, glm::prefix##mat4x3, glm::prefix##mat4x4

        using GlmAggregate = std::variant<
            ENUMERATE_GLM_VECTOR(i64),
            ENUMERATE_GLM_VECTOR(u64),
            ENUMERATE_GLM_VECTOR(d),
            ENUMERATE_GLM_VECTOR(b),
            ENUMERATE_GLM_MATRIX(i64),
            ENUMERATE_GLM_MATRIX(u64),
            ENUMERATE_GLM_MATRIX(d)
        >;

#define WRAP_TYPED_OVERLOAD(func, type) WrapSingleSignature(func<type>)

#define WRAP_GLM_VECTOR_OVERLOADS(func, prefix)            \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##vec2),          \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##vec3),          \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##vec4)

#define WRAP_GLM_MATRIX_OVERLOADS(func, prefix)            \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x4),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x4),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x4)

#define WRAP_GLM_SQUARE_MATRIX_OVERLOADS(func, prefix)     \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat2x2),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat3x3),        \
    WRAP_TYPED_OVERLOAD(func, glm::prefix##mat4x4)

#define REGISTER_FLOAT_OVERLOADS(name, func)               \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, double),                 \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_NUMERIC_OVERLOADS(name, func)             \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, std::int64_t),           \
        WRAP_GLM_VECTOR_OVERLOADS(func, i64),              \
        WRAP_TYPED_OVERLOAD(func, std::uint64_t),          \
        WRAP_GLM_VECTOR_OVERLOADS(func, u64),              \
        WRAP_TYPED_OVERLOAD(func, double),                 \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_INTEGER_OVERLOADS(name, func)             \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, std::int64_t),           \
        WRAP_GLM_VECTOR_OVERLOADS(func, i64),              \
        WRAP_TYPED_OVERLOAD(func, std::uint64_t),          \
        WRAP_GLM_VECTOR_OVERLOADS(func, u64)               \
    ))

#define REGISTER_FLOAT_VECTOR_OVERLOADS(name, func)        \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_NUMERIC_VECTOR_OVERLOADS(name, func)      \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_VECTOR_OVERLOADS(func, i64),              \
        WRAP_GLM_VECTOR_OVERLOADS(func, u64),              \
        WRAP_GLM_VECTOR_OVERLOADS(func, d)                 \
    ))

#define REGISTER_BOOLEAN_OVERLOADS(name, func)             \
    Register(name, MakeOverloader(                         \
        WRAP_TYPED_OVERLOAD(func, bool),                   \
        WRAP_GLM_VECTOR_OVERLOADS(func, b)                 \
    ))

#define REGISTER_BOOLEAN_VECTOR_OVERLOADS(name, func)      \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_VECTOR_OVERLOADS(func, b)                 \
    ))

#define REGISTER_FLOAT_MATRIX_OVERLOADS(name, func)        \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_MATRIX_OVERLOADS(func, d)                 \
    ))

#define REGISTER_FLOAT_SQUARE_MATRIX_OVERLOADS(name, func) \
    Register(name, MakeOverloader(                         \
        WRAP_GLM_SQUARE_MATRIX_OVERLOADS(func, d)          \
    ))

        REGISTER_FLOAT_OVERLOADS("radians", MathMeta::Radians);
        REGISTER_FLOAT_OVERLOADS("degrees", MathMeta::Degrees);
        REGISTER_FLOAT_OVERLOADS("sin", MathMeta::Sin);
        REGISTER_FLOAT_OVERLOADS("cos", MathMeta::Cos);
        REGISTER_FLOAT_OVERLOADS("tan", MathMeta::Tan);
        REGISTER_FLOAT_OVERLOADS("asin", MathMeta::Asin);
        REGISTER_FLOAT_OVERLOADS("acos", MathMeta::Acos);
        REGISTER_FLOAT_OVERLOADS("sinh", MathMeta::Sinh);
        REGISTER_FLOAT_OVERLOADS("cosh", MathMeta::Cosh);
        REGISTER_FLOAT_OVERLOADS("tanh", MathMeta::Tanh);
        REGISTER_FLOAT_OVERLOADS("asinh", MathMeta::Asinh);
        REGISTER_FLOAT_OVERLOADS("acosh", MathMeta::Acosh);
        REGISTER_FLOAT_OVERLOADS("atanh", MathMeta::Atanh);

        REGISTER_FLOAT_OVERLOADS("pow", MathMeta::Pow);
        REGISTER_FLOAT_OVERLOADS("exp", MathMeta::Exp);
        REGISTER_FLOAT_OVERLOADS("exp2", MathMeta::Exp2);
        REGISTER_FLOAT_OVERLOADS("log", MathMeta::Log);
        REGISTER_FLOAT_OVERLOADS("log2", MathMeta::Log2);
        REGISTER_FLOAT_OVERLOADS("sqrt", MathMeta::Sqrt);
        REGISTER_FLOAT_OVERLOADS("inversesqrt", MathMeta::InverseSqrt);

        REGISTER_NUMERIC_OVERLOADS("abs", MathMeta::Abs);
        REGISTER_NUMERIC_OVERLOADS("sign", MathMeta::Sign);
        REGISTER_FLOAT_OVERLOADS("floor", MathMeta::Floor);
        REGISTER_FLOAT_OVERLOADS("trunc", MathMeta::Trunc);
        REGISTER_FLOAT_OVERLOADS("round", MathMeta::Round);
        REGISTER_FLOAT_OVERLOADS("roundEven", MathMeta::RoundEven);
        REGISTER_FLOAT_OVERLOADS("ceil", MathMeta::Ceil);
        REGISTER_FLOAT_OVERLOADS("fract", MathMeta::Fract);
        REGISTER_FLOAT_OVERLOADS("mod", MathMeta::Mod);
        REGISTER_NUMERIC_OVERLOADS("min", MathMeta::Min);
        REGISTER_NUMERIC_OVERLOADS("max", MathMeta::Max);
        REGISTER_NUMERIC_OVERLOADS("clamp", MathMeta::Clamp);
        REGISTER_FLOAT_OVERLOADS("step", MathMeta::Step);
        REGISTER_FLOAT_OVERLOADS("smoothstep", MathMeta::SmoothStep);
        REGISTER_FLOAT_OVERLOADS("isnan", MathMeta::IsNan);
        REGISTER_FLOAT_OVERLOADS("isinf", MathMeta::IsInf);
        REGISTER_FLOAT_OVERLOADS("fma", MathMeta::Fma);
        REGISTER_FLOAT_OVERLOADS("length", MathMeta::Length);
        REGISTER_FLOAT_OVERLOADS("distance", MathMeta::Distance);
        REGISTER_FLOAT_OVERLOADS("dot", MathMeta::Dot);
        REGISTER_FLOAT_OVERLOADS("normalize", MathMeta::Normalize);
        REGISTER_FLOAT_OVERLOADS("faceforward", MathMeta::FaceForward);
        REGISTER_FLOAT_OVERLOADS("reflect", MathMeta::Reflect);
        REGISTER_FLOAT_MATRIX_OVERLOADS("matrixCompMult", MathMeta::MatrixCompMult);
        REGISTER_FLOAT_MATRIX_OVERLOADS("transpose", MathMeta::Transpose);
        REGISTER_FLOAT_SQUARE_MATRIX_OVERLOADS("determinant", MathMeta::Determinant);
        REGISTER_FLOAT_SQUARE_MATRIX_OVERLOADS("inverse", MathMeta::Inverse);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("lessThan", MathMeta::LessThan);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("lessThanEqual", MathMeta::LessThanEqual);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("greaterThan", MathMeta::GreaterThan);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("greaterThanEqual", MathMeta::GreaterThanEqual);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("equal", MathMeta::Equal);
        REGISTER_NUMERIC_VECTOR_OVERLOADS("notEqual", MathMeta::NotEqual);

        REGISTER_BOOLEAN_OVERLOADS("any", MathMeta::Any);
        REGISTER_BOOLEAN_OVERLOADS("all", MathMeta::All);
        REGISTER_BOOLEAN_OVERLOADS("not", MathMeta::Not);

        REGISTER_INTEGER_OVERLOADS("bitCount", MathMeta::BitCount);
        REGISTER_INTEGER_OVERLOADS("findLSB", MathMeta::FindLsb);
        REGISTER_INTEGER_OVERLOADS("findMSB", MathMeta::FindMsb);
        REGISTER_INTEGER_OVERLOADS("bitfieldReverse", MathMeta::BitfieldReverse);
        REGISTER_INTEGER_OVERLOADS("bitfieldExtract", MathMeta::BitfieldExtract);
        REGISTER_INTEGER_OVERLOADS("bitfieldInsert", MathMeta::BitfieldInsert);
