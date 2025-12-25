#include "TextureManager.h"
#include <glad/gl.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

inline bool file_exists (const std::string& path) {
    if (FILE *file = fopen(path.c_str(), "r")) {
        fclose(file);
        return true;
    }
    return false;
}

TextureManager::~TextureManager() {
    unloadAll();
}

unsigned int TextureManager::loadTexture(const std::string& path) {
    // Check if already loaded
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }

    if (!file_exists(path)) {
        std::cerr << "Missing texture file: " << path << std::endl;
        return 0;
    }

    // Load new texture
    unsigned int textureID = loadTextureFromFile(path);

    if (textureID != 0) {
        textures[path] = textureID;
        std::cout << "Loaded texture: " << path << " (ID: " << textureID << ")" << std::endl;
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    return textureID;
}

void TextureManager::registerTexture(const std::string& name, unsigned int textureID) {
    if (textureID == 0) {
        std::cerr << "Warning: Attempting to register invalid texture ID 0" << std::endl;
        return;
    }

    // Check if name already exists
    auto it = textures.find(name);
    if (it != textures.end()) {
        std::cerr << "Warning: Texture name '" << name << "' already exists. Overwriting." << std::endl;
        // Delete the old texture
        glDeleteTextures(1, &it->second);
    }

    textures[name] = textureID;
}

unsigned int TextureManager::getTexture(const std::string& path) const {
    auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }
    return 0;
}

bool TextureManager::hasTexture(const std::string& path) const {
    return textures.find(path) != textures.end();
}

void TextureManager::unloadTexture(const std::string& path) {
    auto it = textures.find(path);
    if (it != textures.end()) {
        glDeleteTextures(1, &it->second);
        textures.erase(it);
        std::cout << "Unloaded texture: " << path << std::endl;
    }
}

void TextureManager::unloadAll() {
    for (auto& [path, textureID] : textures) {
        glDeleteTextures(1, &textureID);
    }
    textures.clear();
    std::cout << "Unloaded all textures\n";
}


size_t TextureManager::getTextureCount() const {
    return textures.size();
}

unsigned int TextureManager::loadTextureFromFile(const std::string& path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        GLenum format, internalFormat;
        if (nrChannels == 1) {
            format = internalFormat = GL_RED;
        } else if (nrChannels == 3) {
            format = GL_RGB; internalFormat = GL_SRGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA; internalFormat = GL_SRGB_ALPHA;
        } else {
            std::cerr << "Unsupported number of channels: " << nrChannels << " in " << path << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &textureID);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "STB Image error: " << stbi_failure_reason() << std::endl;
        glDeleteTextures(1, &textureID);
        return 0;
    }

    return textureID;
}