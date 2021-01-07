#ifndef __TEXTRENDERER_H__
#define __TEXTRENDERER_H__

#include <memory>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

class Shader;
class Texture2D;

struct Character {
    std::shared_ptr<Texture2D> texture;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class TextRenderer {
public:
    TextRenderer(const std::shared_ptr<Shader>& shader);
    ~TextRenderer();

    void RenderText(const std::string& text, const glm::vec2& position,
                    float scale = 1.0f,
                    const glm::vec3& color = glm::vec3(1.0f));
private:

    FT_Library ft;
    FT_Face face;

    GLuint VAO;
    GLuint VBO;
    std::shared_ptr<Shader> shader;

    std::unordered_map<char, Character> characters;

    void initCharacters();
};

#endif
