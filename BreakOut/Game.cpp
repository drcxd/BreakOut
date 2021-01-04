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
#include "Ball.h"

SpriteRenderer* renderer;
std::shared_ptr<GameObject> player;
std::shared_ptr<Ball> ball;

Game::Game(int width, int height)
    : width(width)
    , height(height)
    , state(GameState::GAME_MENU)
    , keys{0} { }

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
    attr.velocity = glm::vec2(0.0f, 0.0f);
    attr.color = glm::vec3(1.0f, 1.0f, 1.0f);
    attr.rotation = 0;
    attr.isSolid = true;
    attr.isDestroyed = false;
    attr.texture = ResourceManager::GetInstance()->
        GetTexture2D("paddle");
    player = std::make_shared<GameObject>(attr);

    BallAttribute ballAttr;
    ballAttr.radius = 10.0f;
    ballAttr.isStatic = true;

    attr.size = glm::vec2(2 * ballAttr.radius);
    attr.position = player->attribute.position +
        glm::vec2(player->attribute.size.x / 2 - ballAttr.radius,
                  -2 * ballAttr.radius);
    attr.velocity = glm::vec2(300.0f, 300.0f);
    attr.color = glm::vec3(1.0f);
    attr.rotation = 0.0f;
    attr.isSolid = true;
    attr.isDestroyed = false;
    attr.texture = ResourceManager::GetInstance()->
        GetTexture2D("face");
    ball = std::make_shared<Ball>(ballAttr, attr);

    player->children.insert(ball);
}

void Game::ProcessInput(float dt) {
    auto& playerAttr = player->attribute;
    if (keys[GLFW_KEY_A] && keys[GLFW_KEY_D] ||
        !keys[GLFW_KEY_A] && !keys[GLFW_KEY_D]) {
        playerAttr.velocity.x = 0.0f;
    } else if (keys[GLFW_KEY_A]) {
        // playerAttr.velocity.x =
        //     playerAttr.position.x > 0 ? -500.0f : 0.0f;
        playerAttr.velocity.x = -500.0f;
    } else {
        // playerAttr.velocity.x =
        //     playerAttr.position.x + playerAttr.size.x < this->width ?
        //     500.0f : 0.0f;
        playerAttr.velocity.x = 500.0f;
    }
}

void Game::Update(float dt) {
    player->Update(dt);
}

void Game::Render() {

    auto background = ResourceManager::GetInstance()->
        GetTexture2D("background");
    renderer->Draw(background, glm::vec2(0.0f, 0.0f),
                   glm::vec2(this->width, this->height), 0.0f,
                   glm::vec3(1.0f, 1.0f, 1.0f));

    levels[0]->Draw(*renderer);

    player->Draw(*renderer);
    ball->Draw(*renderer);
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
