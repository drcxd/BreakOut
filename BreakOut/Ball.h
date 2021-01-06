#ifndef __BALL_H__
#define __BALL_H__

#include "GameObject.h"

struct BallAttribute : public GameObjectAttribute {
    float radius = 0.0f;
    bool isStatic = true;
    bool isSticky = false;
    bool isPassThrough = false;

    BallAttribute() { }
    BallAttribute(const GameObjectAttribute& objAttr)
        : GameObjectAttribute(objAttr) { }
    BallAttribute(float radius, bool isStatic)
        : radius(radius), isStatic(isStatic) { }
};

class Ball : public GameObject {
public:

    Ball(const BallAttribute& ballAttr);
    ~Ball();

    void Update(float dt) override;
    BallAttribute* Attr() override;
    const BallAttribute* Attr() const override;
};
#endif
