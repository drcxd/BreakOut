#include "SpriteRenderer.h"

#include "Shader.h"
#include "Utility.h"

SpriteRenderer::SpriteRenderer(const std::shared_ptr<Shader>& shader)
    : shader(shader)
    , quadVAO(0)
    , quadVBO(0) {
    InitRenderData();
}

SpriteRenderer::~SpriteRenderer() {
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void SpriteRenderer::InitRenderData() {
    float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                 GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*) 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SpriteRenderer::Draw(const std::shared_ptr<Texture2D>& texture,
                          glm::vec2 position, glm::vec2 size,
                          float rotate, glm::vec3 color) const {
    shader->use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model,
                           glm::vec3(size.x / 2, size.y / 2, 0.0f));
    model = glm::rotate(model, glm::radians(rotate),
                        glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model,
                           glm::vec3(-size.x / 2, -size.y / 2, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f));

    shader->setMat4("model", model);
    shader->setVec3("spriteColor", color);
    shader->setTexture("image", 0, texture);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
