#include "ResourceManager.h"

#include <fstream>
#include <sstream>
#include <cassert>
#include <filesystem>

#include <fmt/core.h>
#include "stb_image.h"
#include <glad/glad.h>

#include "Shader.h"
#include "Texture2D.h"

ResourceManager* ResourceManager::singleton = nullptr;

ResourceManager::ResourceManager() {
  // determine the project root dir
  auto path = std::filesystem::current_path();
  auto target = path;
  bool found = false;
  do {
    // std::cout << path.string() << "\n";
    target = path;
    target /= "resources";
    if (std::filesystem::exists(target)) {
      found = true;
      break; // found
    }
    if (path == path.parent_path()) {
      break; // not found
    }
    path = path.parent_path();
  } while (true);
  if (found) {
    projectRootDir = path.string();
  }
  assert(found);
}

ResourceManager::~ResourceManager() {
    // release shaders

    // release textures

}

ResourceManager* ResourceManager::GetInstance() {
    if (!singleton) {
        singleton = new ResourceManager();
    }
    return singleton;
}

Shader*
ResourceManager::LoadShader(const std::string& name,
                            const char* vert,
                            const char* frag,
                            const char* geom) {
    std::string vertCode = LoadShaderCode(vert);
    std::string fragCode = LoadShaderCode(frag);
    std::string geomCode = geom ? LoadShaderCode(geom) : std::string();
    ShaderSource vertS = { vert, vertCode.c_str() };
    ShaderSource fragS = { frag, fragCode.c_str() };
    ShaderSource geomS = { geom, geomCode.c_str() };

    if (vertCode.empty() || fragCode.empty()) {
        fmt::print("Can not load {} or {}\n", vert, frag);
        return nullptr;
    }

    shaders.emplace(
        std::make_pair(
            name,
            std::make_unique<Shader>(&vertS, &fragS,
                                     geom ? &geomS : nullptr)));

    return shaders[name].get();
}

Texture2D*
ResourceManager::
LoadTexture2D(const char* path, const std::string& name,
              bool gammaCorrection) {
    int width, height, channels;
    std::string texturePath = projectRootDir + "/" + path;
    unsigned char *data =
        stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

    if (!data) {
        fmt::print("Failed to load texture {}!\n", texturePath);
    }

    GLint internalFormat;
    GLenum dataFormat;
    switch (channels) {
    case 1: {
        internalFormat = dataFormat = GL_RED;
        break;
    }
    case 3: {
        internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
        dataFormat = GL_RGB;
        break;
    }
    case 4: {
        internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
        dataFormat = GL_RGBA;
        break;
    }
    default: {
        fmt::print("Texture {} has {} channels!\n", texturePath, channels);
        assert(false);
        break;
    }
    }

    std::vector<TexParameteri> params;
    params.push_back({GL_TEXTURE_WRAP_S, GL_REPEAT});
    params.push_back({GL_TEXTURE_WRAP_T, GL_REPEAT});
    params.push_back({GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR});
    params.push_back({GL_TEXTURE_MAG_FILTER, GL_LINEAR});
    TextureSource ts = {
        internalFormat,
        width, height,
        dataFormat, GL_UNSIGNED_BYTE,
        data,
        true,
        params
    };

    textures.emplace(name,
                     std::make_unique<Texture2D>(&ts));

    return textures[name].get();
}

std::string ResourceManager::LoadShaderCode(const char* file) {
    std::ifstream fstream;
    std::stringstream content;
    std::string shaderPath = projectRootDir + "/" + file;
    fstream.open(shaderPath);
    if (!fstream.is_open()) {
        fmt::print("Can not open shader file: {}\n", shaderPath);
        return content.str();
    }
    content << fstream.rdbuf();
    fstream.close();

    return content.str();
}

Shader*
ResourceManager::GetShader(const std::string& name) {
    if (shaders.find(name) == shaders.end()) {
        return nullptr;
    }
    return shaders[name].get();
}

Texture2D*
ResourceManager::GetTexture2D(const std::string& name) {
    if (textures.find(name) == textures.end()) {
        return nullptr;
    }
    return textures[name].get();
}

void ResourceManager::OpenFile(const char* path, std::ifstream& fstream) {
  std::string filePath = projectRootDir + "/" + path;
  fstream.open(filePath);
}

std::string ResourceManager::RelativePathToAbolutePath(const std::string& relativePath) {
  return projectRootDir + "/" + relativePath;
}
