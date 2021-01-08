#ifndef __GAME_H__
#define __GAME_H__

#include <vector>
#include <memory>
#include <unordered_set>

#include <glm/gtc/type_ptr.hpp>

class GameLevel;
class Ball;
class GameObject;
class PowerUp;
class TextRenderer;
class SpriteRenderer;
class PostProcessor;
class ParticleGenerator;

enum class GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
};

struct CollisionInfo {
    bool isCollided;
    glm::vec2 direction;
    glm::vec2 displace;
};

class Game{
public:
    Game(int width, int height);
    ~Game();
    void Init();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();

    GameState State = GameState::GAME_MENU;
    bool Keys[1024] = {0};
    bool Processed[1024] = {0};
    int Width, Height;

private:

    int play_ball = 2;
    int level = 0;

    // Objects
    std::vector<std::unique_ptr<GameObject>> boundary;
    std::vector<std::unique_ptr<GameLevel>> levels;
    std::vector<std::unique_ptr<PowerUp>> powerUps;
    std::unique_ptr<GameObject> player;
    std::unique_ptr<Ball> ball;
    std::unordered_set<GameObject*> objects;

    // Rendering
    std::unique_ptr<PostProcessor> effects;
    std::unique_ptr<SpriteRenderer> sprite_renderer;
    std::unique_ptr<TextRenderer> text_renderer;
    std::unique_ptr<ParticleGenerator> particles;

    // PowerUp
    bool shouldSpawn(int chance) const;
    void spawnPowerUps(const GameObject* brick);
    void updatePowerUps(float dt);
    void activatePowerUp(const PowerUp* p);
    bool otherActivePowerUp(const std::string& type);
    void onPowerUpEnd(const PowerUp* p);

    // Resources
    void loadResources();

    // Collision
    void doCollision();
    bool checkCollision(const GameObject* obj1,
                        const GameObject* obj2) const;
    CollisionInfo checkCollision(const Ball* ball,
                                 const GameObject* brick);
    glm::vec2 calculateCollisionDirection(const glm::vec2& dir);
    void applyCollision(Ball* ball, const CollisionInfo& info);

    // Game Logic
    void reset_player();
    void reset_ball();
    void reset_level();
    void clear_powerups();
};

#endif
