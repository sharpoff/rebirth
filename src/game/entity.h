#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "glm/ext/matrix_transform.hpp"
#include "math/bounds.h"
#include "math/math.h"

#include "EASTL/string.h"

class Physics;

class Entity
{
public:
    Entity(Physics *physics);

    virtual void update(float deltaTime);

    void setName(eastl::string name);
    void setBounds(Bounds bounds);
    void setOverrideMaterial(int32_t materialId);
    void setMesh(int32_t meshId);
    void setBody(JPH::BodyID bodyId, bool isStatic = false);
    void setStatic(bool isStatic = true);

    void setPosition(vec3 position);
    void setRotation(quat rotation);
    void setScale(vec3 scale);

    mat4 getTransform() const;
    vec3 getPosition() const { return position; }
    quat getRotation() const { return rotation; }
    vec3 getScale() const { return scale; }

    eastl::string getName() const { return name; }
    Bounds        getBounds() const { return bounds; }
    int32_t       getOverrideMaterialId() const { return overrideMaterialId; }
    JPH::BodyID   getBodyID() const { return bodyId; }
    int32_t       getMeshID() const { return meshId; }
    bool          isStatic() const { return static_; }

    bool transformDirty = false;

private:
    eastl::string name = "";
    Bounds        bounds{};
    int32_t       overrideMaterialId = -1;
    int32_t       meshId = -1;
    JPH::BodyID   bodyId{};
    bool          static_ = false;

    vec3 position = vec3(0.0f);
    quat rotation = glm::identity<quat>();
    vec3 scale = vec3(1.0f);

    Physics *physics;
};