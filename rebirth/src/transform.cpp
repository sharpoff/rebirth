#include <rebirth/transform.h>

namespace rebirth
{

void Transform::rotate(float angle, vec3 axis) { rotation_ = glm::angleAxis(angle, axis) * rotation_; }

void Transform::rotate(quat q) { rotation_ = q * rotation_; }

void Transform::translate(vec3 pos) { position_ += pos; }

void Transform::scale(vec3 scale) { scale_ *= scale; }

mat4 Transform::getModelMatrix() const
{
    glm::mat4 position = glm::translate(position_);
    glm::mat4 rotation = glm::toMat4(rotation_);
    glm::mat4 scale = glm::scale(scale_);

    return position * rotation * scale;
}

} // namespace rebirth