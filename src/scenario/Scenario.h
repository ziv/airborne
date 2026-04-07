#pragma once
#include "Objective.h"
#include "Trigger.h"
#include "../entities/Entity.h"
#include "../primitives/Types.h"
#include <vector>
#include <string>

struct WeaponLoadout {
    int slots = 0;
    std::vector<std::string> available;
    std::vector<std::string> defaultLoadout;
    std::vector<std::string> selectedLoadout;
};

struct StartConditions {
    Vector3 position = {0, 0, 0};
    float heading = 0.0f;
    MeterPerSecond speed = 0;
    Meter altitude = 100;
    float fuel = 3500;
    bool onCarrier = false;
};

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

struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    std::string difficulty;
    std::string theater;

    StartConditions start;
    WeaponLoadout loadout;

    std::vector<Objective> objectives;
    std::vector<EntityBase> entityDefinitions;
    std::vector<Trigger> triggers;
    ScenarioScoring scoring;
};
