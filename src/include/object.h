#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "Shader.h"
#include <vector>

class Object {
public:
    const Shader* shader = nullptr;
    unsigned int VAO = 0, VBO = 0;

    std::vector<float> vertices;
    std::vector<unsigned int> textures;

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    Object(const char* path, const Shader* shader);
    void draw();
};






/*
struct Vertex {
    glm::vec3 point;
    glm::vec2 texture;
    glm::vec3 normal;
};

struct Material {
    std::string name;
    glm::vec3 diffuseColor;
    unsigned int diffuseTexture;
    float opacity;
};

struct Object {
    std::vector<float> vertices;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);

    const Shader *shader;
    std::vector<unsigned int> textures;

    unsigned int VAO;
    unsigned int VBO;

    Object(const char* path, const Shader* shader);

    ~Object() {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
    }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
};

std::vector<float> LoadObjFile(const char* path, std::vector<unsigned int> &textures);
Material loadMaterial(const char* path, std::string currentMaterial);
std::vector<float> MakeTriangle(std::vector<Vertex>& vertices, const Material &material);*/

#endif