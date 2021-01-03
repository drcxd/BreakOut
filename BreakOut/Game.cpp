#include "Game.h"

Game::Game(int width, int height)
    : width(width)
    , height(height)
    , state(GameState::GAME_MENU)
    , keys{0} { }

Game::~Game() { }

void Game::Init() {

}

void Game::ProcessInput(float dt) {

}

void Game::Update(float dt) {

}

void Game::Render() {

}
