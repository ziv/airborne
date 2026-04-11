/**
 * @file ScenarioLoader.h
 * @brief JSON parser that deserialises scenario JSONC files into Scenario structs.
 *
 * Provides static methods to load a single scenario, load all scenarios from a
 * master config file, or list available .jsonc files in a directory.
 */
#pragma once
#include "Scenario.h"
#include "../lib/json.h"
#include <string>
#include <vector>

/**
 * @brief Static utility class for parsing scenario JSON files.
 *
 * All parsing methods are private; the public API is load(), loadAll(), and
 * listAvailable().
 */
class ScenarioLoader {
    // static Vector3 parseVector3(const nlohmann::json& j);
    // static Faction parseFaction(const std::string& s);
    // static EntityType parseEntityType(const std::string& s);
    // static ObjectiveType parseObjectiveType(const std::string& s);
    // static StartConditions parseStart(const nlohmann::json& j);
    // static WeaponLoadout parseLoadout(const nlohmann::json& j);
    // static ScenarioScoring parseScoring(const nlohmann::json& j);
    // static Objective parseObjective(const nlohmann::json& j);
    // static EntityBase parseEntity(const nlohmann::json& j);
    // static Trigger parseTrigger(const nlohmann::json& j);
    // static Scenario parseScenario(const nlohmann::json& j);

public:
    /// @brief Load a single scenario from a JSONC file.
    static Scenario load(const std::string& path);

    /// @brief Load all scenarios from a master config file containing a "scenarios" array.
    static std::vector<Scenario> loadAll(const std::string& configPath);

    /// @brief List all .jsonc scenario files in a directory, sorted alphabetically.
    static std::vector<std::string> listAvailable(const std::string& scenariosDir);
};
