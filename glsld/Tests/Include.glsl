#ifndef INCLUDE_GLSL_
#define INCLUDE_GLSL_

#include <Debugger.glsl>

const int k114514 = SHIT;

#define DECLARE_STRUCT_ARRAY(type, size) \
struct type##Buffer {                    \
    type data;                           \
    type member;                         \
} type##_buffer[size];

#endif
