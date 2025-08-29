#pragma once

#include <rebirth/math/math.h>

namespace rebirth
{

class Transform
{
public:
    Transform() = default;
    Transform(vec3 position, quat rotation, vec3 scale)
        : position_(position), rotation_(rotation), scale_(scale) {};
    Transform(mat4 matrix);

    void rotate(float angle, vec3 axis);
    void rotate(quat q);

    void translate(vec3 pos);

    void scale(vec3 scale);

    mat4 getModelMatrix() const;

    const vec3 &getPosition() const { return position_; };
    const quat &getRotation() const { return rotation_; };
    const vec3 &getScale() const { return scale_; };

    const vec3 getFront() const { return rotation_ * vec3(0.0f, 0.0f, -1.0f); }
    const vec3 getUp() const { return rotation_ * vec3(0.0f, 1.0f, 0.0f); }
    const vec3 getRight() const { return rotation_ * vec3(1.0f, 0.0f, 0.0f); }

    void setPosition(vec3 position) { position_ = position; };
    void setRotation(quat rotation) { rotation_ = rotation; };
    void setRotation(mat4 rotation) { rotation_ = glm::quat(rotation); };
    void setScale(vec3 scale) { scale_ = scale; };

private:
    vec3 position_ = vec3(0.0);
    quat rotation_ = glm::identity<quat>();
    vec3 scale_ = vec3(1.0);
};

inline Transform operator*(const Transform &t1, const Transform &t2)
{
    return Transform(t1.getModelMatrix() * t2.getModelMatrix());
}

} // namespace rebirth