/**
 * @file GroundTarget.h
 * @brief Destructible ground entity — bridges, depots, radars, bunkers, fuel tanks.
 */
#pragma once
#include "Entity.h"

/// @brief A static structure that can take damage and be destroyed.
struct GroundTarget : EntityBase {
    bool strategicTarget = true;  ///< True if destroying this counts toward mission objectives.

    /// @brief Apply @p amount damage; transitions state to DAMAGED or DESTROYED.
    void takeDamage(float amount);
};
