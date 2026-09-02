#define GL_NV_shader_texture_footprint 1

struct gl_TextureFootprint2DNV {
    uvec2 anchor;
    uvec2 offset;
    uvec2 mask;
    uint  lod;
    uint  granularity;
};

struct gl_TextureFootprint3DNV {
    uvec3 anchor;
    uvec3 offset;
    uvec2 mask;
    uint  lod;
    uint  granularity;
};

bool textureFootprintNV(sampler2D _Sampler, vec2 _Pos, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintNV(isampler2D _Sampler, vec2 _Pos, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintNV(usampler2D _Sampler, vec2 _Pos, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintNV(sampler3D _Sampler, vec3 _Pos, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);
bool textureFootprintNV(isampler3D _Sampler, vec3 _Pos, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);
bool textureFootprintNV(usampler3D _Sampler, vec3 _Pos, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);

bool textureFootprintClampNV(sampler2D _Sampler, vec2 _Pos, float _LodClamp, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintClampNV(isampler2D _Sampler, vec2 _Pos, float _LodClamp, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintClampNV(usampler2D _Sampler, vec2 _Pos, float _LodClamp, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintClampNV(sampler3D _Sampler, vec3 _Pos, float _LodClamp, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);
bool textureFootprintClampNV(isampler3D _Sampler, vec3 _Pos, float _LodClamp, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);
bool textureFootprintClampNV(usampler3D _Sampler, vec3 _Pos, float _LodClamp, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);

bool textureFootprintLodNV(sampler2D _Sampler, vec2 _Pos, float _Lod, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintLodNV(isampler2D _Sampler, vec2 _Pos, float _Lod, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintLodNV(usampler2D _Sampler, vec2 _Pos, float _Lod, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintLodNV(sampler3D _Sampler, vec3 _Pos, float _Lod, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);
bool textureFootprintLodNV(isampler3D _Sampler, vec3 _Pos, float _Lod, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);
bool textureFootprintLodNV(usampler3D _Sampler, vec3 _Pos, float _Lod, int _Granularity, bool _Coarse, out gl_TextureFootprint3DNV _Footprint);

bool textureFootprintGradNV(sampler2D _Sampler, vec2 _Pos, vec2 _Dx, vec2 _Dy, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintGradNV(isampler2D _Sampler, vec2 _Pos, vec2 _Dx, vec2 _Dy, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintGradNV(usampler2D _Sampler, vec2 _Pos, vec2 _Dx, vec2 _Dy, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);

bool textureFootprintGradClampNV(sampler2D _Sampler, vec2 _Pos, vec2 _Dx, vec2 _Dy, float _Lodclamp, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintGradClampNV(isampler2D _Sampler, vec2 _Pos, vec2 _Dx, vec2 _Dy, float _Lodclamp, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
bool textureFootprintGradClampNV(usampler2D _Sampler, vec2 _Pos, vec2 _Dx, vec2 _Dy, float _Lodclamp, int _Granularity, bool _Coarse, out gl_TextureFootprint2DNV _Footprint);
