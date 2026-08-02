#version 450
// 启用所需扩展：缓冲引用、标量块布局、显式64位类型
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_ARB_shader_draw_parameters : enable

void main()
{
    gl_Position = 0;
}