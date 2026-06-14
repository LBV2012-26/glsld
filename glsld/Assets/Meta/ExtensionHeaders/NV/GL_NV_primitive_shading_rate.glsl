#define GL_NV_primitive_shading_rate 1

out gl_PerVertex {
    int gl_ShadingRateNV;
};

perprimitiveNV out gl_MeshPerPrimitiveNV {
    int           gl_ShadingRateNV;
    perviewNV int gl_ShadingRatePerViewNV[];
} gl_MeshPrimitivesNV[];
