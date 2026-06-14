in    uvec3 gl_NumWorkGroups;
const uvec3 gl_WorkGroupSize;

in uvec3 gl_WorkGroupID;
in uvec3 gl_LocalInvocationID;
in uvec3 gl_GlobalInvocationID;
in uint  gl_LocalInvocationIndex;

out uint  gl_PrimitivePointIndicesEXT[];
out uvec2 gl_PrimitiveLineIndicesEXT[];
out uvec3 gl_PrimitiveTriangleIndicesEXT[];

out gl_MeshPerVertexEXT {
    vec4  gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
    float gl_CullDistance[];
} gl_MeshVerticesEXT[];

perprimitiveEXT out gl_MeshPerPrimitiveEXT {
    int  gl_PrimitiveID;
    int  gl_Layer;
    int  gl_ViewportIndex;
    bool gl_CullPrimitiveEXT;
    int  gl_PrimitiveShadingRateEXT;
} gl_MeshPrimitivesEXT[];

void EmitMeshTasksEXT(uint _GroupCountX, uint _GroupCountY, uint _GroupCountZ);
void SetMeshOutputsEXT(uint _VertexCount, uint _PrimitiveCount);
