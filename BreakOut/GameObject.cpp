#include "GameObject.h"

#include "SpriteRenderer.h"

GameObject::
GameObject(const GameObjectAttribute& attribute)
    : attribute(attribute) {

}

GameObject::~GameObject() { }

void GameObject::Update(float dt) {
}

void GameObject::Draw(const SpriteRenderer& renderer) {
    renderer.Draw(attribute.texture,
                  attribute.position,
                  attribute.size,
                  attribute.rotation,
                  attribute.color);
}
