#pragma once

#include "math/math.h"
#include "core/vertex.h"

#include "EASTL/vector.h"

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

namespace util
{
    vec3 mouseToWorldDirection(const vec2 &mouseCoords, const vec2 &screenDim, const mat4 &cameraView, const mat4 &cameraProjection);

    bool rayIntersectBox(const vec3 &rayOrigin, const vec3 &rayDirection, const vec3 &boxMin, const vec3 &boxMax);
    bool rayIntersectVertex(const vec3 &rayOrigin, const vec3 &rayDirection, const vec3 &v0, const vec3 &v1, const vec3 &v2, float &distance);

    // primitive generation
    eastl::vector<Vertex> generateRingVertices(float sphereRadius, float ringThickness, int segments, int thicknessSegments);
    eastl::vector<uint32_t> generateRingIndices(int segments, int thicknessSegments);
    eastl::vector<Vertex> generateSphereVertices(float radius, int segments);
    eastl::vector<uint32_t> generateSphereIndices(int segments);
    eastl::vector<Vertex> generateConeVertices(float radius, float height, int segments);
    eastl::vector<uint32_t> generateConeIndices(int segments);
    eastl::vector<Vertex> generateCylinderVertices(float radius, float height, int subdivisions);
    eastl::vector<uint32_t> generateCylinderIndices(int subdivisions);
    eastl::vector<Vertex> generateCubeVertices();
    eastl::vector<uint32_t> generateCubeIndices();
}