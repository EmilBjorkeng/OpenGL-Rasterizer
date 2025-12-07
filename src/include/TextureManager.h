#ifndef __TEXTURE_MANAGER_H__
#define __TEXTURE_MANAGER_H__

#include <string>
#include <unordered_map>

class TextureManager {
public:
    static TextureManager& getInstance() {
        static TextureManager instance;
        return instance;
    }

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    [[nodiscard]]
    unsigned int loadTexture(const std::string& path);
    void registerTexture(const std::string& name, unsigned int textureID);

    [[nodiscard]]
    unsigned int getTexture(const std::string& path) const;
    [[nodiscard]]
    bool hasTexture(const std::string& path) const;

    void unloadTexture(const std::string& path);
    void unloadAll();

    [[nodiscard]]
    size_t getTextureCount() const;

private:
    TextureManager() = default;
    ~TextureManager() noexcept;

    std::unordered_map<std::string, unsigned int> textures;
    unsigned int loadTextureFromFile(const std::string& path);
};

#endif