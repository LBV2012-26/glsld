out gl_PerVertex {
    int gl_ShadingRateNV;
};

perprimitiveNV out gl_MeshPerPrimitiveNV {
    int           gl_ShadingRateNV;
    perviewNV int gl_ShadingRatePerViewNV[];
} gl_MeshPrimitivesNV[];
