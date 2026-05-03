module;
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "../lib/ray.hpp"
export module JsonConfig;

import Resources;

export nlohmann::json parse_json_file(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    TraceLog(LOG_ERROR, TextFormat("[JsonConfig] unable to open file: %s", path.c_str()));
    throw std::runtime_error("[JsonConfig] unable to open file");
  }
  try {
    return nlohmann::json::parse(file, nullptr, true, true);
  } catch (const nlohmann::json::exception &e) {
    TraceLog(LOG_ERROR, TextFormat("[JsonConfig] failed to parse JSON file: %s, reason: %s", path.c_str(), e.what()));
    throw std::runtime_error("Failed to parse JSON file: " + path);
  }
}

export void save_json_to_file(const nlohmann::json &json, const std::string &path) {
  if (std::ofstream file(path); file.is_open()) {
    file << std::setw(4) << json << std::endl;
  } else {
    TraceLog(LOG_ERROR, TextFormat("[JsonConfig] unable to open file: %s", path.c_str()));
    // throw std::runtime_error("[JsonConfig] unable to open file");
  }
}

// export class AppOptions {
//   std::string path;
//   nlohmann::json config;
//   bool changed = false;
//
//  public:
//   float tilt = 0.25f;
//   float fov = 85.0f;
//
//   Vector3 zenith_color{};
//   Vector3 horizon_color{};
//
//   std::string tiles_token;
//   std::string maps_token;
//
//   explicit AppOptions(std::string options_path)
//       : path(std::move(options_path)),
//         config(parse_json_file(path)),
//         // read all options
//         tilt(config["tilt"].get<float>()),
//         fov(config["fov"].get<float>()),
//         zenith_color(config["zenith_color"].get<Vector3>()),
//         horizon_color(config["horizon_color"].get<Vector3>()) {
//     // token for Mapbox
//     if (config.contains("tiles_token")) {
//       tiles_token = config["tiles_token"].get<std::string>();
//     } else {
//       const auto env_tiles_token = std::string(std::getenv(resources::tiles_token_name));
//       if (env_tiles_token.empty()) throw std::runtime_error("missing tiles token in options or environment variables");
//       tiles_token = env_tiles_token;
//     }
//
//     // token for Tomtom
//     if (config.contains("maps_token")) {
//       maps_token = config["maps_token"].get<std::string>();
//     } else {
//       const auto env_maps_token = std::string(std::getenv(resources::maps_token_name));
//       if (env_maps_token.empty()) throw std::runtime_error("missing maps token in options or environment variables");
//       maps_token = env_maps_token;
//     }
//   }
//
//   void save() {
//     config["tilt"] = tilt;
//     config["fov"] = fov;
//     save_json_to_file(config, path);
//   }
// };

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
  std::optional<T> optional(const std::string &path) const {
    if (config.contains(nlohmann::json::json_pointer(path))) return get<T>(path);
    return std::nullopt;
  }

  template <typename T>
  T get_or(const std::string &path, const T &defaultValue) const {
    if (config.contains(nlohmann::json::json_pointer(path))) return get<T>(path);
    return defaultValue;
  }
};