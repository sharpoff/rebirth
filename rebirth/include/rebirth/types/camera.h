#pragma once

#include <SDL3/SDL.h>
#include <map>
#include <rebirth/math/math.h>
#include <rebirth/math/transform.h>

namespace rebirth
{

enum class CameraType
{
    FirstPerson,
    LookAt,
};

class Camera
{
public:
    void update(float deltaTime);
    void handleEvent(SDL_Event event, float deltaTime);

    void setPosition(vec3 position);
    void setPerspective(float fov, float aspectRatio, float near, float far);

    CameraType type = CameraType::FirstPerson;

    vec3 position = vec3();

    vec3 front = vec3(0.0f, 0.0f, -1.0f);
    vec3 right = vec3(1.0f, 0.0f, 0.0f);
    vec3 up = vec3(0.0f, 1.0f, 0.0f);

    mat4 projection = mat4(1.0f);
    mat4 view = mat4(1.0f);

    float movementSpeed = 3.0;
    float rotationSpeed = 1.0;

    float yaw = 0.0f;
    float pitch = 0.0f;

    float fov = 60.0f;
    float aspectRatio = 0.0f;
    float near = 0.1f, far = 100.0f;

private:
    void updateViewMatrix();

    std::map<unsigned int, bool> keys;
};

} // namespace rebirth