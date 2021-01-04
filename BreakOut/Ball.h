#ifndef __BALL_H__
#define __BALL_H__

#include "GameObject.h"

struct BallAttribute {
    float radius;
    bool isStatic;
};

class Ball : public GameObject {
public:

    Ball(const BallAttribute& ballAttr,
         const GameObjectAttribute& attr);
    ~Ball();

    void Update(float dt) override;

    BallAttribute ballAttribute;
};
#endif
