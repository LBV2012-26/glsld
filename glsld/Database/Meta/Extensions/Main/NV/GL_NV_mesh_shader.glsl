#define __GLSLD_EXTENSION_ENABLED_GL_NV_mesh_shader 1

const uvec3 gl_WorkGroupSize;
in    uvec3 gl_WorkGroupID;
in    uvec3 gl_LocalInvocationID;
in    uvec3 gl_GlobalInvocationID;
in    uint  gl_LocalInvocationIndex;
in    uint  gl_MeshViewCountNV;
in    uint  gl_MeshViewIndicesNV[];

out uint gl_TaskCountNV;

out uint gl_PrimitiveCountNV;
out uint gl_PrimitiveIndicesNV[];

out gl_MeshPerVertexNV {
    vec4            gl_Position;
#if defined(__GLSLD_EXTENSION_ENABLED_GL_NVX_multiview_per_view_attributes)
    perviewNV vec4  gl_PositionPerViewNV[];
#endif
    float           gl_PointSize;
    float           gl_ClipDistance[];
    perviewNV float gl_ClipDistancePerViewNV[][];
    float           gl_CullDistance[];
    perviewNV float gl_CullDistancePerViewNV[][];
} gl_MeshVerticesNV[];

perprimitiveNV out gl_MeshPerPrimitiveNV {
    int           gl_PrimitiveID;
    int           gl_Layer;
    perviewNV int gl_LayerPerViewNV[];
    int           gl_ViewportIndex;
#if defined(__GLSLD_EXTENSION_ENABLED_GL_NV_viewport_array2)
    int           gl_ViewportMask[];
#endif
#if defined(__GLSLD_EXTENSION_ENABLED_GL_NVX_multiview_per_view_attributes)
    perviewNV int gl_ViewportMaskPerViewNV[][];
#endif
} gl_MeshPrimitivesNV[];

const int gl_MaxMeshViewCountNV = 4;

#if defined(__GLSLD_EXTENSION_ENABLED_GL_ARB_shader_draw_parameters)
in int gl_DrawIDARB;
#endif

void writePackedPrimitiveIndices4x8NV(uint _IndexOffset, uint _PackedIndices);
