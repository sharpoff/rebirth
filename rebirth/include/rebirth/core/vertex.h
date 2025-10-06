#pragma once

#include <rebirth/math/math.h>

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 tangent = vec4(0, 0, 0, 0);
    vec4 jointIndices = vec4(-1, -1, -1, -1);
    vec4 jointWeights = vec4(0, 0, 0, 0);
};