// GL_EXT_samplerless_texture_functions

// Texture query functions (no sampler parameter)
int textureSize(texture1D _Texture, int _Lod);
ivec2 textureSize(texture2D _Texture, int _Lod);
ivec3 textureSize(texture3D _Texture, int _Lod);
ivec2 textureSize(textureCube _Texture, int _Lod);
ivec3 textureSize(textureCubeArray _Texture, int _Lod);
ivec2 textureSize(texture2DRect _Texture);
ivec2 textureSize(texture1DArray _Texture, int _Lod);
ivec3 textureSize(texture2DArray _Texture, int _Lod);
int textureSize(textureBuffer _Texture);
ivec2 textureSize(texture2DMS _Texture);
ivec3 textureSize(texture2DMSArray _Texture);

int textureSize(itexture1D _Texture, int _Lod);
ivec2 textureSize(itexture2D _Texture, int _Lod);
ivec3 textureSize(itexture3D _Texture, int _Lod);
ivec2 textureSize(itextureCube _Texture, int _Lod);
ivec3 textureSize(itextureCubeArray _Texture, int _Lod);
ivec2 textureSize(itexture2DRect _Texture);
ivec2 textureSize(itexture1DArray _Texture, int _Lod);
ivec3 textureSize(itexture2DArray _Texture, int _Lod);
int textureSize(itextureBuffer _Texture);
ivec2 textureSize(itexture2DMS _Texture);
ivec3 textureSize(itexture2DMSArray _Texture);

int textureSize(utexture1D _Texture, int _Lod);
ivec2 textureSize(utexture2D _Texture, int _Lod);
ivec3 textureSize(utexture3D _Texture, int _Lod);
ivec2 textureSize(utextureCube _Texture, int _Lod);
ivec3 textureSize(utextureCubeArray _Texture, int _Lod);
ivec2 textureSize(utexture2DRect _Texture);
ivec2 textureSize(utexture1DArray _Texture, int _Lod);
ivec3 textureSize(utexture2DArray _Texture, int _Lod);
int textureSize(utextureBuffer _Texture);
ivec2 textureSize(utexture2DMS _Texture);
ivec3 textureSize(utexture2DMSArray _Texture);

int textureQueryLevels(texture1D _Texture);
int textureQueryLevels(texture2D _Texture);
int textureQueryLevels(texture3D _Texture);
int textureQueryLevels(textureCube _Texture);
int textureQueryLevels(texture1DArray _Texture);
int textureQueryLevels(texture2DArray _Texture);
int textureQueryLevels(textureCubeArray _Texture);

int textureQueryLevels(itexture1D _Texture);
int textureQueryLevels(itexture2D _Texture);
int textureQueryLevels(itexture3D _Texture);
int textureQueryLevels(itextureCube _Texture);
int textureQueryLevels(itexture1DArray _Texture);
int textureQueryLevels(itexture2DArray _Texture);
int textureQueryLevels(itextureCubeArray _Texture);

int textureQueryLevels(utexture1D _Texture);
int textureQueryLevels(utexture2D _Texture);
int textureQueryLevels(utexture3D _Texture);
int textureQueryLevels(utextureCube _Texture);
int textureQueryLevels(utexture1DArray _Texture);
int textureQueryLevels(utexture2DArray _Texture);
int textureQueryLevels(utextureCubeArray _Texture);

int textureSamples(texture2DMS _Texture);
int textureSamples(texture2DMSArray _Texture);

int textureSamples(itexture2DMS _Texture);
int textureSamples(itexture2DMSArray _Texture);

int textureSamples(utexture2DMS _Texture);
int textureSamples(utexture2DMSArray _Texture);

// Texel lookup functions (no sampler parameter)
vec4 texelFetch(texture1D _Texture, int _P, int _Lod);
vec4 texelFetch(texture2D _Texture, ivec2 _P, int _Lod);
vec4 texelFetch(texture3D _Texture, ivec3 _P, int _Lod);
vec4 texelFetch(texture2DRect _Texture, ivec2 _P);
vec4 texelFetch(texture1DArray _Texture, ivec2 _P, int _Lod);
vec4 texelFetch(texture2DArray _Texture, ivec3 _P, int _Lod);
vec4 texelFetch(textureBuffer _Texture, int _P);
vec4 texelFetch(texture2DMS _Texture, ivec2 _P, int _Sample);
vec4 texelFetch(texture2DMSArray _Texture, ivec3 _P, int _Sample);

ivec4 texelFetch(itexture1D _Texture, int _P, int _Lod);
ivec4 texelFetch(itexture2D _Texture, ivec2 _P, int _Lod);
ivec4 texelFetch(itexture3D _Texture, ivec3 _P, int _Lod);
ivec4 texelFetch(itexture2DRect _Texture, ivec2 _P);
ivec4 texelFetch(itexture1DArray _Texture, ivec2 _P, int _Lod);
ivec4 texelFetch(itexture2DArray _Texture, ivec3 _P, int _Lod);
ivec4 texelFetch(itextureBuffer _Texture, int _P);
ivec4 texelFetch(itexture2DMS _Texture, ivec2 _P, int _Sample);
ivec4 texelFetch(itexture2DMSArray _Texture, ivec3 _P, int _Sample);

uvec4 texelFetch(utexture1D _Texture, int _P, int _Lod);
uvec4 texelFetch(utexture2D _Texture, ivec2 _P, int _Lod);
uvec4 texelFetch(utexture3D _Texture, ivec3 _P, int _Lod);
uvec4 texelFetch(utexture2DRect _Texture, ivec2 _P);
uvec4 texelFetch(utexture1DArray _Texture, ivec2 _P, int _Lod);
uvec4 texelFetch(utexture2DArray _Texture, ivec3 _P, int _Lod);
uvec4 texelFetch(utextureBuffer _Texture, int _P);
uvec4 texelFetch(utexture2DMS _Texture, ivec2 _P, int _Sample);
uvec4 texelFetch(utexture2DMSArray _Texture, ivec3 _P, int _Sample);

vec4 texelFetchOffset(texture1D _Texture, int _P, int _Lod, int _Offset);
vec4 texelFetchOffset(texture2D _Texture, ivec2 _P, int _Lod, ivec2 _Offset);
vec4 texelFetchOffset(texture3D _Texture, ivec3 _P, int _Lod, ivec3 _Offset);
vec4 texelFetchOffset(texture2DRect _Texture, ivec2 _P, ivec2 _Offset);
vec4 texelFetchOffset(texture1DArray _Texture, ivec2 _P, int _Lod, int _Offset);
vec4 texelFetchOffset(texture2DArray _Texture, ivec3 _P, int _Lod, ivec2 _Offset);

ivec4 texelFetchOffset(itexture1D _Texture, int _P, int _Lod, int _Offset);
ivec4 texelFetchOffset(itexture2D _Texture, ivec2 _P, int _Lod, ivec2 _Offset);
ivec4 texelFetchOffset(itexture3D _Texture, ivec3 _P, int _Lod, ivec3 _Offset);
ivec4 texelFetchOffset(itexture2DRect _Texture, ivec2 _P, ivec2 _Offset);
ivec4 texelFetchOffset(itexture1DArray _Texture, ivec2 _P, int _Lod, int _Offset);
ivec4 texelFetchOffset(itexture2DArray _Texture, ivec3 _P, int _Lod, ivec2 _Offset);

uvec4 texelFetchOffset(utexture1D _Texture, int _P, int _Lod, int _Offset);
uvec4 texelFetchOffset(utexture2D _Texture, ivec2 _P, int _Lod, ivec2 _Offset);
uvec4 texelFetchOffset(utexture3D _Texture, ivec3 _P, int _Lod, ivec3 _Offset);
uvec4 texelFetchOffset(utexture2DRect _Texture, ivec2 _P, ivec2 _Offset);
uvec4 texelFetchOffset(utexture1DArray _Texture, ivec2 _P, int _Lod, int _Offset);
uvec4 texelFetchOffset(utexture2DArray _Texture, ivec3 _P, int _Lod, ivec2 _Offset);
