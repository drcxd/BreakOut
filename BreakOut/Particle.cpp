#include "Particle.h"

#include <random>

#include "Shader.h"
#include "GameObject.h"
#include "Utility.h"

const float ParticleGenerator::particleQuad[] = {
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f,

    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 1.0f, 0.0f
};

ParticleGenerator::
ParticleGenerator(int number, const std::shared_ptr<Shader>& shader,
                  const std::shared_ptr<Texture2D>& texture)
    : number(number)
    , shader(shader)
    , texture(texture)
    , particles(number) {
    init();
}

ParticleGenerator::
~ParticleGenerator() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void
ParticleGenerator::
Update(float dt, const std::shared_ptr<GameObject> &object,
       int newParticles, const glm::vec2& offset) {
    for (int i = 0; i < newParticles; ++i) {
        int j = getNextParticle();
        respawnParticle(particles[j], object, offset);
    }

    for (auto& p : particles) {
        p.life -= dt;
        if (p.life >= 0) {
            p.position -= p.velocity * dt;
            p.color.a -= dt * 2.5f;
        }
    }
}

void ParticleGenerator::Draw() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    shader->use();
    for (auto& p : particles) {
        if (p.life > 0.0f) {
            shader->setVec2("offset", p.position);
            shader->setVec4("color", p.color);
            shader->setTexture("sprite", 0, texture);
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int ParticleGenerator::getNextParticle() const {
    static int idx = 0;
    for (int i = idx; i < particles.size(); ++i) {
        if (particles[i].life <= 0.0f) {
            idx = i + 1;
            return i;
        }
    }

    for (int i = 0; i < particles.size(); ++i) {
        if (particles[i].life <= 0.0f) {
            idx = i + 1;
            return i;
        }
    }

    idx = 1;
    return 0;
}

void ParticleGenerator::respawnParticle(
    Particle& p,
    const std::shared_ptr<GameObject>& object,
    const glm::vec2& offset) {
    float random = ((rand() % 100) - 50) / 10.0f;
    float color = 0.5f + ((rand() % 100) / 100.0f);
    p.position = object->attribute.position +
        glm::vec2(random) + offset;
    p.color = glm::vec4(glm::vec3(color), 1.0f);
    p.life = 1.0f;
    p.velocity = object->attribute.velocity * 0.1f;
}

void ParticleGenerator::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleQuad),
                 particleQuad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
