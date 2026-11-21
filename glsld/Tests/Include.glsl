#pragma once

const int kIncludeValue = 42;

#define DECLARE_STRUCT_ARRAY(type, size) \
struct type##Buffer {                    \
    type data;                           \
    type member;                         \
} type##_buffer[size];
