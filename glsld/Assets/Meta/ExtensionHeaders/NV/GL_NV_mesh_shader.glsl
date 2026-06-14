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
    perviewNV vec4  gl_PositionPerViewNV[];
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
    int           gl_ViewportMask[];
    perviewNV int gl_ViewportMaskPerViewNV[][];
} gl_MeshPrimitivesNV[];

const int gl_MaxMeshViewCountNV = 4;

in int gl_DrawIDARB;

void writePackedPrimitiveIndices4x8NV(uint _IndexOffset, uint _PackedIndices);
