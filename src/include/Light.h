#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Camera.h"
#include "Shader.h"
#include <glm/glm.hpp>

class Object;

constexpr size_t MAX_LIGHTS = 16;
constexpr float MIN_FAR_PLANE = 1.0f;
constexpr float MAX_FAR_PLANE = 500.0f;

class Light {
public:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color = glm::vec3(0.0f);
    float intensity = 1.0f;

    const Shader *shader = nullptr;
    unsigned int depthMapFBO = 0;
    unsigned int depthCubemap = 0;

    static const unsigned int SHADOW_MAP_SIZE = 1024;
    float shadowNearPlane = 0.01f;
    float shadowFarPlane  = 100.0f;

    Light(glm::vec3 position, glm::vec3 color, float intensity, const Shader *shader);
    ~Light() noexcept;

    // Remove copying
    Light(const Light&) = delete;
    Light& operator=(const Light&) = delete;

    // Allow moving
    Light(Light&& other) noexcept;
    Light& operator=(Light&& other) noexcept;

    float calculateFarPlane() const;
    void renderShadowMap(const std::vector<Object*> &sceneObjects);
};

#endif