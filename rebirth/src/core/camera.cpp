#include <rebirth/core/camera.h>

#include <rebirth/input/input.h>

#include "imgui.h"

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

void Camera::update(float deltaTime)
{
    ImGuiIO io = ImGui::GetIO();
    Input &input = g_input;

    if (io.WantCaptureKeyboard)
        return;

    front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front = glm::normalize(front);

    right = glm::normalize(glm::cross(front, up));

    if (type == CameraType::FirstPerson) {
        float moveSpeed = deltaTime * movementSpeed;
        if (input.isKeyPressed(KeyboardKey::LSHIFT)) {
            moveSpeed *= 4;
        }

        if (input.isKeyPressed(KeyboardKey::W))
            position += front * moveSpeed;
        if (input.isKeyPressed(KeyboardKey::S))
            position -= front * moveSpeed;
        if (input.isKeyPressed(KeyboardKey::A))
            position -= right * moveSpeed;
        if (input.isKeyPressed(KeyboardKey::D))
            position += right * moveSpeed;
    }

    updateViewMatrix();
}

void Camera::handleEvent(SDL_Event event, float deltaTime)
{
    ImGuiIO io = ImGui::GetIO();
    Input &input = g_input;

    if (io.WantCaptureMouse)
        return;

    // mouse
    if (input.isMouseButtonPressed(MouseButton::LEFT)) {
        if (!first) {
            yaw -= event.motion.xrel * rotationSpeed;
            pitch -= event.motion.yrel * rotationSpeed;
        }

        yaw = glm::mod(yaw, 360.0f);
        pitch = glm::clamp(pitch, -89.9f, 89.9f);

        first = !first;
    } else {
        first = true;
    }
}

void Camera::updateViewMatrix()
{
    if (type == CameraType::FirstPerson) {
        view = glm::lookAt(position, position + front, up);
    } else {
        vec3 eye = position + (-front * 10.0f) + (up * 3.0f);
        vec3 target = position + (front * 5.0f);
        view = glm::lookAt(eye, target, up);
    }
}