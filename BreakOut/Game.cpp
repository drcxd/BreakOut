#include "Game.h"

#include <memory>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <functional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>

#include "SpriteRenderer.h"
#include "ResourceManager.h"
#include "Shader.h"
#include "Utility.h"
#include "GameLevel.h"
#include "GameObject.h"
#include "Ball.h"
#include "Particle.h"
#include "PostProcessor.h"
#include "PowerUp.h"

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

    updatePowerUps(dt);

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
    for (auto& p : powerUps) {
        if (!p->Attr()->isDestroyed) {
            p->Draw(*renderer);
        }
    }

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
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/powerup_chaos.png",
                      "chaos");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/powerup_confuse.png",
                      "confuse");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/powerup_increase.png",
                      "pad-size-increase");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/powerup_passthrough.png",
                      "pass-through");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/powerup_speed.png",
                      "speed");
    ResourceManager::GetInstance()->
        LoadTexture2D("./resources/textures/powerup_sticky.png",
                      "sticky");

    levels.emplace_back(std::make_shared<GameLevel>());
    levels[0]->Load("./resources/levels/one.lvl",
                    this->width, this->height / 2);
}

void Game::doCollision() {
    // Ball VS Bricks
    for (auto& brick : levels[0]->bricks) {
        auto info = checkCollision(ball, brick);
        if (!brick->Attr()->isDestroyed && info.isCollided) {
            if (!brick->Attr()->isSolid) {
                brick->Attr()->isDestroyed = true;
                spawnPowerUps(brick);
                if (!ball->Attr()->isPassThrough) {
                    applyCollision(ball, info);
                }
            } else {
                shakeTime = 0.05f;
                effects->shake = true;
                applyCollision(ball, info);
            }
        }
    }

    // Ball VS Boundary
    for (auto& bound : boundary) {
        auto info = checkCollision(ball, bound);
        if (info.isCollided) {
            applyCollision(ball, info);
        }
    }

    // Ball VS Player
    auto info = checkCollision(ball, player);
    if (!ball->Attr()->isStatic && info.isCollided) {
        float playerCenter = player->Attr()->position.x +
            player->Attr()->size.x / 2.0f;
        float dist = ball->Attr()->position.x +
            ball->Attr()->radius - playerCenter;
        float percent = dist / (player->Attr()->size.x / 2.0f);
        percent = glm::clamp(percent, -1.0f, 1.0f);
        glm::vec2 oldVelocity = ball->Attr()->velocity;
        ball->Attr()->velocity.x = BALL_VELOCITY.x * percent * 2.0f;
        ball->Attr()->velocity =
            glm::normalize(ball->Attr()->velocity) *
            glm::length(oldVelocity);
        ball->Attr()->velocity.y = -1.0f *
            std::fabs(ball->Attr()->velocity.y);
        if (ball->Attr()->isSticky) {
            objects.erase(ball);
            player->children.insert(ball);
            ball->Attr()->isStatic = true;
        }
    }

    // Power up VS Player
    for (auto &p : powerUps) {
        if (p->Attr()->isDestroyed) {
            continue;
        }
        if (p->Attr()->position.y >= height) {
            p->Attr()->isDestroyed = true;
            continue;
        }
        if (checkCollision(player, p)) {
            activatePowerUp(p);
            p->Attr()->isDestroyed = true;
            p->Attr()->isActive = true;
        }
    }
}

bool Game::checkCollision(
    const std::shared_ptr<GameObject>& obj1,
    const std::shared_ptr<GameObject>& obj2) const {
    bool collisionX =
        obj1->Attr()->position.x + obj1->Attr()->size.x >=
        obj2->Attr()->position.x &&
        obj2->Attr()->position.x + obj2->Attr()->size.x >=
        obj1->Attr()->position.x;
    bool collisionY =
        obj1->Attr()->position.y + obj1->Attr()->size.y >=
        obj2->Attr()->position.y &&
        obj2->Attr()->position.y + obj2->Attr()->size.y >=
        obj1->Attr()->position.y;
    return collisionX && collisionY;
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
        ball->Attr()->position.x += info.direction.x == 1.0f ?
            -penetration : +penetration;
    } else {
        ball->Attr()->velocity.y *= -1.0f;
        float penetration = ball->Attr()->radius -
            std::fabs(info.displace.y);
        ball->Attr()->position.y += info.direction.y == 1.0f ?
            -penetration : penetration;
    }
}

bool Game::shouldSpawn(int chance) const {
    int random = rand() % chance;
    return random == 0;
}

void Game::spawnPowerUps(const std::shared_ptr<GameObject>& object) {
    PowerUpAttribute puAttr;
    puAttr.size = glm::vec2(60.0f, 20.0f);
    puAttr.velocity = glm::vec2(0.0f, 150.0f);
    puAttr.position = object->Attr()->position;
    puAttr.endCallback = [=](const PowerUp* p) ->void {
        this->onPowerUpEnd(p);
    };
    if (shouldSpawn(75)) {
        puAttr.type = "speed";
        puAttr.color = glm::vec3(0.5f, 0.5f, 1.0f);
        puAttr.duration = 0.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_shared<PowerUp>(puAttr));
    }
    if (shouldSpawn(75)) {
        puAttr.type = "sticky";
        puAttr.color = glm::vec3(1.0f, 0.5f, 1.0f);
        puAttr.duration = 20.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_shared<PowerUp>(puAttr));
    }
    if (shouldSpawn(75)) {
        puAttr.type = "pass-through";
        puAttr.color = glm::vec3(0.5f, 1.0f, 0.5f);
        puAttr.duration = 10.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_shared<PowerUp>(puAttr));
    }
    if (shouldSpawn(75)) {
        puAttr.type = "pad-size-increase";
        puAttr.color = glm::vec3(1.0f, 0.6f, 0.4f);
        puAttr.duration = 0.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_shared<PowerUp>(puAttr));
    }
    if (shouldSpawn(15)) {
        puAttr.type = "confuse";
        puAttr.color = glm::vec3(1.0f, 0.3f, 0.3f);
        puAttr.duration = 3.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_shared<PowerUp>(puAttr));
    }
    if (shouldSpawn(15)) {
        puAttr.type = "chaos";
        puAttr.color = glm::vec3(0.9f, 0.25f, 0.25f);
        puAttr.duration = 3.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_shared<PowerUp>(puAttr));
    }
}

void Game::activatePowerUp(const std::shared_ptr<PowerUp>& p) {
    if (p->Attr()->type == "speed") {
        ball->Attr()->velocity *= 1.2;
    } else if (p->Attr()->type == "sticky") {
        ball->Attr()->isSticky = true;
        player->Attr()->color = glm::vec3(1.0f, 0.5f, 1.0f);
    } else if (p->Attr()->type == "pass-through") {
        ball->Attr()->isPassThrough = true;
        ball->Attr()->color = glm::vec3(1.0f, 0.5f, 0.5f);
    } else if (p->Attr()->type == "pad-size-increase") {
        player->Attr()->size.x += 50;
    } else if (p->Attr()->type == "confuse") {
        if (!effects->chaos) {
            effects->confuse = true;
        }
    } else if (p->Attr()->type == "chaos") {
        if (!effects->confuse) {
            effects->chaos = true;
        }
    }
}

void Game::updatePowerUps(float dt) {
    for (auto& p : powerUps) {
        p->Update(dt);
    }
    powerUps.erase(
        std::remove_if(
            powerUps.begin(), powerUps.end(),
            [] (const std::shared_ptr<PowerUp>& p) {
                return p->Attr()->isDestroyed &&
                    !p->Attr()->isActive;
            }),
        powerUps.end());
}

bool Game::otherActivePowerUp(const std::string& type) {
    for (const auto& p : powerUps) {
        if (p->Attr()->isActive && p->Attr()->type == type) {
            return true;
        }
    }
    return false;
}

void Game::onPowerUpEnd(const PowerUp* p) {
    const auto& type = p->Attr()->type;
    if (type == "sticky") {
        ball->Attr()->isSticky = false;
        player->Attr()->color = glm::vec3(1.0f);
    } else if (type == "pass-through") {
        ball->Attr()->isPassThrough = false;
        ball->Attr()->color = glm::vec3(1.0f);
    } else if (type == "confuse") {
        if (!otherActivePowerUp("confuse")) {
            effects->confuse = false;
        }
    } else if (type == "chaos") {
        if (!otherActivePowerUp("chaos")) {
            effects->chaos = false;
        }
    }
}
