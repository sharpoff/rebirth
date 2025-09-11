#pragma once

#include <rebirth/math/aabb.h>
#include <rebirth/types/scene.h>
#include <rebirth/game/entity.h>

class Renderer;

struct Box : Entity
{
    Box(ModelID modelId, Transform transform, bool isStatic);

    void draw(Renderer &renderer) override;

private:
    ModelID modelId = ModelID::Invalid;
    AABB aabb = AABB();
    vec3 scale = vec3(1.0f);
};