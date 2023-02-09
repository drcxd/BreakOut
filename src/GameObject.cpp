#include "GameObject.h"

#include <glm/gtc/type_ptr.hpp>

#include "SpriteRenderer.h"

GameObject::
GameObject(const GameObjectAttribute& objAttr)
    : attr(std::make_unique<GameObjectAttribute>(objAttr)) { }

GameObject::~GameObject() { }

void GameObject::Update(float dt) {
    glm::vec2 displace = dt * Attr()->velocity;
    Attr()->position += displace;

    for (auto& child : children) {
        child->Attr()->position += displace;
        child->Update(dt);
    }
}

void GameObject::Draw(const SpriteRenderer& renderer) {
    renderer.Draw(Attr()->texture,
                  Attr()->position,
                  Attr()->size,
                  Attr()->rotation,
                  Attr()->color);
}

GameObjectAttribute*
GameObject::Attr() {
    return attr.get();
}

const GameObjectAttribute*
GameObject::Attr() const {
    return attr.get();
}
