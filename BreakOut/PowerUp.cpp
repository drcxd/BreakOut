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
