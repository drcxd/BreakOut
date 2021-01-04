#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <memory>

#include <glm/gtc/type_ptr.hpp>

class Texture2D;
class SpriteRenderer;

struct GameObjectAttribute {
    glm::vec2 size;
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec3 color;
    float rotation;
    bool isSolid;
    bool isDestroyed;
    std::shared_ptr<Texture2D> texture;
};

class GameObject {
public:

    GameObject(const GameObjectAttribute& attribute);
    virtual ~GameObject();

    void Update(float dt);
    void Draw(const SpriteRenderer& renderer);

    GameObjectAttribute attribute;
};
#endif
