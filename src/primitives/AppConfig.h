#pragma once
#include <string>
#include "../lib/json.hpp"

class AppConfig {
    [[nodiscard]] const nlohmann::json &getNode(const std::string &path) const;

public:
    nlohmann::json config;

    AppConfig();

    /// @brief Fetch key by type
    /// @brief Key must start with "/" or an empty string for the whole tree
    template<typename T>
    T get(const std::string &path) const {
        return getNode(path).get<T>();
    }
};