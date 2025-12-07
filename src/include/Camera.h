#ifndef __CAMERA_H__
#define __CAMERA_H__

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    float fov = 90.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    float moveSpeed = 2.0f;
    float turnSpeed = 60.0f;
    float sensitivity = 0.0025f;

    Camera() noexcept;
    // Constructor with position and quaternion
    Camera(const glm::vec3& initPosition, const glm::quat& initRotation) noexcept;
    // Constructor with position and Euler angles (pitch, yaw, roll)
    Camera(const glm::vec3& initPosition, const glm::vec3& eulerAngles) noexcept;

    // Allow copying
    Camera(const Camera&) = default;
    Camera& operator=(const Camera&) = default;

    // Allow moving
    Camera(Camera&&) = default;
    Camera& operator=(Camera&&) = default;

    glm::vec3 Forward() const noexcept;
    glm::vec3 Right()   const noexcept;
    glm::vec3 Up()      const noexcept;

    void clampRotation() noexcept;
    [[nodiscard]]
    glm::mat4 GetViewMatrix() const noexcept;
};

#endif
