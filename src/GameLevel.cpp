#include "GameLevel.h"

#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

#include <fmt/core.h>

#include "GameObject.h"
#include "ResourceManager.h"

GameLevel::GameLevel() { }

GameLevel::~GameLevel() {
    bricks.clear();
}

void GameLevel::Load(const char* path,
                     int levelWidth, int levelHeight) {
    std::ifstream fstream;
    ResourceManager::GetInstance()->OpenFile(path, fstream);
    if (!fstream) {
        fmt::print("Failed loading level file {}!\n");
        return;
    }

    std::vector<std::vector<int>> tileData;
    while (fstream) {
        std::string line;
        std::getline(fstream, line);
        if (!fstream) {
            break;
        }

        std::stringstream ss(line);
        int tileCode;
        std::vector<int> row;
        while (ss >> tileCode) {
            row.push_back(tileCode);
        }

        tileData.push_back(row);
    }

    init(tileData, levelWidth, levelHeight);
}

void GameLevel::Draw(const SpriteRenderer& renderer) const {
    for (const auto& brick : bricks) {
        if (!brick->Attr()->isDestroyed) {
            brick->Draw(renderer);
        }
    }
}

bool GameLevel::IsComplete() const {
    for (const auto& brick : bricks) {
        if (!brick->Attr()->isDestroyed &&
            !brick->Attr()->isSolid) {
            return false;
        }
    }
    return true;
}

void GameLevel::init(const std::vector<std::vector<int>>& tileData,
                     int levelWidth, int levelHeight) {

    int row = tileData.size();
    if (!row) {
        return;
    }
    int col = tileData[0].size();
    if (!col) {
        return;
    }

    float brickWidth = (float)levelWidth / (float)col;
    float brickHeight = (float)levelHeight / (float)row;
    bricks.clear();
    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < col; ++j) {
            if (!tileData[i][j]) {
                continue;
            }
            GameObjectAttribute attr;
            attr.size = glm::vec2(brickWidth, brickHeight);
            attr.position = glm::vec2(j * brickWidth, i * brickHeight);
            attr.velocity = glm::vec2(0.0f);
            attr.rotation = 0;
            attr.isDestroyed = false;
            if (tileData[i][j] == 1) {
                attr.color = glm::vec3(0.8f, 0.8f, 0.7f);
                attr.texture = ResourceManager::GetInstance()->
                    GetTexture2D("brick_solid");
                attr.isSolid = true;
            } else if (tileData[i][j] > 1) {
                switch (tileData[i][j]) {
                case 2:
                    attr.color = glm::vec3(0.2f, 0.6f, 1.0f);
                    break;
                case 3:
                    attr.color = glm::vec3(0.0f, 0.7f, 0.0f);
                    break;
                case 4:
                    attr.color = glm::vec3(0.8f, 0.8f, 0.4f);
                    break;
                case 5:
                    attr.color = glm::vec3(1.0f, 0.5f, 0.5f);
                    break;
                default:
                    assert(false);
                    break;
                }
                attr.isSolid = false;
                attr.texture = ResourceManager::GetInstance()->
                    GetTexture2D("brick");
            }
            bricks.emplace_back(std::make_unique<GameObject>(attr));
        }
    }
}

void GameLevel::Reset() {
    for (auto& brick : bricks) {
        brick->Attr()->isDestroyed = false;
    }
}
