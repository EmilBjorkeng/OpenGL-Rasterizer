#include "Material.h"
#include "STB/stb_image.h"
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

unsigned int loadImage(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (!data) {
        std::cout << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    GLenum format, internalFormat;
    if (nrChannels == 1) format = internalFormat = GL_RED;
    else if (nrChannels == 3) { format = GL_RGB; internalFormat = GL_SRGB; }
    else if (nrChannels == 4) { format = GL_RGBA; internalFormat = GL_SRGB_ALPHA; }
    else {
        std::cout << "Unsupported number of channels: " << nrChannels << std::endl;
        stbi_image_free(data);
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture;
}

bool Material::loadMTL(const std::string& path, const std::string &materialName) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Failed to open MTL file: " << path << "\n";
        return false;
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
            std::string texFile;
            ss >> texFile;
            diffuseTexture = loadImage((directory / texFile).string().c_str());
            if (diffuseTexture == 0) std::cout << "Failed to load: " << path << std::endl;
        }
    }
    return true;
}