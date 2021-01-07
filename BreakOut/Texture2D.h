#ifndef __TEXTURE2D_H__
#define __TEXTURE2D_H__

#include <vector>

#include <glad/glad.h>

struct TexParameteri {
    GLenum pname;
    GLint param;
};

struct TextureSource {
    GLint internalFormat = GL_RGB;
    int width = 0, height = 0;
    GLenum format = GL_RGB;
    GLenum type = GL_UNSIGNED_BYTE;
    void* data = nullptr;
    bool mipmap = true;
    std::vector<TexParameteri> params = {
        { GL_TEXTURE_WRAP_S, GL_REPEAT },
        { GL_TEXTURE_WRAP_T, GL_REPEAT },
        { GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR },
        { GL_TEXTURE_MAG_FILTER, GL_LINEAR },
    };
};

class Texture2D {
public:
    unsigned int ID;
    Texture2D(TextureSource* texture);
    ~Texture2D();
    void SetTexParams(const std::vector<TexParameteri>& params);
};

#endif
