module;
#include <raylib.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

export module JsonConfig;

export class JsonConfig {
 public:
  nlohmann::json config;

  explicit JsonConfig(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      TraceLog(LOG_ERROR, TextFormat("[JsonConfig] unable to open file: %s", path.c_str()));
      throw std::runtime_error("[JsonConfig] unable to open file");
    }
    try {
      config = nlohmann::json::parse(file, nullptr, true, true);
    } catch (const nlohmann::json::exception &e) {
      TraceLog(LOG_ERROR, TextFormat("[JsonConfig] failed to parse JSON file: %s, reason: %s", path.c_str(), e.what()));
      throw std::runtime_error("Failed to parse JSON file: " + path);
    }

    TraceLog(LOG_DEBUG, TextFormat("[JsonConfig] file %s loaded", path.c_str()));
  }

  [[nodiscard]] const nlohmann::json &node(const std::string &path) const {
    try {
      return config.at(nlohmann::json::json_pointer(path));
    } catch (const nlohmann::json::exception &e) {
      TraceLog(LOG_ERROR, TextFormat("Config error, missing or invalid path: %s, reason: %s", path.c_str(), e.what()));
      throw std::runtime_error("Config path not found: " + path);
    }
  }

  template <typename T>
  T get(const std::string &path) const {
    try {
      return node(path).get<T>();
    } catch (const nlohmann::json::type_error &e) {
      TraceLog(LOG_ERROR, TextFormat("Config error, type mismatch at path: %s, reason: %s", path.c_str(), e.what()));
      throw std::runtime_error("Config type mismatch at path: " + path);
    }
  }

  template <typename T>
  std::optional<T> optional(const std::string &path) const noexcept {
    if (config.contains(nlohmann::json::json_pointer(path))) return get<T>(path);
    return std::nullopt;
  }

  template <typename T>
  T get_or(const std::string &path, const T &defaultValue) const noexcept {
    if (config.contains(nlohmann::json::json_pointer(path))) return get<T>(path);
    return defaultValue;
  }
};