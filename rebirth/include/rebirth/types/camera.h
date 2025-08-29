#pragma once

#include <SDL3/SDL.h>
#include <map>
#include <rebirth/math/math.h>
#include <rebirth/math/transform.h>

namespace rebirth
{

class Camera
{
public:
    void update(float deltaTime);
    void handleEvent(SDL_Event event, float deltaTime);

    void setPosition(vec3 position) { transform.setPosition(position); }
    void setMovementSpeed(float speed) { this->movementSpeed = speed; }
    void setRotationSpeed(float speed) { this->rotationSpeed = speed; }
    void setPerspective(float fov, float aspectRatio, float near, float far);

    const float &getFov() const { return fov; }
    const float &getAspectRatio() const { return aspectRatio; }
    const float &getNear() const { return near; }
    const float &getFar() const { return far; }

    const Transform &getTransform() const { return transform; }
    const float &getMovementSpeed() const { return movementSpeed; }
    const float &getRotationSpeed() const { return rotationSpeed; }
    const mat4 &getProjection() const { return projection; }
    const mat4 getViewMatrix() const;

private:
    mat4 projection = mat4(1.0f);
    Transform transform;
    glm::vec3 velocity = vec3(0.0f);

    float yaw, pitch;

    float fov;
    float aspectRatio;
    float near, far;

    float movementSpeed = 3.0;
    float rotationSpeed = 1.0;

    std::map<unsigned int, bool> keys;
};

} // namespace rebirth