#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <unordered_map>
#include <string>
#include <memory>

class Texture2D;
class Shader;

class ResourceManager {
public:

    ~ResourceManager();
    static ResourceManager* GetInstance();
    const std::shared_ptr<Shader>&
    LoadShader(const std::string& name, const char* vert,
               const char* frag, const char* geom = nullptr);
    const std::shared_ptr<Texture2D>&
    LoadTexture2D(const char* path, const std::string& name,
                  bool gammaCorrection = false);
    const std::shared_ptr<Shader>&
    GetShader(const std::string& name);
    const std::shared_ptr<Texture2D>&
    GetTexture2D(const std::string& name);

private:

    static ResourceManager* singleton;
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;

    std::unordered_map<std::string,
                       std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string,
                       std::shared_ptr<Texture2D>> textures;
};
#endif
