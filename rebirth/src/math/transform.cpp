#include <rebirth/math/transform.h>

Transform::Transform(mat4 matrix)
{
    vec3 skew;
    vec4 perspective;

    glm::decompose(matrix, scale_, rotation_, position_, skew, perspective);
}

void Transform::rotate(float angle, vec3 axis)
{
    rotation_ = glm::normalize(glm::angleAxis(angle, axis) * rotation_);
}

void Transform::rotate(quat q) { rotation_ = glm::normalize(q * rotation_); }

void Transform::translate(vec3 pos) { position_ += pos; }

void Transform::scale(vec3 scale) { scale_ *= scale; }

mat4 Transform::getModelMatrix() const
{
    glm::mat4 position = glm::translate(mat4(1.0f), position_);
    glm::mat4 rotation = glm::toMat4(rotation_);
    glm::mat4 scale = glm::scale(mat4(1.0f), scale_);

    return position * rotation * scale;
}