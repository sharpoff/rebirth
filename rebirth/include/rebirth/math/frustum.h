#pragma once

#include <rebirth/math/aabb.h>
#include <rebirth/math/sphere.h>
#include <rebirth/types/camera.h>

namespace rebirth
{

// Credit: https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
struct Plane
{
    Plane() = default;
    Plane(const vec3 &point, const vec3 &normal)
        : normal(glm::normalize(normal)), distance(glm::dot(normal, point)) {};

    vec3 normal = {0.0f, 1.0f, 0.0f};

    // distance from origin to the nearest point in the plane
    float distance = 0.0f;
};

struct Frustum
{
    Frustum(const Camera &camera);

    Plane topPlane;
    Plane bottomPlane;

    Plane rightPlane;
    Plane leftPlane;

    Plane farPlane;
    Plane nearPlane;
};

bool isInFrustum(const Frustum &frustum, const AABB &aabb, const Transform &transform);
bool isInFrustum(const Frustum &frustum, const Sphere &sphere, const Transform &transform);

bool isOnPlane(const AABB &aabb, const Plane &plane);
bool isOnPlane(const Sphere &sphere, const Plane &plane);

} // namespace rebirth