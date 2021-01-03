#ifndef __SPRITERENDERER_H__
#define __SPRITERENDERER_H__

#include <memory>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

class Shader;
class Texture2D;

class SpriteRenderer {
public:

    SpriteRenderer(std::shared_ptr<Shader> shader);
    ~SpriteRenderer();

    void Draw(
        const std::shared_ptr<Texture2D>& texture, glm::vec2 position,
        glm::vec2 size = glm::vec2(10.0f, 10.0f),
        float rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f)
        );

private:

    std::shared_ptr<Shader> shader;
    GLuint quadVAO;

    void InitRenderData();
};
#endif