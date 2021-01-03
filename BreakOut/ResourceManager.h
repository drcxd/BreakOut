#ifndef __RESOURCEMANAGER_H__
#define __RESOURCEMANAGER_H__

#include <unordered_map>
#include <string>
#include <memory>

#include "Texture2D.h"
#include "Shader.h"

class ResourceManager {
public:

    ~ResourceManager();
    static ResourceManager* GetInstance();
    std::shared_ptr<Shader>
    LoadShader(const std::string& name, const char* vert,
               const char* frag, const char* geom = nullptr);
    std::shared_ptr<Texture2D>
    LoadTexture2D(const char* path, bool gammaCorrection = false);

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
