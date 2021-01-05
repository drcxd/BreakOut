#ifndef __GAME_H__
#define __GAME_H__

#include <vector>
#include <memory>
#include <unordered_set>

#include <glm/gtc/type_ptr.hpp>

class GameLevel;
class Ball;
class GameObject;

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

    GameState state;
    bool keys[1024];
    int width, height;

private:

    std::vector<std::shared_ptr<GameObject>> boundary;
    std::vector<std::shared_ptr<GameLevel>> levels;
    void loadResources();

    void doCollision();
    CollisionInfo checkCollision(std::shared_ptr<Ball>& ball,
                                 std::shared_ptr<GameObject>& brick);
    std::unordered_set<std::shared_ptr<GameObject>> objects;
    glm::vec2 calculateCollisionDirection(const glm::vec2& dir);
    void applyCollision(std::shared_ptr<Ball>& ball,
                        const CollisionInfo& info);
};

#endif
