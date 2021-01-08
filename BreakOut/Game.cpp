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
#include <ik/irrKlang.h>

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
#include "TextRenderer.h"

irrklang::ISoundEngine *soundEngine = irrklang::createIrrKlangDevice();

float shakeTime = 0.0f;

const glm::vec2 BALL_VELOCITY = glm::vec2(200.0f, -200.0f);
const glm::vec2 PLAYER_SIZE = glm::vec2(100.0f, 20.0f);

Game::Game(int width, int height)
    : Width(width)
    , Height(height) { }

Game::~Game() { }

void Game::Init() {
    loadResources();

    auto spriteShader = ResourceManager::GetInstance()->
        GetShader("sprite");
    sprite_renderer = std::make_unique<SpriteRenderer>(spriteShader);

    auto fontShader = ResourceManager::GetInstance()->
        GetShader("text");
    text_renderer = std::make_unique<TextRenderer>(fontShader);

    // Player
    GameObjectAttribute attr;
    attr.size = glm::vec2(100.0f, 20.0f);
    attr.position = glm::vec2(this->Width / 2 - attr.size.x / 2,
                              this->Height - attr.size.y);
    attr.velocity = glm::vec2(0.0f, 0.0f);
    attr.color = glm::vec3(1.0f, 1.0f, 1.0f);
    attr.rotation = 0;
    attr.isSolid = true;
    attr.isDestroyed = false;
    attr.texture = ResourceManager::GetInstance()->
        GetTexture2D("paddle");
    player = std::make_unique<GameObject>(attr);
    objects.insert(player.get());

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
    ball = std::make_unique<Ball>(ballAttr);
    player->children.insert(ball.get());

    // Boundary
    attr.size = glm::vec2(Width, 1.0f);
    attr.position = glm::vec2(0.0f, -1.0f);
    attr.isSolid = true;
    attr.isDestroyed = false;
    boundary.emplace_back(std::make_unique<GameObject>(attr));

    attr.size = glm::vec2(1.0f, Height);
    attr.position = glm::vec2(-1.0f, 0.0f);
    boundary.emplace_back(std::make_unique<GameObject>(attr));

    attr.position = glm::vec2(Width, 0.0f);
    boundary.emplace_back(std::make_unique<GameObject>(attr));

    particles = std::make_unique<ParticleGenerator>(
            500,
            ResourceManager::GetInstance()->GetShader("particle"),
            ResourceManager::GetInstance()->GetTexture2D("particle")
        );

    effects = std::make_unique<PostProcessor>(
        ResourceManager::GetInstance()->GetShader("postprocess"),
        Width, Height);

    soundEngine->play2D("./resources/audio/breakout.mp3", true);
}

void Game::ProcessInput(float dt) {
    if (State == GameState::GAME_ACTIVE) {
        auto& playerAttr = *player->Attr();
        if (Keys[GLFW_KEY_A] && Keys[GLFW_KEY_D] ||
            !Keys[GLFW_KEY_A] && !Keys[GLFW_KEY_D]) {
            playerAttr.velocity.x = 0.0f;
        } else if (Keys[GLFW_KEY_A]) {
            playerAttr.velocity.x = -500.0f;
        } else {
            playerAttr.velocity.x = 500.0f;
        }

        if (Keys[GLFW_KEY_SPACE] && ball->Attr()->isStatic) {
            ball->Attr()->isStatic = false;
            player->children.erase(ball.get());
            objects.insert(ball.get());
        }

        if (Keys[GLFW_KEY_C] && !Processed[GLFW_KEY_C]) {
            effects->chaos = true;
            State = GameState::GAME_WIN;
        }
    } else if (State == GameState::GAME_MENU) {
        if (Keys[GLFW_KEY_W] && !Processed[GLFW_KEY_W]) {
            --level;
            level = (level + 4) % 4;
            Processed[GLFW_KEY_W] = true;
        }
        if (Keys[GLFW_KEY_S] && !Processed[GLFW_KEY_S]) {
            ++level;
            level %= 4;
            Processed[GLFW_KEY_S] = true;
        }
        if (Keys[GLFW_KEY_ENTER] && !Processed[GLFW_KEY_ENTER]) {
            State = GameState::GAME_ACTIVE;
            Processed[GLFW_KEY_ENTER] = true;
            reset_player();
            reset_ball();
            clear_powerups();
            player->children.insert(ball.get());
            objects.erase(ball.get());
            play_ball = 2;
        }
    } else if (State == GameState::GAME_WIN) {
        if (Keys[GLFW_KEY_ENTER] && !Processed[GLFW_KEY_ENTER]) {
            reset_level();
            effects->chaos = false;
            State = GameState::GAME_MENU;
            Processed[GLFW_KEY_ENTER] = true;
        }
    }

}

void Game::Update(float dt) {
    if (State == GameState::GAME_ACTIVE) {
        for (auto& object : objects) {
            object->Update(dt);
        }
        doCollision();

        particles->Update(dt, ball.get(), 2,
                          glm::vec2(ball->Attr()->radius / 2.0f));

        updatePowerUps(dt);

        if (shakeTime > 0.0f) {
            shakeTime -= dt;
            if (shakeTime <= 0.0f) {
                effects->shake = false;
            }
        }

        if (ball->Attr()->position.y > Height + 200) {
            --play_ball;
            if (play_ball >= 0) {
                reset_player();
                reset_ball();
                player->children.insert(ball.get());
                objects.erase(ball.get());
            } else {
                State = GameState::GAME_MENU;
            }
        }

        if (levels[level]->IsComplete()) {
            State = GameState::GAME_WIN;
            effects->chaos = true;
        }
    }
}

void Game::Render() {
    effects->BeginRender();

    auto background = ResourceManager::GetInstance()->
        GetTexture2D("background");
    sprite_renderer->Draw(background, glm::vec2(0.0f, 0.0f),
                   glm::vec2(this->Width, this->Height), 0.0f,
                   glm::vec3(1.0f, 1.0f, 1.0f));

    levels[level]->Draw(*sprite_renderer);
    for (auto& p : powerUps) {
        if (!p->Attr()->isDestroyed) {
            p->Draw(*sprite_renderer);
        }
    }
    player->Draw(*sprite_renderer);
    particles->Draw();
    ball->Draw(*sprite_renderer);

    text_renderer->RenderText(fmt::format("Ball: {}", play_ball),
                             glm::vec2(0.0f, 0.0f), 0.5f);

    if (State == GameState::GAME_MENU) {
        text_renderer->RenderText("Press ENTER to start",
                                 glm::vec2(Width / 2 - 150, Height / 2 - 50),
                                 0.75f);
        text_renderer->RenderText("Press W or S to select level",
                                 glm::vec2(Width / 2 - 200, Height / 2 + 48 - 50),
                                 0.75f);
    }

    if (State == GameState::GAME_WIN) {
        text_renderer->RenderText("You Won!",
                                 glm::vec2(Width / 2 - 100, Height / 2 - 50),
                                 0.75f);
        text_renderer->RenderText("Press Enter to Retry, Press ESC to quit",
                                 glm::vec2(Width / 2 - 250, Height/ 2 + 48 - 50),
                                 0.75f);
    }

    effects->EndRender();
    effects->Render((float)glfwGetTime());
}

void Game::loadResources() {
    auto spriteShader = ResourceManager::GetInstance()->
        LoadShader("sprite", "./shaders/sprite.vert",
                   "./shaders/sprite.frag");
    glm::mat4 projection =
        glm::ortho(0.0f, (float)Width, (float)Height, 0.0f,
                   -1.0f, 1.0f);
    spriteShader->use();
    spriteShader->setMat4("projection", projection);

    auto particleShader = ResourceManager::GetInstance()->
        LoadShader("particle", "./shaders/particle.vert",
                   "./shaders/particle.frag");
    particleShader->use();
    particleShader->setMat4("projection", projection);

    projection = glm::ortho(0.0f, (float)Width,
                            float(Height), 0.0f);
    auto textShader = ResourceManager::GetInstance()->
        LoadShader("text", "./shaders/font.vert",
                   "./shaders/font.frag");
    textShader->use();
    textShader->setMat4("projection", projection);

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
    for (int i = 0; i < 4; ++i) {
        levels.emplace_back(std::make_unique<GameLevel>());
    }
    levels[0]->Load("./resources/levels/one.lvl",
                    this->Width, this->Height / 2);
    levels[1]->Load("./resources/levels/two.lvl",
                    this->Width, this->Height / 2);
    levels[2]->Load("./resources/levels/three.lvl",
                    this->Width, this->Height / 2);
    levels[3]->Load("./resources/levels/four.lvl",
                    this->Width, this->Height / 2);
}

void Game::doCollision() {
    // Ball VS Bricks
    for (auto& brick : levels[level]->bricks) {
        auto info = checkCollision(ball.get(), brick.get());
        if (!brick->Attr()->isDestroyed && info.isCollided) {
            if (!brick->Attr()->isSolid) {
                brick->Attr()->isDestroyed = true;
                spawnPowerUps(brick.get());
                if (!ball->Attr()->isPassThrough) {
                    applyCollision(ball.get(), info);
                }
                soundEngine->
                    play2D("./resources/audio/bleep.mp3", false);
            } else {
                shakeTime = 0.05f;
                effects->shake = true;
                applyCollision(ball.get(), info);
                soundEngine->
                    play2D("./resources/audio/solid.wav", false);
            }
        }
    }

    // Ball VS Boundary
    for (auto& bound : boundary) {
        auto info = checkCollision(ball.get(), bound.get());
        if (info.isCollided) {
            applyCollision(ball.get(), info);
        }
    }

    // Ball VS Player
    auto info = checkCollision(ball.get(), player.get());
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
            objects.erase(ball.get());
            player->children.insert(ball.get());
            ball->Attr()->isStatic = true;
        }
        soundEngine->play2D("./resources/audio/bleep.wav", false);
    }

    // Power up VS Player
    for (auto &p : powerUps) {
        if (p->Attr()->isDestroyed) {
            continue;
        }
        if (p->Attr()->position.y >= Height) {
            p->Attr()->isDestroyed = true;
            continue;
        }
        if (checkCollision(player.get(), p.get())) {
            activatePowerUp(p.get());
            p->Attr()->isDestroyed = true;
            p->Attr()->isActive = true;
            soundEngine->
                play2D("./resources/audio/powerup.wav", false);
        }
    }
}

bool Game::checkCollision(const GameObject* obj1,
                          const GameObject* obj2) const {
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
Game::checkCollision(const Ball* ball,
                     const GameObject* brick) {
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

void Game::applyCollision(Ball* ball, const CollisionInfo& info) {
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

void Game::spawnPowerUps(const GameObject* object) {
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
        powerUps.push_back(std::make_unique<PowerUp>(puAttr));
    }
    if (shouldSpawn(75)) {
        puAttr.type = "sticky";
        puAttr.color = glm::vec3(1.0f, 0.5f, 1.0f);
        puAttr.duration = 20.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_unique<PowerUp>(puAttr));
    }
    if (shouldSpawn(75)) {
        puAttr.type = "pass-through";
        puAttr.color = glm::vec3(0.5f, 1.0f, 0.5f);
        puAttr.duration = 10.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_unique<PowerUp>(puAttr));
    }
    if (shouldSpawn(75)) {
        puAttr.type = "pad-size-increase";
        puAttr.color = glm::vec3(1.0f, 0.6f, 0.4f);
        puAttr.duration = 0.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_unique<PowerUp>(puAttr));
    }
    if (shouldSpawn(15)) {
        puAttr.type = "confuse";
        puAttr.color = glm::vec3(1.0f, 0.3f, 0.3f);
        puAttr.duration = 3.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_unique<PowerUp>(puAttr));
    }
    if (shouldSpawn(15)) {
        puAttr.type = "chaos";
        puAttr.color = glm::vec3(0.9f, 0.25f, 0.25f);
        puAttr.duration = 3.0f;
        puAttr.texture = ResourceManager::GetInstance()->
            GetTexture2D(puAttr.type);
        powerUps.push_back(std::make_unique<PowerUp>(puAttr));
    }
}

void Game::activatePowerUp(const PowerUp* p) {
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
            [] (const std::unique_ptr<PowerUp>& p) {
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

void Game::reset_player() {
    player->Attr()->size = PLAYER_SIZE;
    player->Attr()->position = glm::vec2(
        Width / 2.0f - player->Attr()->size.x / 2.0f,
        Height  - player->Attr()->size.y
        );
}

void Game::reset_ball() {
    ball->Attr()->velocity = BALL_VELOCITY;
    ball->Attr()->position = glm::vec2(
        player->Attr()->position.x + player->Attr()->size.x / 2.0f -
        ball->Attr()->size.x / 2.0f,
        player->Attr()->position.y - ball->Attr()->size.y
        );
    ball->Attr()->isStatic = true;
}

void Game::clear_powerups() {
    for (auto& p : powerUps) {
        onPowerUpEnd(p.get());
    }
    powerUps.clear();
}

void Game::reset_level() {
    levels[level]->Reset();
}