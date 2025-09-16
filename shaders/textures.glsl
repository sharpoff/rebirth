#ifndef TEXTURES_GLSL
#define TEXTURES_GLSL

layout (binding = 1) uniform sampler1D texture1Ds[];
layout (binding = 1) uniform sampler2D texture2Ds[];
layout (binding = 1) uniform sampler3D texture3Ds[];
layout (binding = 1) uniform samplerCube textureCubes[];

#define TEX_1D(id, uv) texture(texture1Ds[nonuniformEXT(id)], uv)
#define TEX_2D(id, uv) texture(texture2Ds[nonuniformEXT(id)], uv)
#define TEX_3D(id, uv) texture(texture3Ds[nonuniformEXT(id)], uv)
#define TEX_CUBE(id, uv) texture(textureCubes[nonuniformEXT(id)], uv)

#endif