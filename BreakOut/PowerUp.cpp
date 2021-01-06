#include "PowerUp.h"

#include <memory>

PowerUp::PowerUp(const PowerUpAttribute& puAttr)
    : GameObject(std::make_unique<PowerUpAttribute>(puAttr)) { }

PowerUp::~PowerUp() { }

PowerUpAttribute* PowerUp::Attr() {
    return (PowerUpAttribute*)GameObject::attr.get();
}

const PowerUpAttribute* PowerUp::Attr() const {
    return (const PowerUpAttribute*)GameObject::attr.get();
}

void PowerUp::Update(float dt) {
    GameObject::Update(dt);
    if (Attr()->isActive) {
        Attr()->duration -= dt;
    }
    if (Attr()->duration <= 0.0f) {
        Attr()->isActive = false;
        if (Attr()->endCallback) {
            Attr()->endCallback(this);
        }
    }
}
