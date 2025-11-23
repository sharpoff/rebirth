#include "core/camera.h"

#include "input/input.h"

#include "imgui.h"

Camera::Camera(Input *input)
{
    assert(input);
    pInput = input;
}

void Camera::update(float deltaTime)
{
    m_front.x = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    m_front.y = sin(glm::radians(pitch));
    m_front.z = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    m_front = glm::normalize(m_front);

    m_right = glm::normalize(glm::cross(m_front, m_up));

    if (!ImGui::GetIO().WantCaptureKeyboard && keyboardInput && m_type == CameraType::FirstPerson) {
        float moveSpeed = deltaTime * movementSpeed;
        if (pInput->getKey(KeyboardKey::LSHIFT, InputAction::Pressed)) {
            moveSpeed *= 4;
        }

        if (pInput->getKey(KeyboardKey::W, InputAction::Pressed))
            m_position += m_front * moveSpeed;
        if (pInput->getKey(KeyboardKey::S, InputAction::Pressed))
            m_position -= m_front * moveSpeed;
        if (pInput->getKey(KeyboardKey::A, InputAction::Pressed))
            m_position -= m_right * moveSpeed;
        if (pInput->getKey(KeyboardKey::D, InputAction::Pressed))
            m_position += m_right * moveSpeed;
    }

    updateViewMatrix();
}

void Camera::processInput()
{
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    // mouse
    if (pInput->getMouseButton(MouseButton::LEFT, InputAction::Pressed)) {
        vec2 relPosition = pInput->getMouseRelativeMotion();
        if (!first) {
            int pitchSign = m_type == CameraType::Orbit ? -1 : 1; // reverse pitch for orbit camera

            yaw -= relPosition.x * rotationSpeed;
            pitch -= pitchSign * relPosition.y * rotationSpeed;
        } else {
            first = false;
        }

        yaw = glm::mod(yaw, 360.0f);
        pitch = glm::clamp(pitch, -89.9f, 89.9f);
    }

    if (pInput->getMouseButton(MouseButton::LEFT, InputAction::Released))
        first = true;
}

void Camera::updateViewMatrix()
{
    if (m_type == CameraType::FirstPerson) {
        view = glm::lookAt(m_position, m_position + m_front, m_up);
    } else if (m_type == CameraType::Orbit) {
        vec3 eye = m_position + (m_front * eyeFrontOffset) + (m_up * eyeUpOffset);
        vec3 target = m_position + (m_front * targetFrontOffset) + (m_up * targetUpOffset);
        view = glm::lookAt(eye, target, m_up);
    }
}

void Camera::setPosition(vec3 position)
{
    this->m_position = position;
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
    this->m_type = type;
}