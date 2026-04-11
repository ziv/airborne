/**
 * @file Scenario.h
 * @brief Data structures defining a complete mission scenario — start conditions,
 *        weapon loadout, objectives, entity placements, triggers, and scoring.
 */
#pragma once
#include "Objective.h"
#include "Trigger.h"
#include "../entities/Entity.h"
#include "../primitives/Types.h"
#include <vector>
#include <string>

/// @brief Available weapons and the player's current selection.
struct WeaponLoadout {
    int slots = 0;                              ///< Number of weapon hardpoints.
    std::vector<std::string> available;         ///< Pool of selectable weapon types.
    std::vector<std::string> defaults;    ///< Factory-default weapon assignment.
    // std::vector<std::string> selectedLoadout;   ///< Player's current selection.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WeaponLoadout, slots, available);

/// @brief Initial aircraft position, heading, speed, altitude, fuel, and launch mode.
struct StartConditions {
    Vector3 position = {0, 0, 0};
    float heading = 0.0f;
    MeterPerSecond speed = 0;
    Meter altitude = 100;
    float fuel = 3500;       ///< Starting fuel in kilograms.
    bool carrier = false;  ///< True for a carrier catapult launch.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(StartConditions, position, heading, speed, altitude, fuel, carrier);

/// @brief Point values and bonus flags for the post-mission debrief.
struct ScenarioScoring {
    int objectiveComplete = 1000;
    int bonusObjective = 500;
    int enemyAircraftKill = 250;
    int samDestroyed = 200;
    int aaaDestroyed = 100;
    bool timeBonus = true;
    bool fuelBonus = true;
    int noDamageBonus = 500;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScenarioScoring, objectiveComplete, bonusObjective, enemyAircraftKill, samDestroyed, aaaDestroyed, timeBonus, fuelBonus, noDamageBonus);

/// @brief Complete mission definition loaded from a JSONC scenario file.
struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    std::string difficulty;
    std::string theater;                          ///< Geographic region / map name.

    StartConditions start;
    WeaponLoadout loadout;

    std::vector<Objective> objectives;            ///< Mission objectives (required + optional).
    std::vector<EntityBase> entityDefinitions;    ///< Entities to spawn at mission start.
    std::vector<Trigger> triggers;                ///< Event-driven scripting triggers.
    ScenarioScoring scoring;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Scenario, id, name, description, difficulty, theater, start, loadout, objectives, entityDefinitions, triggers, scoring);
