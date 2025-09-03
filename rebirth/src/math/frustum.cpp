#include <rebirth/math/frustum.h>
#include <rebirth/math/sphere.h>
#include <rebirth/util/common.h>
#include <rebirth/util/logger.h>

namespace rebirth
{

Frustum::Frustum(const Camera &camera)
{
    const vec3 position = camera.position;
    const vec3 right = camera.right;
    const vec3 front = camera.front;
    const vec3 up = camera.up;

    const float near = camera.near;
    const float far = camera.far;
    const float fov = camera.fov;
    const float aspect = camera.aspectRatio;

    const float halfVSide = far * tanf(fov * .5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = far * front;

    nearPlane = {position + near * front, front};
    farPlane = {position + frontMultFar, -front};
    rightPlane = {position, glm::cross(frontMultFar - right * halfHSide, up)};
    leftPlane = {position, glm::cross(up, frontMultFar + right * halfHSide)};
    topPlane = {position, glm::cross(right, frontMultFar - up * halfVSide)};
    bottomPlane = {position, glm::cross(frontMultFar + up * halfVSide, right)};
}

bool isInFrustum(const Frustum &frustum, const AABB &aabb, const Transform &transform)
{
    util::logWarn("isInFrustum() not working!");
    // TODO: make it work
    // return isOnPlane(aabb, frustum.topPlane) && isOnPlane(aabb, frustum.bottomPlane) &&
    //        isOnPlane(aabb, frustum.leftPlane) && isOnPlane(aabb, frustum.rightPlane) &&
    //        isOnPlane(aabb, frustum.nearPlane) && isOnPlane(aabb, frustum.farPlane);
    return true;
}

bool isInFrustum(const Frustum &frustum, const Sphere &sphere, const Transform &transform)
{
    const glm::vec3 scale = transform.getScale();
    const glm::vec3 center{transform.getModelMatrix() * glm::vec4(sphere.center, 1.f)};
    const float maxScale = std::max(std::max(scale.x, scale.y), scale.z);

    Sphere worldSphere(center, sphere.radius * (maxScale * 0.5f));

    return (
        isOnPlane(worldSphere, frustum.leftPlane) && isOnPlane(worldSphere, frustum.rightPlane) &&
        isOnPlane(worldSphere, frustum.farPlane) && isOnPlane(worldSphere, frustum.nearPlane) &&
        isOnPlane(worldSphere, frustum.topPlane) && isOnPlane(worldSphere, frustum.bottomPlane)
    );
}

bool isOnPlane(const AABB &aabb, const Plane &plane)
{
    vec3 center = (aabb.max + aabb.min) * 0.5f;
    vec3 extents = aabb.max - center;

    // Compute the projection interval radius of aabb onto L(t) = center + t * plane.normal
    const float radius = extents.x * std::abs(plane.normal.x) +
                         extents.y * std::abs(plane.normal.y) +
                         extents.z * std::abs(plane.normal.z);

    // Compute distance of box center from plane
    float distance = glm::dot(plane.normal, center) - plane.distance;

    return -radius <= std::abs(distance);
}

bool isOnPlane(const Sphere &sphere, const Plane &plane)
{
    float distance = glm::dot(plane.normal, sphere.center) - plane.distance;
    return distance > -sphere.radius;
}

} // namespace rebirth