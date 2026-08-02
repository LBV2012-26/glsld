#ifndef INCLUDE_GLSL_
#define INCLUDE_GLSL_

const int kIncludeValue = 42;

#define DECLARE_STRUCT_ARRAY(type, size) \
struct type##Buffer {                    \
    type data;                           \
    type member;                         \
} type##_buffer[size];

#endif
