module;
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "lib/ray.hpp"
export module AppOptions;

import Resources;

nlohmann::json parse_json_file(const std::string &path) {
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

void save_json_to_file(const nlohmann::json &json, const std::string &path) {
  if (std::ofstream file(path); file.is_open()) {
    file << std::setw(4) << json << std::endl;
  } else {
    TraceLog(LOG_ERROR, TextFormat("[JsonConfig] unable to open file: %s", path.c_str()));
    // throw std::runtime_error("[JsonConfig] unable to open file");
  }
}

export class AppOptions {
  std::string path;
  nlohmann::json config;
  bool changed = false;

  float tilt = 0.25f;
  float fov = 85.0f;

  Vector3 zenith_color{};
  Vector3 horizon_color{};

  // do not need setters, not available to
  // change during runtime
  std::string tiles_token;
  std::string maps_token;

 public:
  explicit AppOptions(std::string options_path)
      : path(std::move(options_path)),
        config(parse_json_file(path)),
        // read all options
        tilt(config["tilt"].get<float>()),
        fov(config["fov"].get<float>()),
        zenith_color(config["zenith_color"].get<Vector3>()),
        horizon_color(config["horizon_color"].get<Vector3>()) {
    // token for Mapbox
    if (config.contains("tiles_token")) {
      tiles_token = config["tiles_token"].get<std::string>();
    } else {
      const auto env_tiles_token = std::string(std::getenv(resources::tiles_token_name));
      if (env_tiles_token.empty()) throw std::runtime_error("missing tiles token in options or environment variables");
      tiles_token = env_tiles_token;
    }

    // token for Tomtom
    if (config.contains("maps_token")) {
      maps_token = config["maps_token"].get<std::string>();
    } else {
      const auto env_maps_token = std::string(std::getenv(resources::maps_token_name));
      if (env_maps_token.empty()) throw std::runtime_error("missing maps token in options or environment variables");
      maps_token = env_maps_token;
    }
  }

  void save() {
    config["tilt"] = tilt;
    config["fov"] = fov;
    config["zenith_color"] = zenith_color;
    config["horizon_color"] = horizon_color;
    save_json_to_file(config, path);
  }

  [[nodiscard]] float get_tilt() const { return tilt; }
  [[nodiscard]] float get_fov() const { return fov; }

  Vector3 &get_zenith_color() { return zenith_color; }
  Vector3 &get_horizon_color() { return horizon_color; }

  std::string &get_tiles_token() { return tiles_token; }
  std::string &get_maps_token() { return maps_token; }

  void set_tilt(const float new_tilt) { tilt = new_tilt; }
  void set_fov(const float new_fov) { fov = new_fov; }

  void set_zenith_color(const Vector3 &new_color) { zenith_color = new_color; }
  void set_horizon_color(const Vector3 &new_color) { horizon_color = new_color; }
};
