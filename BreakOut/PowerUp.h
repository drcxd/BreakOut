#ifndef __POWERUP_H__
#define __POWERUP_H__

#include <string>

#include "GameObject.h"

struct PowerUpAttribute : public GameObjectAttribute {
    std::string type;
    float duration = 0.0f;
    bool isActive = false;

    PowerUpAttribute() { }
    PowerUpAttribute(const GameObjectAttribute& objAttr)
        : GameObjectAttribute(objAttr) { }
};

class PowerUp : public GameObject {
public:
    PowerUp(const PowerUpAttribute& puAttr);
    ~PowerUp();

    PowerUpAttribute* Attr() override;
    const PowerUpAttribute* Attr() const override;
};
#endif