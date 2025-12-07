#include "Camera.h"

Camera::Camera() noexcept {}

Camera::Camera(const glm::vec3& initPosition, const glm::quat& initRotation) noexcept
    : position(initPosition), rotation(initRotation) {}

Camera::Camera(const glm::vec3& initPosition, const glm::vec3& eulerAngles) noexcept
    : position(initPosition), rotation(glm::quat(glm::radians(eulerAngles))) {}

glm::vec3 Camera::Forward() const noexcept { return rotation * glm::vec3(0.0f, 0.0f, -1.0f); }
glm::vec3 Camera::Right()   const noexcept { return rotation * glm::vec3(1.0f, 0.0f, 0.0f); }
glm::vec3 Camera::Up()      const noexcept { return rotation * glm::vec3(0.0f, 1.0f, 0.0f); }

void Camera::clampRotation() noexcept {
    // TODO clamp Rotation
    rotation = glm::normalize(rotation);
}

glm::mat4 Camera::GetViewMatrix() const noexcept {
    glm::mat4 rotMat = glm::toMat4(glm::inverse(rotation));
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), -position);
    return rotMat * transMat;
}