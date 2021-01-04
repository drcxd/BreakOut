#ifndef __GAME_H__
#define __GAME_H__

#include <vector>
#include <memory>

class GameLevel;

enum class GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
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
    bool quit;


private:

    std::vector<std::shared_ptr<GameLevel>> levels;
    void loadResources();
};

#endif
