#pragma once

#include <SDL3/SDL.h>
#include <map>
#include <rebirth/math.h>

namespace rebirth
{

class Camera
{
public:
    void update(float deltaTime);
    void handleEvent(SDL_Event event, float deltaTime);

    void setPosition(vec3 position) { this->position = position; }
    void setMovementSpeed(float speed) { this->movementSpeed = speed; }
    void setRotationSpeed(float speed) { this->rotationSpeed = speed; }
    void setProjection(mat4 proj) { projection = proj; }

    vec3 getPosition() { return position; }
    float getMovementSpeed() { return movementSpeed; }
    float getRotationSpeed() { return rotationSpeed; }
    mat4 getProjection() { return projection; }
    mat4 getViewMatrix();

private:
    mat4 getRotationMatrix();

    mat4 projection = mat4(1.0f);
    vec3 position = vec3(0.0f);
    glm::vec3 velocity = vec3(0.0f);
    float pitch = 0.0f;
    float yaw = 0.0f;

    float movementSpeed = 3.0;
    float rotationSpeed = 0.01;

    std::map<unsigned int, bool> keys;
};

} // namespace rebirth