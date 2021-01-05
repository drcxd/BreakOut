#include "Game.h"

#include <memory>
#include <cmath>
#include <cassert>

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

const glm::vec2 BALL_VELOCITY = glm::vec2(200.0f, -200.0f);

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

    // Player
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

    // Ball
    BallAttribute ballAttr;
    ballAttr.radius = 10.0f;
    ballAttr.isStatic = true;

    attr.size = glm::vec2(2 * ballAttr.radius);
    attr.position = player->attribute.position +
        glm::vec2(player->attribute.size.x / 2 - ballAttr.radius,
                  -2 * ballAttr.radius - 1.0f);
    attr.velocity = BALL_VELOCITY;
    attr.color = glm::vec3(1.0f);
    attr.rotation = 0.0f;
    attr.isSolid = true;
    attr.isDestroyed = false;
    attr.texture = ResourceManager::GetInstance()->
        GetTexture2D("face");
    ball = std::make_shared<Ball>(ballAttr, attr);

    // Boundary
    attr.size = glm::vec2(width, 1.0f);
    attr.position = glm::vec2(0.0f, -1.0f);
    attr.isSolid = true;
    attr.isDestroyed = false;
    boundary.emplace_back(std::make_shared<GameObject>(attr));

    attr.size = glm::vec2(1.0f, height);
    attr.position = glm::vec2(-1.0f, 0.0f);
    boundary.emplace_back(std::make_shared<GameObject>(attr));

    attr.position = glm::vec2(width, 0.0f);
    boundary.emplace_back(std::make_shared<GameObject>(attr));

    player->children.insert(ball);
    objects.insert(player);
}

void Game::ProcessInput(float dt) {
    auto& playerAttr = player->attribute;
    if (keys[GLFW_KEY_A] && keys[GLFW_KEY_D] ||
        !keys[GLFW_KEY_A] && !keys[GLFW_KEY_D]) {
        playerAttr.velocity.x = 0.0f;
    } else if (keys[GLFW_KEY_A]) {
        playerAttr.velocity.x = -500.0f;
    } else {
        playerAttr.velocity.x = 500.0f;
    }

    if (keys[GLFW_KEY_SPACE] && ball->ballAttribute.isStatic) {
        ball->ballAttribute.isStatic = false;
        player->children.erase(ball);
        objects.insert(ball);
    }
}

void Game::Update(float dt) {
    for (auto& object : objects) {
        object->Update(dt);
    }
    doCollision();
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

void Game::doCollision() {
    for (auto& brick : levels[0]->bricks) {
        auto info = checkCollision(ball, brick);
        if (!brick->attribute.isDestroyed && info.isCollided) {
            if (!brick->attribute.isSolid) {
                brick->attribute.isDestroyed = true;
            }
            applyCollision(ball, info);
        }
    }

    for (auto& bound : boundary) {
        auto info = checkCollision(ball, bound);
        if (info.isCollided) {
            applyCollision(ball, info);
        }
    }

    auto info = checkCollision(ball, player);
    if (info.isCollided) {
        float playerCenter = player->attribute.position.x +
            player->attribute.size.x / 2.0f;
        float dist = ball->attribute.position.x +
            ball->ballAttribute.radius - playerCenter;
        float percent = dist / (player->attribute.size.x / 2.0f);
        glm::vec2 oldVelocity = ball->attribute.velocity;
        ball->attribute.velocity.x = BALL_VELOCITY.x * percent * 2.0f;
        ball->attribute.velocity =
            glm::normalize(ball->attribute.velocity) *
            glm::length(BALL_VELOCITY);
        ball->attribute.velocity.y = -1.0f *
            std::fabs(ball->attribute.velocity.y);
    }
}

CollisionInfo
Game::checkCollision(std::shared_ptr<Ball>& ball,
                     std::shared_ptr<GameObject>& brick) {
    glm::vec2 ballCenter = ball->attribute.position +
        glm::vec2(ball->ballAttribute.radius);
    glm::vec2 halfSize = brick->attribute.size * 0.5f;
    glm::vec2 brickCenter = brick->attribute.position + halfSize;
    glm::vec2 dist = ballCenter - brickCenter;
    glm::vec2 closest = glm::clamp(dist, -halfSize, halfSize) +
        brickCenter;

    dist = closest - ballCenter;

    CollisionInfo info;
    info.isCollided = glm::length(dist) < ball->ballAttribute.radius;
    info.direction= info.isCollided ?
        calculateCollisionDirection(dist) :
        glm::vec2(1.0f);
    info.displace = dist;

    return info;
}

glm::vec2
Game::calculateCollisionDirection(const glm::vec2& dir) {
    static glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
        glm::vec2(0.0f, -1.0f),
        glm::vec2(-1.0f, 0.0f),
    };
    float max = 0.0f;
    int best = -1;
    for (int i = 0; i < 4; ++i) {
        float dotProduct = glm::dot(glm::normalize(compass[i]), dir);
        if (dotProduct > max) {
            max = dotProduct;
            best = i;
        }
    }
    return compass[best];
}

void Game::applyCollision(std::shared_ptr<Ball>& ball,
                          const CollisionInfo& info) {
    if (info.direction == glm::vec2(1.0f, 0.0f) ||
        info.direction == glm::vec2(-1.0f, 0.0f)) {
        ball->attribute.velocity.x *= -1.0f;
        float penetration = ball->ballAttribute.radius -
            std::fabs(info.displace.x);
        ball->attribute.position += info.direction.x == 1.0f ?
            -penetration : +penetration;
    } else {
        ball->attribute.velocity.y *= -1.0f;
        float penetration = ball->ballAttribute.radius -
            std::fabs(info.displace.y);
        ball->attribute.position += info.direction.y == 1.0f ?
            -penetration : penetration;
    }
}