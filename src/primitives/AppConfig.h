/**
 * @file AppConfig.h
 * @brief Singleton-like configuration store backed by a JSONC file.
 *
 * Loads `res/config/app.jsonc` at construction and exposes values via
 * JSON-pointer paths (e.g. `"/airplane/maxSpeed"`).
 */
#pragma once
#include <string>
#include "../lib/json.hpp"

class AppConfig {
    /// @brief Resolve a JSON-pointer path to a const reference into the config tree.
    /// @throws std::runtime_error if the path does not exist.
    [[nodiscard]] const nlohmann::json &getNode(const std::string &path) const;

public:
    nlohmann::json config;   ///< Parsed JSON configuration tree.

    /// @brief Load configuration from `res/config/app.jsonc`.
    AppConfig();

    /// @brief Fetch a typed value by JSON-pointer path.
    /// @tparam T    Desired return type (int, float, std::string_view, …).
    /// @param path  JSON pointer (must start with "/" or be empty for the root).
    /// @return      The value at the given path, converted to @p T.
    template<typename T>
    T get(const std::string &path) const {
        return getNode(path).get<T>();
    }
};