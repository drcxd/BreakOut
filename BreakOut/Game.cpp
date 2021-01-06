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
#include "Particle.h"
#include "PostProcessor.h"

PostProcessor* effects;
SpriteRenderer* renderer;
std::shared_ptr<GameObject> player;
std::shared_ptr<Ball> ball;
std::shared_ptr<ParticleGenerator> particles;

float shakeTime = 0.0f;

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

    ballAttr.size = glm::vec2(2 * ballAttr.radius);
    ballAttr.position = player->Attr()->position +
        glm::vec2(player->Attr()->size.x / 2 - ballAttr.radius,
                  -2 * ballAttr.radius - 1.0f);
    ballAttr.velocity = BALL_VELOCITY;
    ballAttr.color = glm::vec3(1.0f);
    ballAttr.rotation = 0.0f;
    ballAttr.isSolid = true;
    ballAttr.isDestroyed = false;
    ballAttr.texture = ResourceManager::GetInstance()->
        GetTexture2D("face");
    ball = std::make_shared<Ball>(ballAttr);

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

    particles = std::make_shared<ParticleGenerator>(
            500,
            ResourceManager::GetInstance()->GetShader("particle"),
            ResourceManager::GetInstance()->GetTexture2D("particle")
        );

    effects = new PostProcessor(
        ResourceManager::GetInstance()->GetShader("postprocess"),
        width, height);
}

void Game::ProcessInput(float dt) {
    auto& playerAttr = *player->Attr();
    if (keys[GLFW_KEY_A] && keys[GLFW_KEY_D] ||
        !keys[GLFW_KEY_A] && !keys[GLFW_KEY_D]) {
        playerAttr.velocity.x = 0.0f;
    } else if (keys[GLFW_KEY_A]) {
        playerAttr.velocity.x = -500.0f;
    } else {
        playerAttr.velocity.x = 500.0f;
    }

    if (keys[GLFW_KEY_SPACE] && ball->Attr()->isStatic) {
        ball->Attr()->isStatic = false;
        player->children.erase(ball);
        objects.insert(ball);
    }
}

void Game::Update(float dt) {
    for (auto& object : objects) {
        object->Update(dt);
    }
    doCollision();

    particles->Update(dt, ball, 2,
                      glm::vec2(ball->Attr()->radius / 2.0f));

    if (shakeTime > 0.0f) {
        shakeTime -= dt;
        if (shakeTime <= 0.0f) {
            effects->shake = false;
        }
    }
}

void Game::Render() {

    effects->BeginRender();
    auto background = ResourceManager::GetInstance()->
        GetTexture2D("background");
    renderer->Draw(background, glm::vec2(0.0f, 0.0f),
                   glm::vec2(this->width, this->height), 0.0f,
                   glm::vec3(1.0f, 1.0f, 1.0f));

    levels[0]->Draw(*renderer);

    player->Draw(*renderer);
    particles->Draw();
    ball->Draw(*renderer);
    effects->EndRender();
    effects->Render((float)glfwGetTime());
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

    auto particleShader = ResourceManager::GetInstance()->
        LoadShader("particle", "./shaders/particle.vert",
                   "./shaders/particle.frag");
    particleShader->use();
    particleShader->setMat4("projection", projection);

    ResourceManager::GetInstance()->
        LoadShader("postprocess", "./shaders/postprocess.vert",
                   "./shaders/postprocess.frag");

    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/particle.png",
                      "particle", false);
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
        if (!brick->Attr()->isDestroyed && info.isCollided) {
            if (!brick->Attr()->isSolid) {
                brick->Attr()->isDestroyed = true;
            } else {
                shakeTime = 0.05f;
                effects->shake = true;
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
        float playerCenter = player->Attr()->position.x +
            player->Attr()->size.x / 2.0f;
        float dist = ball->Attr()->position.x +
            ball->Attr()->radius - playerCenter;
        float percent = dist / (player->Attr()->size.x / 2.0f);
        glm::vec2 oldVelocity = ball->Attr()->velocity;
        ball->Attr()->velocity.x = BALL_VELOCITY.x * percent * 2.0f;
        ball->Attr()->velocity =
            glm::normalize(ball->Attr()->velocity) *
            glm::length(BALL_VELOCITY);
        ball->Attr()->velocity.y = -1.0f *
            std::fabs(ball->Attr()->velocity.y);
    }
}

CollisionInfo
Game::checkCollision(std::shared_ptr<Ball>& ball,
                     std::shared_ptr<GameObject>& brick) {
    glm::vec2 ballCenter = ball->Attr()->position +
        glm::vec2(ball->Attr()->radius);
    glm::vec2 halfSize = brick->Attr()->size * 0.5f;
    glm::vec2 brickCenter = brick->Attr()->position + halfSize;
    glm::vec2 dist = ballCenter - brickCenter;
    glm::vec2 closest = glm::clamp(dist, -halfSize, halfSize) +
        brickCenter;

    dist = closest - ballCenter;

    CollisionInfo info;
    info.isCollided = glm::length(dist) < ball->Attr()->radius;
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
        ball->Attr()->velocity.x *= -1.0f;
        float penetration = ball->Attr()->radius -
            std::fabs(info.displace.x);
        ball->Attr()->position += info.direction.x == 1.0f ?
            -penetration : +penetration;
    } else {
        ball->Attr()->velocity.y *= -1.0f;
        float penetration = ball->Attr()->radius -
            std::fabs(info.displace.y);
        ball->Attr()->position += info.direction.y == 1.0f ?
            -penetration : penetration;
    }
}