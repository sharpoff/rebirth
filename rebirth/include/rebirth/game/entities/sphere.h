#pragma once

#include <rebirth/types/scene.h>
#include <rebirth/game/entity.h>
#include <rebirth/math/bounds.h>

class Renderer;

struct Sphere : Entity
{
    Sphere(ModelID modelId, float radius, Transform transform, bool isStatic);

    void draw(Renderer &renderer) override;

private:
    ModelID modelId = ModelID::Invalid;
    Bounds bounds{};
    vec3 scale = vec3(1.0f);
};