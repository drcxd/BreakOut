#include "Ball.h"

Ball::Ball(const BallAttribute& ballAttr)
    : GameObject(std::make_unique<BallAttribute>(ballAttr)) { }

Ball::~Ball() { }

void Ball::Update(float dt) {
    if (!Attr()->isStatic) {
        GameObject::Update(dt);
    }
}

BallAttribute* Ball::Attr() {
    return (BallAttribute*)GameObject::attr.get();
}

const BallAttribute* Ball::Attr() const {
    return (const BallAttribute*)GameObject::attr.get();
}
