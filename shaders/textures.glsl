#ifndef TEXTURES
#define TEXTURES

layout (binding = 1) uniform sampler2D textures[];
layout (binding = 1) uniform samplerCube textureCubes[];

#define TEX(id, uv) texture(textures[nonuniformEXT(id)], uv)

#endif