#include "MAPLoader.h"
#include <fstream>
#include <sstream>
#include <filesystem>

Scene MAPLoader::loadMAP(const std::string& path, const Shader &shader, const Shader &shadowShader) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open MAP file: " << path << "\n";
        return {};
    }

    Scene scene;
    std::string line;

    while (std::getline(in, line)) {
        line.erase(0, line.find_first_not_of(" \t")); // Remove leading spaces
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "OBJECT") {
            std::string objPath;
            float px, py, pz;
            float rx, ry, rz;
            float sx, sy, sz;
            int useLighting;

            ss >> objPath;
            if (!objPath.empty() && objPath.back() == ',') objPath.pop_back();

            char comma;
            ss >> px >> py >> pz >> comma >> rx >> ry >> rz >> comma >> sx >> sy >> sz >> comma >> useLighting;

            auto obj = std::make_unique<Object>(objPath, &shader);
            obj->position = glm::vec3(px, py, pz);
            obj->rotation = glm::vec3(rx, ry, rz);
            obj->scale = glm::vec3(sx, sy, sz);
            obj->useLighting = useLighting != 0;

            Object* objPtr = obj.get();
            scene.sceneObjects.push_back(objPtr);
            scene.objectOwnership.push_back(std::move(obj));
        }
        else if (type == "LIGHT") {
            float px, py, pz, r, g, b, intensity;
            char comma;
            ss >> px >> py >> pz >> comma >> r >> g >> b >> comma >> intensity;

            auto light = std::make_unique<Light>(
                glm::vec3(px, py, pz), glm::vec3(r, g, b), intensity, &shadowShader);

            Light* lightPtr = light.get();
            scene.lightOwnership.push_back(std::move(light));
            scene.sceneLights.push_back(lightPtr);

            // Light Object
            auto lightObj = std::make_unique<Object>("assets/LightSphere.obj", &shader);
            lightObj->position = glm::vec3(px, py, pz);
            lightObj->scale = glm::vec3(0.05f);
            lightObj->useLighting = false;

            // Change the color of the light object
            size_t diffuseOffset = 9;
            auto& verts = lightObj->vertices;
            for (size_t i = 0; i < verts.size(); i += OBJECT_STRIDE) {
                verts[i + diffuseOffset + 0] = r;
                verts[i + diffuseOffset + 1] = g;
                verts[i + diffuseOffset + 2] = b;
            }
            // Re-upload the vertex data
            glBindBuffer(GL_ARRAY_BUFFER, lightObj->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                lightObj->vertices.size() * sizeof(float),
                lightObj->vertices.data());

            Object* lightObjPtr = lightObj.get();
            scene.sceneObjects.push_back(lightObjPtr);
            scene.objectOwnership.push_back(std::move(lightObj));
        }
        else {
            std::cerr << "Unknown type in MAP file: " << type << "\n";
            return {};
        }
    }

    // Separate opaque and transparent objects
    for (auto& obj : scene.sceneObjects) {
        if (obj->hasTransparency) scene.transparentObjects.push_back(obj);
        else scene.opaqueObjects.push_back(obj);
    }

    return scene;
}