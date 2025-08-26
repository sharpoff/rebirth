#pragma once

#include <rebirth/math.h>

namespace rebirth
{

class Transform
{
public:
    Transform() = default;
    Transform(vec3 position, quat rotation, vec3 scale) : position_(position), rotation_(rotation), scale_(scale) {};

    void rotate(float angle, vec3 axis);
    void rotate(quat q);

    void translate(vec3 pos);

    void scale(vec3 scale);

    mat4 getModelMatrix() const;

    vec3 getPosition() const { return position_; };
    quat getRotation() const { return rotation_; };
    vec3 getScale() const { return scale_; };

    void setPosition(vec3 position) { position_ = position; };
    void setRotation(quat rotation) { rotation_ = rotation; };
    void setScale(vec3 scale) { scale_ = scale; };

private:
    vec3 position_ = vec3(0.0);
    quat rotation_ = quat(1.0, 0.0, 0.0, 0.0);
    vec3 scale_ = vec3(1.0);
};

inline mat4 operator*(const Transform &t1, const Transform &t2) { return t1.getModelMatrix() * t2.getModelMatrix(); }

} // namespace rebirth