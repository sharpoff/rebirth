#include "imgui.h"
#include <algorithm>
#include <rebirth/camera.h>

namespace rebirth
{

void Camera::update(float deltaTime)
{
    mat4 rotation = getRotationMatrix();

    float speedBoost = 1.0f;
    if (keys[SDLK_LSHIFT])
        speedBoost = 4.0f;

    position += vec3(rotation * vec4(velocity * movementSpeed * speedBoost * deltaTime, 0.0f));
}

void Camera::handleEvent(SDL_Event event, float deltaTime)
{
    auto io = ImGui::GetIO();

    // mouse
    if (!io.WantCaptureMouse && event.type == SDL_EVENT_MOUSE_MOTION && event.button.button == SDL_BUTTON_LEFT) {

        yaw += event.motion.xrel * rotationSpeed;
        pitch -= event.motion.yrel * rotationSpeed;

        pitch = std::clamp(pitch, glm::radians(-89.9f), glm::radians(89.9f));
    }

    // keyboard
    if (!io.WantCaptureKeyboard) {
        keys[event.key.key] = event.type == SDL_EVENT_KEY_DOWN;
        vec2 cameraMotion = vec2(keys[SDLK_W], keys[SDLK_D]) - vec2(keys[SDLK_S], keys[SDLK_A]);

        velocity = vec3(cameraMotion.y, 0.0, -cameraMotion.x);
    }
}

mat4 Camera::getViewMatrix()
{
    mat4 translation = translate(mat4(1.0f), position);
    mat4 rotation = getRotationMatrix();
    return glm::inverse(translation * rotation);
}

mat4 Camera::getRotationMatrix()
{
    glm::quat pitchRotation = glm::angleAxis(pitch, vec3(1.0f, 0.0f, 0.0f));
    glm::quat yawRotation = glm::angleAxis(yaw, vec3(0.0f, -1.0f, 0.0f));

    return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

} // namespace rebirth