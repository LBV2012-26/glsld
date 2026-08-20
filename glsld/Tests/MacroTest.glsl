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
