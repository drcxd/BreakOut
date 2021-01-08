#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include <vector>
#include <memory>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

class Shader;
class Texture2D;
class GameObject;

struct Particle {
    glm::vec2 position, velocity;
    glm::vec4 color;
    float life;
    Particle()
        : position(glm::vec2(0.0f))
        , velocity(glm::vec2(0.0f))
        , color(glm::vec4(1.0f))
        , life(0.0f) { }
};

class ParticleGenerator {
public:

    ParticleGenerator(int number,
                      const std::shared_ptr<Shader>& shader,
                      const std::shared_ptr<Texture2D>& texture);
    ~ParticleGenerator();
    void Update(float dt, const GameObject* object,
                int newParticles,
                const glm::vec2& offset = glm::vec2(0.0f));
    void Draw();

private:

    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture2D> texture;
    GLuint VAO;
    GLuint VBO;
    const int number;
    std::vector<Particle> particles;

    void init();
    int getNextParticle() const;
    void respawnParticle(Particle& p,
                         const GameObject* object,
                         const glm::vec2& offset = glm::vec2(0.0f));

    static const float particleQuad[];
};
#endif
