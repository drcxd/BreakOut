#include "Game.h"

#include <memory>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SpriteRenderer.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "Utility.h"
#include "GameLevel.h"
#include "GameObject.h"

SpriteRenderer* renderer;
GameObject* player;

Game::Game(int width, int height)
    : width(width)
    , height(height)
    , state(GameState::GAME_MENU)
    , keys{0}
    , quit(false) { }

Game::~Game() { }

void Game::Init() {
    loadResources();

    auto spriteShader = ResourceManager::GetInstance()->
        GetShader("sprite");
    renderer = new SpriteRenderer(spriteShader);

    GameObjectAttribute attr;
    attr.size = glm::vec2(100.0f, 20.0f);
    attr.position = glm::vec2(this->width / 2 - attr.size.x / 2,
                              this->height - attr.size.y);
    attr.velocity = glm::vec2(500.0f, 0.0f);
    attr.color = glm::vec3(1.0f, 1.0f, 1.0f);
    attr.rotation = 0;
    attr.isSolid = true;
    attr.isDestroyed = false;
    attr.texture = ResourceManager::GetInstance()->
        GetTexture2D("paddle");
    player = new GameObject(attr);
}

void Game::ProcessInput(float dt) {
    auto& playerAttr = player->attribute;
    if (keys[GLFW_KEY_A]) {
        playerAttr.position -= dt * playerAttr.velocity;
        playerAttr.position.x = fmax(playerAttr.position.x, 0.0f);
    }
    if (keys[GLFW_KEY_D]) {
        playerAttr.position += dt * playerAttr.velocity;
        playerAttr.position.x = fmin(playerAttr.position.x,
                                     this->width - playerAttr.size.x);
    }
}

void Game::Update(float dt) {
}

void Game::Render() {

    auto background = ResourceManager::GetInstance()->
        GetTexture2D("background");
    renderer->Draw(background, glm::vec2(0.0f, 0.0f),
                   glm::vec2(this->width, this->height), 0.0f,
                   glm::vec3(1.0f, 1.0f, 1.0f));

    levels[0]->Draw(*renderer);

    player->Draw(*renderer);
}

void Game::loadResources() {
    auto spriteShader = ResourceManager::GetInstance()->
        LoadShader("sprite", "./shaders/sprite.vert",
                   "./shaders/sprite.frag");
    glm::mat4 projection =
        glm::ortho(0.0f, (float)width, (float)height, 0.0f,
                   -1.0f, 1.0f);
    spriteShader->use();
    spriteShader->setMat4("projection", projection);

    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/awesomeface.png", "face",
                      true);
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/background.jpg",
                      "background");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/block.png", "brick");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/block_solid.png",
                      "brick_solid");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/paddle.png", "paddle");

    levels.emplace_back(std::make_shared<GameLevel>());
    levels[0]->Load("./resources/levels/one.lvl",
                    this->width, this->height / 2);
}
