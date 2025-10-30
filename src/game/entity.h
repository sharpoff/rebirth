#pragma once

#include <Jolt/Jolt.h>

#include "Jolt/Physics/Body/BodyID.h"

#include "math/bounds.h"
#include "math/math.h"

#include "EASTL/string.h"

class Physics;

struct EntityCreateInfo
{
    eastl::string name = "Entity";
    Bounds        bounds{};
    vec3 scale = vec3(1.0f);
    JPH::BodyID   bodyId;
    bool          isStatic = true;
};

class Entity
{
public:
    void initialize(Physics *physics, const EntityCreateInfo &createInfo);

    void setMesh(int32_t meshId);
    void setOverrideMaterial(int32_t materialId);

    void setPosition(vec3 position);
    void setRotation(quat rotation);
    void setScale(vec3 scale);

    mat4 getTransform() const;
    vec3 getPosition() const;
    quat getRotation() const;
    vec3 getScale() const { return scale; }

    eastl::string getName() const { return name; }
    Bounds getBounds() const { return bounds; }
    JPH::BodyID   getBodyID() const { return bodyId; }
    int32_t       getMeshID() const { return meshId; }
    int32_t       getOverrideMaterialId() const { return overrideMaterialId; }
    bool          isStatic() const { return static_; }

    bool transformDirty = false;

protected:
    eastl::string name = "";
    Bounds        bounds{};
    JPH::BodyID   bodyId{};
    int32_t       meshId = -1;
    int32_t       overrideMaterialId = -1;
    bool          static_ = false;

    // position and rotation stored in physics bodyId
    vec3 scale = vec3(1.0f);

    Physics *physics;
};