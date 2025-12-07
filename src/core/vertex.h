#pragma once

#include <math/common.h>

struct Vertex
{
    Vertex(vec3 position = vec3(0.0f), vec3 normal = vec3(1.0), vec2 uv = vec2(0.0f), vec4 tangent = vec4(0, 0, 0, 0), vec4 jointIndices = vec4(-1, -1, -1, -1), vec4 jointWeights = vec4(0, 0, 0, 0))
        : position(position), uv_x(uv.x()), uv_y(uv.y()), tangent(tangent), jointIndices(jointIndices), jointWeights(jointWeights) {}

    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 tangent;
    vec4 jointIndices;
    vec4 jointWeights;
};