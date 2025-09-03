#pragma once

#include <rebirth/math/aabb.h>
#include <rebirth/types/scene.h>

using namespace rebirth;

namespace rebirth
{

struct Object
{
    Object(Scene &scene, Transform transform = Transform(), std::string name = "Object")
        : name(name), scene(scene), transform(transform) {};

    std::string name = "Object";

    Scene &scene;
    RigidBodyID rigidBodyId = -1;
    Transform transform = Transform();
    AABB aabb;
};

} // namespace rcg