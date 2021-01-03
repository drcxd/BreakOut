#ifndef __TEXTURE2D_H__
#define __TEXTURE2D_H__

#include <vector>

#include <glad/glad.h>

struct TexParameteri {
    GLenum pname;
    GLint param;
};

struct TextureSource {
    const char* name;
    GLint internalFormat;
    int width, height;
    GLenum format, type;
    void* data;
    std::vector<TexParameteri> params;
};

class Texture2D {
public:
    unsigned int ID;
    Texture2D(TextureSource* texture);
    ~Texture2D();
    void SetTexParams(const std::vector<TexParameteri>& params);
};

#endif
