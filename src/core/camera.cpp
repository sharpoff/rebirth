#include "core/camera.h"

#include "input/input.h"

#include "SDL3/SDL_events.h"
#include "imgui.h"

void Camera::initialize(Input *input)
{
    assert(input);
    this->input = input;
}

void Camera::update(float deltaTime)
{
    front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front = glm::normalize(front);

    right = glm::normalize(glm::cross(front, up));

    if (!ImGui::GetIO().WantCaptureKeyboard && keyboardInput && type == CameraType::FirstPerson) {
        float moveSpeed = deltaTime * movementSpeed;
        if (input->getKey(KeyboardKey::LSHIFT, InputAction::Pressed)) {
            moveSpeed *= 4;
        }

        if (input->getKey(KeyboardKey::W, InputAction::Pressed))
            position += front * moveSpeed;
        if (input->getKey(KeyboardKey::S, InputAction::Pressed))
            position -= front * moveSpeed;
        if (input->getKey(KeyboardKey::A, InputAction::Pressed))
            position -= right * moveSpeed;
        if (input->getKey(KeyboardKey::D, InputAction::Pressed))
            position += right * moveSpeed;
    }

    updateViewMatrix();
}

void Camera::processEvent(const SDL_Event &event)
{
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    // mouse
    if (input->getMouseButton(MouseButton::LEFT, InputAction::Pressed)) {
        if (!first) {
            int pitchSign = type == CameraType::Orbit ? -1 : 1; // reverse pitch for orbit camera

            yaw -= event.motion.xrel * rotationSpeed;
            pitch -= pitchSign * event.motion.yrel * rotationSpeed;
        } else {
            first = false;
        }

        yaw = glm::mod(yaw, 360.0f);
        pitch = glm::clamp(pitch, -89.9f, 89.9f);
    }

    if (input->getMouseButton(MouseButton::LEFT, InputAction::Released))
        first = true;
}

void Camera::updateViewMatrix()
{
    if (type == CameraType::FirstPerson) {
        view = glm::lookAt(position, position + front, up);
    } else if (type == CameraType::Orbit) {
        vec3 eye = position + (front * eyeFrontOffset) + (up * eyeUpOffset);
        vec3 target = position + (front * targetFrontOffset) + (up * targetUpOffset);
        view = glm::lookAt(eye, target, up);
    }
}

void Camera::setPosition(vec3 position)
{
    this->position = position;
    updateViewMatrix();
}

void Camera::setPerspective(float fov, float aspectRatio, float near, float far)
{
    projection = math::perspective(fov, aspectRatio, near, far);

    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->near = near;
    this->far = far;
}

void Camera::setPerspectiveInf(float fov, float aspectRatio, float near)
{
    projection = math::perspectiveInf(fov, aspectRatio, near);

    this->fov = fov;
    this->aspectRatio = aspectRatio;
    this->near = near;
    this->far = 1000.0f;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, float near, float far)
{
    projection = glm::ortho(left, right, bottom, top, near, far);

    this->near = near;
    this->far = far;
}

void Camera::setCameraType(CameraType type)
{
    this->type = type;
}