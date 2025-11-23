#include "core/util.h"
#include <glm/gtx/intersect.hpp>

namespace util
{
    vec3 mouseToWorldDirection(const vec2 &mouseCoords, const vec2 &screenDim, const mat4 &cameraView, const mat4 &cameraProjection)
    {
        double ndc_x = (2.0 * mouseCoords.x / screenDim.x) - 1.0;
        double ndc_y = (2.0 * mouseCoords.y / screenDim.y) - 1.0;

        vec4 clipSpace = vec4(ndc_x, ndc_y, 0.0, 1.0);
        vec4 worldSpace = glm::inverse(cameraProjection * cameraView) * clipSpace;

        vec3 direction = vec3(glm::normalize(worldSpace));
        worldSpace /= worldSpace.w; // perspective division

        return direction;
    }

    bool rayIntersectBox(const vec3 &rayOrigin, const vec3 &rayDirection, const vec3 &boxMin, const vec3 &boxMax)
    {
        vec3 tMin = (boxMin - rayOrigin) / rayDirection;
        vec3 tMax = (boxMax - rayOrigin) / rayDirection;
        vec3 t1 = min(tMin, tMax);
        vec3 t2 = max(tMin, tMax);

        float tNear = fmax(fmax(t1.x, t1.y), t1.z);
        float tFar = fmax(fmax(t2.x, t2.y), t2.z);

        return tNear <= tFar;
    }

    bool rayIntersectVertex(const vec3 &rayOrigin, const vec3 &rayDirection, const vec3 &v0, const vec3 &v1, const vec3 &v2, float &distance)
    {
        const float EPSILON = 1e-8f;

        vec3  edge1 = v1 - v0;
        vec3  edge2 = v2 - v0;
        vec3  h = glm::cross(rayDirection, edge2);

        float det = glm::dot(edge1, h);
        if (fabs(det) < EPSILON) {
            return false; // ray is parallel to the triangle.
        }

        float invDet = 1.0f / det;
        vec3  s = rayOrigin - v0;
        float u = invDet * glm::dot(s, h);
        if (u < 0.0f || u > 1.0f) {
            return false;
        }
        vec3  q = glm::cross(s, edge1);
        float v = invDet * glm::dot(rayDirection, q);
        if (v < 0.0f || u + v > 1.0f) {
            return false;
        }
        distance = invDet * glm::dot(edge2, q); // distance along the ray to the intersection
        return distance > EPSILON;
    }
} // namespace util