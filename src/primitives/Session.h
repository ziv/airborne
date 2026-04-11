#pragma once
#include "../lib/json.hpp"
#include <filesystem>

using json = nlohmann::json;

class Session {
    std::filesystem::path path = "app_session.json";
    json data;

    void error(const std::string &msg);

public:
    json &get() { return data; }

    template<typename T>
    T getValue(const std::string &key, T defaultValue) {
        if (data.contains(key)) {
            return data[key].get<T>();
        }
        return defaultValue;
    }

    void read();

    void write();
};
