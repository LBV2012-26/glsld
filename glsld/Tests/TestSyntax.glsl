#version 460 core
#pragma shader_stage(fragment)

#extension GL_EXT_ray_tracing :  enable

out gl_PerVertex {
    invariant vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
}

void main() {
    accelerationStructureEXT AS;
}
