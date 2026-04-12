/**
 * @file Scenario.h
 * @brief Data structures defining a complete mission scenario — start conditions,
 *        weapon loadout, objectives, entity placements, triggers, and scoring.
 */
#pragma once
#include "Objective.h"
#include "../entities/Entity.h"
#include "../primitives/Types.h"
#include <vector>
#include <string>

/// @brief Available weapons and the player's current selection.
// struct WeaponLoadout {
//     int slots = 0; ///< Number of weapon hardpoints.
//     std::vector<std::string> available; ///< Pool of selectable weapon types.
//     std::vector<std::string> defaults; ///< Factory-default weapon assignment.
//     // std::vector<std::string> selectedLoadout;   ///< Player's current selection.
// };
//
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WeaponLoadout, slots, available);

/// @brief Initial aircraft position, heading, speed, altitude, fuel, and launch mode.
struct StartConditions {
    Vector3 position = {0.0f, 0.0f, 0.0f};
    float heading = 0.0f;
    MeterPerSecond speed = 0.0f;
    Meter altitude = 0.0f;
    float fuel = 3500.0f; ///< Starting fuel in kilograms.
    bool carrier = false; ///< True for a carrier catapult launch.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartConditions, position, heading, speed, altitude, fuel, carrier);

/// @brief Point values and bonus flags for the post-mission debrief.
// struct ScenarioScoring {
//     int objectiveComplete = 1000;
//     int bonusObjective = 500;
//     int enemyAircraftKill = 250;
//     int samDestroyed = 200;
//     int aaaDestroyed = 100;
//     bool timeBonus = true;
//     bool fuelBonus = true;
//     int noDamageBonus = 500;
// };
//
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScenarioScoring, objectiveComplete, bonusObjective, enemyAircraftKill, samDestroyed, aaaDestroyed, timeBonus, fuelBonus,
//                                    noDamageBonus);

/// @brief Complete mission definition loaded from a JSONC scenario file.
struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    std::string difficulty;
    std::string theater; ///< Geographic region / map name.

    StartConditions start;
    // WeaponLoadout loadout;

    std::vector<EntityDef> entities; ///< Entities to spawn at mission start.
    // std::vector<Objective> objectives; ///< Mission objectives (required + optional).
    // std::vector<Trigger> triggers; ///< Event-driven scripting triggers.
    // ScenarioScoring scoring;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Scenario, id, name, description, difficulty, theater, start, entities);
