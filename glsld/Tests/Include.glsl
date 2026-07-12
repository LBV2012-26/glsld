#ifndef INCLUDE_GLSL_
#define INCLUDE_GLSL_

const int k114514 = 114514;

#define DECLARE_STRUCT_ARRAY(type, size) \
struct type##Buffer {                    \
    type data;                           \
    type member;                         \
} type##_buffer[size];

#endif
