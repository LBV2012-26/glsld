// **基础用例**

// 1) 对象宏最小用例
// #define F int
// F a;

// 2) 对象宏链式展开
// #define A B
// #define B C
// #define C float
// C x;
// A y;

// 3) 函数宏（1参）
// #define SQR(x) (x * x)
// SQR(a);

// 4) 函数宏（2参）
// #define MUL(x, y) (x * y)
// MUL(a, b);

// 5) 0参数函数宏
// #define ONE() 1
// ONE();

// 6) 函数宏调用（有空白）
// #define F(x, y) (x + y)
// F (a, b);

// **进阶/嵌套用例**

// 7) 实参是表达式
// #define F(x, y) (x + y)
// F(a + b, c * d);

// 8) 函数宏嵌套调用
// #define F(x, y) (x * y)
// #define G(t) F(t, t)
// G(a);

// 9) 深层嵌套调用
// #define F(x, y) (x + y)
// #define G(x) F(x, x)
// #define H(x) G(G(x))
// H(a);

// 10) 实参里带括号与逗号运算
// #define F(x, y) (x + y)
// F((a, b), c);

// 11) 对象宏 + 函数宏混合
// #define T int
// #define CAST(x) (T)(x)
// CAST(a);

// 12) 自递归宏（防死循环）
// #define A A
// A;

// 13) 互相递归宏（防死循环）
// #define A B
// #define B A
// A;

// 14) token paste 基础
// #define CAT(x, y) x##y
// CAT(ab, cd);

// 15) token paste + 宏参数
// #define X ab
// #define Y cd
// #define CAT(x, y) x##y
// CAT(X, Y);

// 16) token paste 生成数字/标识符
// #define CAT(x, y) x##y
// CAT(12, 34);
// CAT(v, 2);

// **不完整输入（模拟用户打到一半）**

// 17) 调用少右括号
// #define F(x, y) (x * y)
// F(a, b

// 18) 调用尾逗号
// #define F(x, y) (x * y)
// F(a, );

// 19) 调用缺第二参数
// #define F(x, y) (x * y)
// F(a

// 20) 函数宏定义未闭合参数列表
// #define F(x, y
// F(a, b);

// 21) 函数宏定义参数后无 `)`
// #define F(x, y (x + y)
// F(a, b);

// 22) 对象宏定义写到一半
// #define F
// F a;

// 23) token paste 右侧缺失
// #define CAT(x, y) x##
// CAT(a, b);

// 24) 函数宏展开体中括号不完整
// #define F(x, y) (x * (y)
// F(a, b);

// 25) 调用中嵌套不完整
// #define F(x, y) (x + y)
// F(G(a, b), c
