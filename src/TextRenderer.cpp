#include "TextRenderer.h"

#include <fmt/core.h>

#include "Shader.h"
#include "Texture2D.h"
#include "ResourceManager.h"

TextRenderer::TextRenderer(const Shader* shader)
    : shader(shader) {
    if (FT_Init_FreeType(&ft)) {
        fmt::print("ERROR::FREETYPE: Failed init FreeType library!\n");
    }

    std::string fontPath =
        ResourceManager::GetInstance()->RelativePathToAbolutePath(
            "./resources/fonts/arial.ttf");
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        fmt::print("ERROR::FREETYPE: Failed to load font!\n");
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    initCharacters();
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4,
                 nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TextRenderer::~TextRenderer() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void TextRenderer::initCharacters() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fmt::print("ERROR::FREETYPE: "
                       "Failed to load glyph for {}!\n", c);
            continue;
        }

        TextureSource ts;
        ts.internalFormat = GL_RED;
        ts.width = face->glyph->bitmap.width;
        ts.height = face->glyph->bitmap.rows;
        ts.format = GL_RED;
        ts.data = face->glyph->bitmap.buffer;
        ts.mipmap = false;
        ts.params.clear();
        ts.params.push_back({GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE});
        ts.params.push_back({GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE});
        ts.params.push_back({GL_TEXTURE_MIN_FILTER, GL_LINEAR});
        ts.params.push_back({GL_TEXTURE_MAG_FILTER, GL_LINEAR});

        Character character = {
            std::make_unique<Texture2D>(&ts),
            glm::ivec2(face->glyph->bitmap.width,
                       face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left,
                       face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        characters[c] = std::move(character);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void TextRenderer::RenderText(const std::string& text,
                              const glm::vec2& position,
                              float scale, const glm::vec3& color) {
    shader->use();
    shader->setVec3("textColor", color);
    glBindVertexArray(VAO);

    float x = position.x;
    float y = position.y;

    for (char c : text) {
        Character& ch = characters[c];
        float xpos = x + ch.bearing.x * scale;
        float ypos = y +
            (characters['H'].bearing.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            { xpos, ypos + h, 0.0f, 1.0f },
            { xpos + w, ypos, 1.0f, 0.0f },
            { xpos, ypos, 0.0f, 0.0f },
            { xpos, ypos + h, 0.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 1.0f },
            { xpos + w, ypos, 1.0f, 0.0f }
        };

        shader->setTexture("text", 0, ch.texture.get());
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
}
