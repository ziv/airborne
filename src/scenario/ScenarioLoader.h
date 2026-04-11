/**
 * @file ScenarioLoader.h
 * @brief JSON parser that deserialises scenario JSONC files into Scenario structs.
 *
 * Provides static methods to load a single scenario, load all scenarios from a
 * master config file, or list available .jsonc files in a directory.
 */
#pragma once
#include "Scenario.h"
#include <string>
#include <vector>

/**
 * @brief Static utility class for parsing scenario JSON files.
 *
 * All parsing methods are private; the public API is load(), loadAll(), and
 * listAvailable().
 */
class ScenarioLoader {
public:
    /// @brief Load a single scenario from a JSONC file.
    static Scenario load(const std::string& path);

    /// @brief Load all scenarios from a master config file containing a "scenarios" array.
    static std::vector<Scenario> loadAll(const std::string& configPath);

    /// @brief List all .jsonc scenario files in a directory, sorted alphabetically.
    static std::vector<std::string> listAvailable(const std::string& scenariosDir);
};
