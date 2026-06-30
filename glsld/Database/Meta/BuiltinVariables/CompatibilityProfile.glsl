// ===================================================================
// GLSL Built-in Variables and Constants (Full Extraction)
// ===================================================================

// --- Built-In Language Variables ---

// --- Compatibility Profile Built-In Language Variables ---
out gl_PerVertex { // part of the gl_PerVertex block described in 7.1;
    // in addition to other gl_PerVertex members...;
    vec4  gl_ClipVertex;
    vec4  gl_FrontColor;
    vec4  gl_BackColor;
    vec4  gl_FrontSecondaryColor;
    vec4  gl_BackSecondaryColor;
    vec4  gl_TexCoord[];
    float gl_FogFragCoord;
};

out gl_PerVertex {
    vec4 gl_Position;    // will use gl_Position;
    vec4 gl_FrontColor;  // will consume gl_color in the fragment shader;
    vec4 gl_BackColor;
    vec4 gl_TexCoord[3]; // 3 elements of gl_TexCoord will be used;
}; // no other aspects of the fixed interface will be used;

in  vec4 gl_TexCoord[3];
out vec4 gl_TexCoord[4];

in gl_PerVertex { // part of the gl_PerVertex block described in 7.1;
    // in addition to other gl_PerVertex members...;
    vec4  gl_ClipVertex;
    vec4  gl_FrontColor;
    vec4  gl_BackColor;
    vec4  gl_FrontSecondaryColor;
    vec4  gl_BackSecondaryColor;
    vec4  gl_TexCoord[];
    float gl_FogFragCoord;
} gl_in[];

in gl_PerVertex {
    vec4 gl_ClipVertex;
    vec4 gl_FrontColor;
} gl_in[]; // must be present and must be "gl_in[]";

in gl_PerFragment {
    in float gl_FogFragCoord;
    in vec4  gl_TexCoord[];
    in vec4  gl_Color;
    in vec4  gl_SecondaryColor;
};
out vec4 gl_FragColor;
out vec4 gl_FragData[gl_MaxDrawBuffers];

// --- Compatibility Profile Vertex Shader Built-In Inputs ---
in vec4  gl_Color;
in vec4  gl_SecondaryColor;
in vec3  gl_Normal;
in vec4  gl_Vertex;
in vec4  gl_MultiTexCoord0;
in vec4  gl_MultiTexCoord1;
in vec4  gl_MultiTexCoord2;
in vec4  gl_MultiTexCoord3;
in vec4  gl_MultiTexCoord4;
in vec4  gl_MultiTexCoord5;
in vec4  gl_MultiTexCoord6;
in vec4  gl_MultiTexCoord7;
in float gl_FogCoord;

// --- Compatibility Profile Built-In Constants ---
const int gl_MaxTextureUnits  = 2;
const int gl_MaxTextureCoords = 8;
const int gl_MaxClipPlanes    = 8;
const int gl_MaxVaryingFloats = 60;

// --- Built-In Uniform State ---
//
// Depth range in window coordinates,
// section 13.6.1 "Controlling the Viewport" in the
// OpenGL Specification.
//
// Note: Depth-range state is only for viewport 0.
//
struct gl_DepthRangeParameters {
    float near; // n;
    float far;  // f;
    float diff; // f - n;
};
uniform gl_DepthRangeParameters gl_DepthRange;
uniform int                     gl_NumSamples;

// --- Compatibility Profile State ---
//
// compatibility profile only
//
uniform mat4 gl_ModelViewMatrix;
uniform mat4 gl_ProjectionMatrix;
uniform mat4 gl_ModelViewProjectionMatrix;
uniform mat4 gl_TextureMatrix[gl_MaxTextureCoords];

//
// compatibility profile only
//
uniform mat3 gl_NormalMatrix; // transpose of the inverse of the;
                              // upper leftmost 3x3 of gl_ModelViewMatrix

uniform mat4 gl_ModelViewMatrixInverse;
uniform mat4 gl_ProjectionMatrixInverse;
uniform mat4 gl_ModelViewProjectionMatrixInverse;
uniform mat4 gl_TextureMatrixInverse[gl_MaxTextureCoords];

uniform mat4 gl_ModelViewMatrixTranspose;
uniform mat4 gl_ProjectionMatrixTranspose;
uniform mat4 gl_ModelViewProjectionMatrixTranspose;
uniform mat4 gl_TextureMatrixTranspose[gl_MaxTextureCoords];

uniform mat4 gl_ModelViewMatrixInverseTranspose;
uniform mat4 gl_ProjectionMatrixInverseTranspose;
uniform mat4 gl_ModelViewProjectionMatrixInverseTranspose;
uniform mat4 gl_TextureMatrixInverseTranspose[gl_MaxTextureCoords];

//
// compatibility profile only
//
uniform float gl_NormalScale;

//
// compatibility profile only
//
uniform vec4 gl_ClipPlane[gl_MaxClipPlanes];

//
// compatibility profile only
//
struct gl_PointParameters {
    float size;
    float sizeMin;
    float sizeMax;
    float fadeThresholdSize;
    float distanceConstantAttenuation;
    float distanceLinearAttenuation;
    float distanceQuadraticAttenuation;
};

uniform gl_PointParameters gl_Point;

//
// compatibility profile only
//
struct gl_MaterialParameters {
    vec4  emission;  // Ecm;
    vec4  ambient;   // Acm;
    vec4  diffuse;   // Dcm;
    vec4  specular;  // Scm;
    float shininess; // Srm;
};
uniform gl_MaterialParameters gl_FrontMaterial;
uniform gl_MaterialParameters gl_BackMaterial;

//
// compatibility profile only
//
struct gl_LightSourceParameters {
    vec4  ambient;              // Acli;
    vec4  diffuse;              // Dcli;
    vec4  specular;             // Scli;
    vec4  position;             // Ppli;
    vec4  halfVector;           // Derived: Hi;
    vec3  spotDirection;        // Sdli;
    float spotExponent;         // Srli;
    float spotCutoff;           // Crli;
                                // (range: [0.0,90.0], 180.0)
    float spotCosCutoff;        // Derived: cos(Crli);
                                // (range: [1.0,0.0],-1.0)
    float constantAttenuation;  // K0;
    float linearAttenuation;    // K1;
    float quadraticAttenuation; // K2;
};

uniform gl_LightSourceParameters gl_LightSource[gl_MaxLights];

struct gl_LightModelParameters {
    vec4 ambient;                  // Acs;
};

uniform gl_LightModelParameters gl_LightModel;

//
// compatibility profile only
//
// Derived state from products of light and material.
//

struct gl_LightModelProducts {
    vec4 sceneColor; // Derived. Ecm + Acm * Acs;
};

uniform gl_LightModelProducts gl_FrontLightModelProduct;
uniform gl_LightModelProducts gl_BackLightModelProduct;

struct gl_LightProducts {
    vec4 ambient;  // Acm * Acli;
    vec4 diffuse;  // Dcm * Dcli;
    vec4 specular; // Scm * Scli;
};

uniform gl_LightProducts gl_FrontLightProduct[gl_MaxLights];
uniform gl_LightProducts gl_BackLightProduct[gl_MaxLights];

//
// compatibility profile only
//
uniform vec4 gl_TextureEnvColor[gl_MaxTextureUnits];
uniform vec4 gl_EyePlaneS[gl_MaxTextureCoords];
uniform vec4 gl_EyePlaneT[gl_MaxTextureCoords];
uniform vec4 gl_EyePlaneR[gl_MaxTextureCoords];
uniform vec4 gl_EyePlaneQ[gl_MaxTextureCoords];
uniform vec4 gl_ObjectPlaneS[gl_MaxTextureCoords];
uniform vec4 gl_ObjectPlaneT[gl_MaxTextureCoords];
uniform vec4 gl_ObjectPlaneR[gl_MaxTextureCoords];
uniform vec4 gl_ObjectPlaneQ[gl_MaxTextureCoords];

//
// compatibility profile only
//
struct gl_FogParameters {
    vec4  color;
    float density;
    float start;
    float end;
    float scale; // Derived: 1.0 / (end - start);
};

uniform gl_FogParameters gl_Fog;

// --- Redeclaring Built-In Blocks ---
out gl_PerVertex {
    vec4  gl_Position;  // will use gl_Position;
    float gl_PointSize; // will use gl_PointSize;
    vec4  t;            // error, only gl_PerVertex members allowed;
}; // no other members of gl_PerVertex will be used

out layout(xfb_buffer = 1, xfb_stride = 16) gl_PerVertex {
    vec4 gl_Position;
    layout(xfb_offset = 0) float gl_ClipDistance[4];
};
