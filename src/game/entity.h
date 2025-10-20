#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "glm/ext/matrix_transform.hpp"
#include "math/bounds.h"
#include "math/math.h"

class Entity
{
public:
    vec3 position = vec3(0.0f);
    vec3 scale = vec3(1.0f);
    quat rotation = glm::identity<quat>();

    mat4 getTransform() const
    {
        return glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale);
    };

    Bounds      bounds{};
    int32_t     overrideMaterialId = -1;
    int32_t     meshId = -1;
    JPH::BodyID bodyId;

    bool isStatic = false;
};