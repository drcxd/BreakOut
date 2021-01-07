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

    GameState state = GameState::GAME_MENU;
    bool keys[1024];
    bool processed[1024];
    int width, height;

private:

    int play_ball = 2;
    int level = 0;

    // Objects
    std::vector<std::shared_ptr<GameObject>> boundary;
    std::vector<std::shared_ptr<GameLevel>> levels;
    std::unordered_set<std::shared_ptr<GameObject>> objects;
    std::vector<std::shared_ptr<PowerUp>> powerUps;

    // PowerUp
    bool shouldSpawn(int chance) const;
    void spawnPowerUps(const std::shared_ptr<GameObject>& brick);
    void updatePowerUps(float dt);
    void activatePowerUp(const std::shared_ptr<PowerUp>& p);
    bool otherActivePowerUp(const std::string& type);
    void onPowerUpEnd(const PowerUp* p);

    // Resources
    void loadResources();

    // Collision
    void doCollision();
    bool checkCollision(const std::shared_ptr<GameObject>& obj1,
                        const std::shared_ptr<GameObject>& obj2) const;
    CollisionInfo checkCollision(std::shared_ptr<Ball>& ball,
                                 std::shared_ptr<GameObject>& brick);
    glm::vec2 calculateCollisionDirection(const glm::vec2& dir);
    void applyCollision(std::shared_ptr<Ball>& ball,
                        const CollisionInfo& info);

    // Game Logic
    void reset_player();
    void reset_ball();
    void reset_level();
    void clear_powerups();
};

#endif
