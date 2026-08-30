#version 460 core
#extension GL_EXT_nonuniform_qualifier : enable

// 1. 预处理测试
#define MAX_LIGHTS 32
#define PI 3.1415926
#if defined(MAX_LIGHTS)
    #define HAS_LIGHTS 1
#else
    #define HAS_LIGHTS 0
#endif

void bbbbbbbbbb() {
    int x = true;
}

void ffffffffffffffffffffffffff(int ppppppppppp) {
    bbbbbbbbbb();
    int a[5] = {0, 0, 0, 0, 0};

    if (true)
        int b, c, d, e, f;
}

// 2. 结构体定义
struct LightData {
    vec3 position;
    float radius;
    vec4 color;
};

// 嵌套结构体
struct SceneData {
    LightData mainLight;
    int lightCount;
    float ambientIntensity;
};

// 3. 接口块 (Interface Blocks) 和修饰符解析
// 测试 layout(), set, binding, uniform block
layout(set = 0, binding = 0, std140) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
    vec3 camPos;
} camera; // 实例名为 camera

// 测试 buffer block (SSBO) 和无名实例
layout(std430, binding = 1) buffer LightBuffer {
    LightData lights[]; // 变长数组
}; // 注意：这里没有实例名，直接访问 lights

// 测试 push_constant
layout(push_constant) uniform PushConsts {
    mat4 model;
    float time;
} pc;

// 4. 全局变量与修饰符
// 测试多变量声明、数组、layout
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec4 outColor;

// 简单全局变量
const float epsilon = 0.001, aaaaaaaaaaaaaaaaaaaaaaaaaaaa = 0.0;
layout(set = 1, binding = 0) uniform sampler2D texSampler;

// 5. 函数原型 (前向声明)
// 测试参数列表解析：包含修饰符、无名参数
float calculateShadow(in vec3 pos, vec3); // 第二个参数无名
void weird_func(void); // void 参数

void struct_func(void, SceneData scene) {
    return;
}

// 6. 主函数：包含复杂控制流
void main() {
    // 变量声明与初始化
    vec3 N = normalize(inNormal);
    vec3 L = normalize(lights[0].position - inPosition);
    
    float intensity = 0.0;

    // 作用域测试：IF-ELSE
    // 这里 N 是上面声明的 N
    if (pc.time > 0.0) {
        // 测试 Shadowing (遮蔽)：这里的 N 是新的局部变量
        vec3 N = vec3(0.0, 1.0, 0.0); 
        intensity = dot(N, L);
    } else {
        intensity = 0.5;
    }
    // 这里的 N 恢复为 main 函数开头的 N

    // 作用域测试：FOR 循环
    // i 的作用域应限制在 for 循环内
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        if (i >= camera.lightCount) { 
            break; // Jump statement
        }

        // 复杂的初始化声明
        float dist = length(lights[i].position - inPosition);
        
        // 嵌套 IF 和 CONTINUE
        if (dist > lights[i].radius) {
            continue;
        }

        // 匿名作用域 (Anonymous Scope)
        {
            vec3 tempColor = lights[i].color.rgb;
            outColor.rgb += tempColor * dist;
        }
        // tempColor 在这里应该不可见
    }
    // i 和 dist 在这里应该不可见

    // 作用域测试：WHILE 循环
    int iter = 0;
    while (iter < 10) {
        iter++;
        if (iter == 5) discard; // Discard statement
    }

    // 作用域测试：DO-WHILE
    do {
        iter--;
    } while (iter > 0);

    // 作用域测试：SWITCH
    // 测试 case 和 default
    switch (iter) {
        case 0:
            outColor.a = 1.0;
            break;
        case 1: {
            int testswitch;
        }
        case 2:
            outColor.a = 0.5;
            break;
        default:
            outColor.a = 0.0;
    }

    // 函数调用
    float shadow = calculateShadow(inPosition, N);
    outColor *= shadow;
}

// 7. 函数定义
float calculateShadow(in vec3 pos, const vec3 normal) {
    // 测试 return 语句
    if (pos.y < 0.0) return 0.0;
    return 1.0;
}

// 测试 void 参数的定义
void weird_func(void) {
    int a = 0;

    if (true) {
        int b;
    } else if (true) {
        int c;
    } else if (true) {
        int d;
    } else {
        int e;
    }

    return;
}
