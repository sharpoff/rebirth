#pragma once

#include "math/bounds.h"
#include "math/math.h"

#include "EASTL/string.h"

class Entity
{
public:
    Entity() = default;
    virtual ~Entity();

    virtual void update(float deltaTime) = 0;
    virtual void processInput(float deltaTime) = 0;

    void setMesh(uint32_t meshId) { m_meshId = meshId; };
    void setCustomMaterial(uint32_t materialId) { m_customMaterialId = materialId; };

    mat4          getTransform() const { return m_transform; };
    eastl::string getName() const { return m_name; }
    Bounds        getBounds() const { return m_bounds; }
    uint32_t      getMeshID() const { return m_meshId; }
    uint32_t      getCustomMaterialId() const { return m_customMaterialId; }

protected:
    eastl::string m_name = "";
    Bounds        m_bounds = {};
    mat4          m_transform = mat4(1.0f);
    uint32_t      m_meshId = 0;
    uint32_t      m_customMaterialId = 0;
};