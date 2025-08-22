#pragma once

#include <rebirth/math.h>

namespace rebirth
{

class Transform
{
public:
    Transform() = default;
    Transform(vec3 position, quat orientation, vec3 scale);

    void rotate(float angle, vec3 axis);
    void rotate(quat q);

    void translate(vec3 pos);

    void scale(vec3 scale);

    mat4 getModelMatrix() const;

    vec3 getPosition() const { return m_position; };
    quat getRotation() const { return m_rotation; };
    vec3 getScale() const { return m_scale; };

    void setTranslation(vec3 pos) { m_position = pos; };
    void setRotation(quat rotation) { m_rotation = rotation; };
    void setScale(vec3 scale) { m_scale = scale; };

private:
    vec3 m_position = vec3(0.0);
    quat m_rotation = quat(1.0, 0.0, 0.0, 0.0);
    vec3 m_scale = vec3(1.0);
};

} // namespace rebirth