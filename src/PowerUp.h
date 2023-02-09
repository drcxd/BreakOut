#ifndef __POWERUP_H__
#define __POWERUP_H__

#include <string>
#include <functional>

#include "GameObject.h"

class PowerUp;

struct PowerUpAttribute : public GameObjectAttribute {
    std::string type;
    float duration = 0.0f;
    bool isActive = false;
    std::function<void(const PowerUp*)> endCallback;

    PowerUpAttribute() { }
    PowerUpAttribute(const GameObjectAttribute& objAttr)
        : GameObjectAttribute(objAttr) { }
};

class PowerUp : public GameObject {
public:
    PowerUp(const PowerUpAttribute& puAttr);
    ~PowerUp();
    void Update(float dt) override;

    PowerUpAttribute* Attr() override;
    const PowerUpAttribute* Attr() const override;
};

// class SpeedPowerUp : public PowerUp {
// public:
//     SpeedPowerUp();

// private:
//     static const PowerUpAttribute ATTR;
// };
#endif
