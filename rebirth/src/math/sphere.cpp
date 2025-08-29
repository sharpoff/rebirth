#include <rebirth/math/sphere.h>
#include <rebirth/types/mesh.h>

namespace rebirth
{

// credit: https://yearlyboar.wordpress.com/2015/01/31/calculating-bounding-spheres/
Sphere calculateBoundingSphere(std::vector<Vertex> vertices)
{
    vec3 min = vec3(std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min());

    for (auto &vert : vertices) {
        min.x = std::min(min.x, vert.position.x);
        min.y = std::min(min.y, vert.position.y);
        min.z = std::min(min.z, vert.position.z);

        max.x = std::max(max.x, vert.position.x);
        max.y = std::max(max.y, vert.position.y);
        max.z = std::max(max.z, vert.position.z);
    }

    vec3 diff = max - min;
    float diameter = std::max(diff.x, std::max(diff.y, diff.z));

    vec3 center = (max + min) * 0.5f;
    float radius = diameter / 2.0f;

    // check if all vertices within the sphere
    float squareRadius = radius * radius;
    for (auto &vert : vertices) {
        vec3 position = vert.position;

        vec3 direction = glm::normalize(position - center);
        float squareDistance = glm::length2(direction);

        if (squareDistance > squareRadius) {
            float distance = std::sqrt(squareDistance);

            float difference = distance - radius;

            float newDiameter = 2 * radius + difference;
            radius = newDiameter / 2;
            squareRadius = radius * radius;

            difference /= 2;

            center += difference * direction;
        }
    }

    return Sphere(center, radius);
}

} // namespace rebirth