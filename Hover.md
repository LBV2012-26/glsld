#### 一、变量
代码：
```glsl
spirv_decorate(extensions = "GL_EXT_spirv_intrinsics", id = 140) layout(binding=1, set=0) uniform readonly
spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [5353], set = "GLSL.std.450", id = 5341) MyVar;
```
Hover:
```glsl
(global variable) MyVar
```
---
```glsl
-> spirv_type(extensions = ["SPV_KHR_ray_tracing"], capabilities = [5353], set = "GLSL.std.450", id = 5341)
```
---
```glsl
// With [Layout]
layout(set = 0, binding = 0)
// With [Storage]
uniform
// With [Decorate]
readonly spirv_decorate(extensions = "GL_EXT_spirv_intrinsics")
// Defined in this file, line xx
spirv_type(...) MyVar;
```
代码：
```glsl
layout(location = 0) out vec4 FragColor;
```
Hover:
```glsl
(global variable) FragColor
```
---
```glsl
-> vec4
```
---
```glsl
// With [Layout]
layout(location = 0)
// With [Storage]
out
// Defined in this file, line xx
vec4 FragColor;
```
代码：
```glsl
layout(std140, set = 0, binding = 1) uniform MyUniformBuffer {
    mat4 my_matrix;
} ubo;
```
Hover:
```glsl
(global variable) ubo
```
---
```glsl
-> MyUniformBuffer
```
---
```glsl
// With [Layout]
layout(std140, set = 0, binding = 1)
// With [Storage]
uniform
// Defined in this file, line xx
MyUniformBuffer { ... } ubo;
```
代码：
```glsl
int my_var; // 局部变量
```
Hover:
```glsl
(local variable) my_var
```
---
```glsl
-> int
```
---
```glsl
// Defined in this file, line xx
int my_var;
```
#### 二、函数
代码：
```glsl
spirv_instruction(id = 140)
float FRem(float x, float y);
```
Hover:
```glsl
(function) FRem
```
---
```glsl
-> float
// Parameters
float x
float y
```
---
```glsl
// With [Decorate]
spirv_instruction(id = 140)
// Defined in this file, line xx
float FRem(float x, float y);
```
#### 三、类型
代码：
```glsl
struct MyStruct {
    int a;
    vec3 b;
};
```
Hover:
```glsl
(struct) MyStruct
```
---
```glsl
// Defined in this file, line xx
struct MyStruct;
```
代码：
```glsl
layout(buffer_reference, std430) buffer LightDataBuffer {
    LightData data;
};
```
Hover:
```glsl
(buffer) LightDataBuffer
```
---
```glsl
// With [Layout]
layout(buffer_reference, std430)
// Defined in this file, line xx
buffer LightDataBuffer;
```
#### 四、类型成员/字段
代码：
```glsl
struct MyStruct {
    int member;
};
```
Hover:
```glsl
(field) member
```
---
```glsl
-> int
```
---
```glsl
// Defined in this file, line xx
// In struct "MyStruct"
int member;
```