#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "math/bounds.h"
#include "math/math.h"

class GameObject
{
public:
    vec3 position = vec3(0.0f);
    vec3 scale = vec3(1.0f);
    quat rotation = glm::identity<quat>();

    Bounds      bounds{};
    int32_t     meshId = -1;
    JPH::BodyID bodyId;
};