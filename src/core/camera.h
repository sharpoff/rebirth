#pragma once

#include <math/math.h>
#include "core/stl.h"

enum class CameraType
{
    FirstPerson,
    Orbit,
};

class Input;

class Camera
{
public:
    Camera(Input *input);
    ~Camera() = default;

    void update(float deltaTime);
    void processInput();

    void setPosition(vec3 position);
    void setPerspective(float fov, float aspectRatio, float near, float far);
    void setCameraType(CameraType type);
    void setKeyboardInput(bool mode) { keyboardInput = mode; }
    void setMouseInput(bool mode) { mouseInput = mode; }
    
    // Orbit camera parameters
    void setEyeUpOffset(float offset) { eyeUpOffset = offset; }
    void setEyeFrontOffset(float offset) { eyeFrontOffset = offset; }
    void setTargetUpOffset(float offset) { targetUpOffset = offset; }
    void setTargetFrontOffset(float offset) { targetFrontOffset = offset; }

    // getters
    const mat4 &getProjection() const { return projection; }
    const mat4 &getView() const { return view; }
    const vec3 &getPosition() const { return m_position; }
    quat getRotation() { return glm::angleAxis(glm::radians(yaw), vec3(0, 1, 0)); }
    const float &getFov() const { return fov; }

private:
    void updateViewMatrix();

    CameraType m_type = CameraType::FirstPerson;

    vec3 m_position = vec3();

    vec3 m_front = vec3(0, 0, -1);
    vec3 m_right = vec3(1, 0, 0);
    vec3 m_up = vec3(0, 1, 0);

    mat4 projection = mat4(1.0f);
    mat4 view = mat4(1.0f);

    float movementSpeed = 3.0;
    float rotationSpeed = 1.0;

    float yaw = 0.0f;
    float pitch = 0.0f;

    float fov = 60.0f;
    float aspectRatio = 0.0f;
    float near = 0.1f, far = 100.0f;

    bool keyboardInput = true;
    bool mouseInput = true;

    // Orbit camera parameters
    float eyeUpOffset = 0.0f;
    float eyeFrontOffset = 0.0f;
    float targetUpOffset = 0.0f;
    float targetFrontOffset = 0.0f;

    UnorderedMap<unsigned int, bool> keys;
    bool first = true;

    Input *pInput;
};