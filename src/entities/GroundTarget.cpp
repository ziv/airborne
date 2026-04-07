#include "GroundTarget.h"

void GroundTarget::takeDamage(float amount) {
    if (!isAlive()) return;

    health -= amount;
    if (health <= 0.0f) {
        health = 0.0f;
        state = EntityState::DESTROYED;
    } else if (health < maxHealth * 0.5f) {
        state = EntityState::DAMAGED;
    }
}
