#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Camera.h"
#include "Shader.h"
#include <glm/glm.hpp>

class Object;

const size_t MAX_LIGHTS = 16;
const unsigned int SHADOW_MAP_SIZE = 1024;
const float SHADOW_NEAR_PLANE = 0.001f;
const float SHADOW_FAR_PLANE  = 100.0f; // TODO calculate based on light intensity

class Light {
public:
    glm::vec3 position;
    glm::vec3 color;
    float intensity;

    const Shader *shader = nullptr;
    unsigned int depthMapFBO = 0;
    unsigned int depthCubemap = 0;

    Light(glm::vec3 position, glm::vec3 color, float intensity, const Shader *shader);
    ~Light();

    // Remove copying
    Light(const Light&) = delete;
    Light& operator=(const Light&) = delete;

    // Allow moving
    Light(Light&& other) noexcept;
    Light& operator=(Light&& other) noexcept;

    void renderShadowMap(const std::vector<Object*> &sceneObjects);
};

#endif