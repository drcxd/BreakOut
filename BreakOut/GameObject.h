#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__

#include <memory>
#include <unordered_set>

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

    GameObject(const GameObjectAttribute& objAttr);
    GameObject(std::unique_ptr<GameObjectAttribute>&& ptr)
        : attr(std::move(ptr)) { }
    virtual ~GameObject();

    virtual void Update(float dt);
    virtual void Draw(const SpriteRenderer& renderer);

    virtual GameObjectAttribute* Attr();
    virtual const GameObjectAttribute* Attr() const;

    std::unordered_set<std::shared_ptr<GameObject>> children;

protected:

    std::unique_ptr<GameObjectAttribute> attr;
};
#endif
