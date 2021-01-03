#include "Game.h"

#include <memory>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SpriteRenderer.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "Utility.h"

SpriteRenderer* renderer;

Game::Game(int width, int height)
    : width(width)
    , height(height)
    , state(GameState::GAME_MENU)
    , keys{0}
    , quit(false) { }

Game::~Game() { }

void Game::Init() {
    auto spriteShader = ResourceManager::GetInstance()->
        LoadShader("sprite", "./shaders/sprite.vert",
                   "./shaders/sprite.frag");

    glm::mat4 projection =
        glm::ortho(0.0f, (float)width, (float)height, 0.0f,
                   -1.0f, 1.0f);

    spriteShader->use();
    spriteShader->setMat4("projection", projection);
    renderer = new SpriteRenderer(spriteShader);
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/awesomeface.png",
                      "face", true);
}

void Game::ProcessInput(float dt) {
}

void Game::Update(float dt) {

}

void Game::Render() {
    auto face = ResourceManager::GetInstance()->GetTexture2D("face");
    renderer->Draw(face, glm::vec2(200.0f, 200.0f),
                   glm::vec2(300.0f, 400.0f), 45.0f,
                   glm::vec3(0.0f, 1.0f, 0.0f));

}
