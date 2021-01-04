#include "Ball.h"

Ball::Ball(const BallAttribute& ballAttr,
           const GameObjectAttribute& attr)
    : GameObject(attr)
    , ballAttribute(ballAttr) {

}

Ball::~Ball() {

}

void Ball::Update(float dt) {
    if (!ballAttribute.isStatic) {
        GameObject::Update(dt);
    }
}
