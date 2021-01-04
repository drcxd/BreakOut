#include "GameObject.h"

#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>

#include "SpriteRenderer.h"

GameObject::
GameObject(const GameObjectAttribute& attribute)
    : attribute(attribute) {

}

GameObject::~GameObject() { }

void GameObject::Update(float dt) {
    glm::vec2 displace = dt * attribute.velocity;
    attribute.position += displace;

    for (auto& child : children) {
        child->attribute.position += displace;
        child->Update(dt);
    }
}

void GameObject::Draw(const SpriteRenderer& renderer) {
    renderer.Draw(attribute.texture,
                  attribute.position,
                  attribute.size,
                  attribute.rotation,
                  attribute.color);
}
