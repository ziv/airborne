#pragma once
#include "Entity.h"

// Static structures: bridges, depots, radars, bunkers, fuel tanks, runways
struct GroundTarget : EntityBase {
    bool strategicTarget = true;

    void takeDamage(float amount);
};
