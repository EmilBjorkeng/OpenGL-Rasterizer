#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "Shader.h"
#include "Light.h"
#include <glm/glm.hpp>
#include <vector>

// Floats per vertex
const size_t OBJECT_STRIDE = 13;
const size_t MAX_TEXTURES = 16;

class Object {
public:
    const Shader* shader = nullptr;
    unsigned int VAO = 0, VBO = 0;
    bool hasTransparency = false;

    std::vector<float> vertices;
    std::vector<unsigned int> textures;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    bool useLighting = true;

    Object(const char* path, const Shader* shader);
    glm::mat4 GetModelMatrix();
    void draw(const glm::mat4 view, const glm::mat4 projection, std::vector<Light> &sceneLight);
};

#endif