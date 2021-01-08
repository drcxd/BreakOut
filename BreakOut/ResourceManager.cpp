#include "ResourceManager.h"

#include <fstream>
#include <sstream>
#include <cassert>

#include <fmt/core.h>
#include "stb_image.h"
#include <glad/glad.h>

#include "Shader.h"
#include "Texture2D.h"

static std::string LoadShaderCode(const char* file);

ResourceManager* ResourceManager::singleton = nullptr;

ResourceManager::ResourceManager() { }

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
    unsigned char *data =
        stbi_load(path, &width, &height, &channels, 0);

    if (!data) {
        fmt::print("Failed to load texture {}!\n", path);
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
        fmt::print("Texture {} has {} channels!\n", path, channels);
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

std::string LoadShaderCode(const char* file) {
    std::ifstream fstream;
    std::stringstream content;

    fstream.open(file);
    if (!fstream.is_open()) {
        fmt::print("Can not open shader file: {}\n", file);
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
