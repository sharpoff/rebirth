#include <rebirth/types/camera.h>

#include <algorithm>

#include "imgui.h"

namespace rebirth
{

void Camera::setPerspective(float fov, float aspectRatio, float near, float far)
{
    projection = math::perspective(fov, aspectRatio, near, far);

    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->near = near;
    this->far = far;
}

void Camera::update(float deltaTime)
{
    glm::quat yawRotation = glm::angleAxis(glm::radians(yaw), vec3(0.0f, -1.0f, 0.0f));
    glm::quat pitchRotation = glm::angleAxis(glm::radians(pitch), vec3(1.0f, 0.0f, 0.0f));
    transform.setRotation(yawRotation * pitchRotation);

    float speedBoost = 1.0f;
    if (keys[SDLK_LSHIFT])
        speedBoost = 4.0f;

    transform.translate(vec3(
        transform.getRotation() * vec4(velocity * movementSpeed * speedBoost * deltaTime, 0.0f)
    ));
}

void Camera::handleEvent(SDL_Event event, float deltaTime)
{
    auto io = ImGui::GetIO();

    // mouse
    if (!io.WantCaptureMouse && event.type == SDL_EVENT_MOUSE_MOTION &&
        event.button.button == SDL_BUTTON_LEFT) {

        yaw += event.motion.xrel * rotationSpeed;
        pitch -= event.motion.yrel * rotationSpeed;

        pitch = std::clamp(pitch, -89.9f, 89.9f);
    }

    // keyboard
    if (!io.WantCaptureKeyboard) {
        keys[event.key.key] = event.type == SDL_EVENT_KEY_DOWN;
        vec2 cameraMotion = vec2(keys[SDLK_W], keys[SDLK_D]) - vec2(keys[SDLK_S], keys[SDLK_A]);

        velocity = vec3(cameraMotion.y, 0.0, -cameraMotion.x);
    }
}

const mat4 Camera::getViewMatrix() const
{
    mat4 translation = translate(mat4(1.0f), transform.getPosition());
    mat4 rotation = glm::toMat4(transform.getRotation());
    return glm::inverse(translation * rotation);
}

} // namespace rebirth