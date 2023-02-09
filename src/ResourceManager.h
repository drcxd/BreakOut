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

    Shader*
    LoadShader(const std::string& name, const char* vert,
               const char* frag, const char* geom = nullptr);

    Texture2D*
    LoadTexture2D(const char* path, const std::string& name,
                  bool gammaCorrection = false);

    Shader*
    GetShader(const std::string& name);

    Texture2D*
    GetTexture2D(const std::string& name);

  void OpenFile(const char* path, std::ifstream& fstream);

  std::string RelativePathToAbolutePath(const std::string& relativePath);

private:

    static ResourceManager* singleton;
    ResourceManager();
    ResourceManager(const ResourceManager&) = delete;
  std::string LoadShaderCode(const char* file);
    std::unordered_map<std::string,
                       std::unique_ptr<Shader>> shaders;
    std::unordered_map<std::string,
                       std::unique_ptr<Texture2D>> textures;
  std::string projectRootDir;
};
#endif
