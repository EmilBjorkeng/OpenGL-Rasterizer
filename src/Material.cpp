#include "Material.h"
#include "TextureManager.h"
#include "STB/stb_image.h"
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

void Material::loadMTL(const std::string& path, const std::string &materialName) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open MTL file: " << path << "\n";
        return;
    }

    std::string line;
    bool skipping = false;
    std::filesystem::path directory = std::filesystem::path(path).parent_path();

    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "newmtl") {
            std::string name;
            ss >> name;
            skipping = (name != materialName);
            if (!skipping) this->name = name;
        }
        else if (skipping) {
            continue;
        }
        else if (type == "Kd") {
            ss >> diffuseColor.x >> diffuseColor.y >> diffuseColor.z;
        }
        else if (type == "d") {
            ss >> opacity;
        }
        else if (type == "Tr") {
            float transparency;
            ss >> transparency;
            opacity = 1.0f - transparency;
        }
        else if (type == "map_Kd") {
            std::string texturePath;
            ss >> texturePath;

            // Relative path
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                texturePath = path.substr(0, lastSlash + 1) + texturePath;
            }

            diffuseTexture = TextureManager::getInstance().loadTexture(texturePath);
        }
    }
}