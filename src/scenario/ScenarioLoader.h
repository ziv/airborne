#pragma once
#include "Scenario.h"
#include "../lib/json.hpp"
#include <string>
#include <vector>

class ScenarioLoader {
    static Vector3 parseVector3(const nlohmann::json& j);
    static Faction parseFaction(const std::string& s);
    static EntityType parseEntityType(const std::string& s);
    static ObjectiveType parseObjectiveType(const std::string& s);
    static StartConditions parseStart(const nlohmann::json& j);
    static WeaponLoadout parseLoadout(const nlohmann::json& j);
    static ScenarioScoring parseScoring(const nlohmann::json& j);
    static Objective parseObjective(const nlohmann::json& j);
    static EntityBase parseEntity(const nlohmann::json& j);
    static Trigger parseTrigger(const nlohmann::json& j);
    static Scenario parseScenario(const nlohmann::json& j);

public:
    static Scenario load(const std::string& path);
    static std::vector<Scenario> loadAll(const std::string& configPath);
    static std::vector<std::string> listAvailable(const std::string& scenariosDir);
};
