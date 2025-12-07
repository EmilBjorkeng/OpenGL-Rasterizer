#ifndef __MAPLOADER_H__
#define __MAPLOADER_H__

#include "Object.h"
#include "Light.h"
#include <vector>
#include <memory>

struct Scene {
    // Raw pointers
    std::vector<Object*> sceneObjects;
    std::vector<Light*> sceneLights;
    std::vector<Object*> opaqueObjects;
    std::vector<Object*> transparentObjects;

    // Ownership
    std::vector<std::unique_ptr<Object>> objectOwnership;
    std::vector<std::unique_ptr<Light>> lightOwnership;

    Scene() = default;

    // Remove copying
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // Allow moving
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;
};

class MAPLoader {
public:
    [[nodiscard]]
    static Scene loadMAP(const std::string &path, const Shader &shader, const Shader &shadowShader);
};

#endif