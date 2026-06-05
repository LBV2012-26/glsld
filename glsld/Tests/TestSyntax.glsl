#version 460 core
#pragma shader_stage(fragment)

#include "Include.glsl"

#define DECLARE_STRUCT(name, type) \
    struct name {                  \
        type field;                \
    } my_struct;

void main() {
    DECLARE_STRUCT(MyStruct, float);
    DECLARE_STRUCT_ARRAY(MyStruct, 114514)
    my_struct.field = 1.0;
    MyStruct_buffer[0].data.field = 1.0;
}
