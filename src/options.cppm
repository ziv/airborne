module;
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "lib/ray.hpp"
export module AppOptions;

import Resources;
import JsonConfig;

static std::string required_env(const char *name, std::string_view label) {
  if (const char *value = std::getenv(name); value && *value) {
    return value;
  }
  throw std::runtime_error(std::format("missing {} token in options or environment variables", label));
}

export class AppOptions {
  std::string path;
  nlohmann::json config;
  bool changed = false;

  float tilt = 0.25f;
  float fov = 85.0f;

  Vector3 zenith_color{};
  Vector3 horizon_color{};
  Vector3 ambient_color{};

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
        horizon_color(config["horizon_color"].get<Vector3>()),
        ambient_color(config["ambient_color"].get<Vector3>()) {
    // token for Mapbox
    if (config.contains("tiles_token")) {
      tiles_token = config["tiles_token"].get<std::string>();
    } else {
      tiles_token = required_env(resources::tiles_token_name, "tiles");
    }

    // token for Tomtom
    if (config.contains("maps_token")) {
      maps_token = config["maps_token"].get<std::string>();
    } else {
      maps_token = required_env(resources::maps_token_name, "maps");
    }
  }

  void save() {
    config["tilt"] = tilt;
    config["fov"] = fov;
    config["zenith_color"] = zenith_color;
    config["horizon_color"] = horizon_color;
    config["ambient_color"] = ambient_color;
    save_json_to_file(config, path);
  }

  [[nodiscard]] float get_tilt() const { return tilt; }
  [[nodiscard]] float get_fov() const { return fov; }

  [[nodiscard]] Vector3 &get_zenith_color() { return zenith_color; }
  [[nodiscard]] Vector3 &get_horizon_color() { return horizon_color; }
  [[nodiscard]] Vector3 &get_ambient_color() { return ambient_color; }

  std::string &get_tiles_token() { return tiles_token; }
  std::string &get_maps_token() { return maps_token; }

  void set_tilt(const float new_tilt) { tilt = new_tilt; }
  void set_fov(const float new_fov) { fov = new_fov; }

  void set_zenith_color(const Vector3 &new_color) { zenith_color = new_color; }
  void set_horizon_color(const Vector3 &new_color) { horizon_color = new_color; }
  void set_ambient_color(const Vector3 &new_color) { ambient_color = new_color; }
};
