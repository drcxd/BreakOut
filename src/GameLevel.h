#ifndef __GAMELEVEL_H__
#define __GAMELEVEL_H__

#include <vector>
#include <memory>

class GameObject;
class SpriteRenderer;

class GameLevel {
public:
    GameLevel();
    ~GameLevel();
    void Load(const char* path, int levelWidth, int levelHeight);
    void Draw(const SpriteRenderer& renderer) const;
    bool IsComplete() const;
    void Reset();

    std::vector<std::unique_ptr<GameObject>> bricks;

private:
    void init(const std::vector<std::vector<int>>& tileData,
              int levelWidth, int levelHeight);
    void clearBricks();
};
#endif
