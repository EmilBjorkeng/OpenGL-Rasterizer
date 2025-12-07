#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <glm/glm.hpp>
#include <string>

class Material {
public:
    std::string name;
    glm::vec3 diffuseColor = glm::vec3(0.8f);
    float opacity = 1.0f;
    unsigned int diffuseTexture = 0;

    Material() noexcept = default;

    // Allow copying
    Material(const Material&) = default;
    Material& operator=(const Material&) = default;

    // Allow moving
    Material(Material&&) = default;
    Material& operator=(Material&&) = default;

    [[nodiscard]]
    bool loadMTL(const std::string& path, const std::string &materialName);
};

#endif