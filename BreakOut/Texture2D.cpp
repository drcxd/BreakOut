#include "Texture2D.h"

#include <fmt/core.h>

#include "Utility.h"

Texture2D::Texture2D(TextureSource* texture) {
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, texture->internalFormat,
                 texture->width, texture->height, 0,
                 texture->format, texture->type, texture->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    SetTexParams(texture->params);
    glBindTexture(GL_TEXTURE_2D, 0);
    Utility::CheckGLError();
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &ID);
}

void Texture2D::SetTexParams(
    const std::vector<TexParameteri>& params) {

    glBindTexture(GL_TEXTURE_2D, ID);
    for (auto param : params) {
        glTexParameteri(GL_TEXTURE_2D, param.pname, param.param);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}
