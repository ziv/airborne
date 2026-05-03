module;
#include <nlohmann/json.hpp>
#include <string>

#include "../../lib/ray.hpp"

export module Types:Config;

import :Units;  // namespace config

// player/aircraft
// --------------------------

export struct PlayerCameraConfig {
  float tilt = 0.45f;  ///< Downward pitch offset (radians) applied to the pilot's view.
  float fov = 85.0f;
};

export struct PlayerControlsConfig {
  float pitch_ratio = 2.0f;  ///< Pitch sensitivity (degrees per second at full deflection).
  float roll_ratio = 3.0f;   ///< Roll sensitivity.
  float yaw_ratio = 1.0f;    ///< Yaw (rudder) sensitivity.
};

export struct PlayerPhysicsConfig {
  Newton weight = 120000.0f;          ///< Aircraft empty weight (Newtons).
  Newton engine_thrust = 130000.0f;    ///< Maximum engine thrust at full military power (N).
  MeterPerSecond max_speed = 600.0f;   ///< Hard speed cap (m/s) — safety limit.
  MeterPerSecond stall_speed = 65.0f;  ///< Speed below which lift drops sharply.
  MeterPerSecond vle_speed = 150.0f;   ///< Max gear-extended speed; above this → turbulence.
  Ratio lift_coefficient = 1.93f;      ///< Base lift coefficient (proportional to v²).
  Ratio drag_coefficient = 0.36f;      ///< Base aerodynamic drag coefficient.
  Ratio bank_induce_yaw_ratio = 0.4f;    ///< Adverse yaw factor from bank angle.
  Ratio lift_loss_pitch_ratio = 0.9f;    ///< Nose-down pitch tendency when lift vector tilts.
  Ratio air_damping_factor = 2.5f;      ///< Additional damping factor for aerodynamic stability.
  Ratio min_authority = 0.1f;          ///< Minimum control authority at stall speed (0–1).
  Ratio friction_coefficient = 2.0f;   // todo complete
  Ratio pitch_ratio = 4.0f;            ///< Pitch acceleration sensitivity
  Ratio roll_ratio = 6.0f;             ///< Roll acceleration sensitivity.
  Ratio yaw_ratio = 1.0f;              ///< Yaw acceleration sensitivity.
};

export struct PlayerPositionConfig {
  float threshold = 5000.0f;
  std::string height_path = "assets/images/north-hm.png";
  float max_relative_height = 5000.f;
  float height_map_size_ratio = 125.0f;  // ratio between the large area and the map we check the height
  float height_above_ground = 3.0f;     // the pilot is not sitting on "0"
};

// views
// --------------------------

export struct CockpitConfig {
  std::string texture_path;
  std::string shader_path;
  Color tint_color{};
};

export struct MinimapConfig {
  std::string map_texture;   ///< Legacy: path to the fixed satellite map texture (unused by map streamer).
  Pixel size = 150;         ///< Size (width and height) of the square minimap widget.
  Ratio maps_ratio = 62.5f;  ///< Legacy: ratio between world coordinates and the fixed map texture.
  int default_map_zoom = 14;  ///< Initial slippy-map zoom level (1–20).
};

// HUD config from here

export struct HudLadderConfig {
  Pixel x = 440;
  Pixel y = 200;
  Pixel width = 320;
  Pixel height = 220;
  Pixel offset = 0;
};

export struct RocConfig {
  Pixel x;
  Pixel y;
  Pixel width;
  Pixel height;
};

export struct SpeedometerConfig {
  Pixel x;
  Pixel y;
  Size font;
};

export struct HeightIndicatorConfig {
  Pixel x;
  Pixel y;
  Size font;
};

export struct BoresightConfig {
  Pixel x;
  Pixel y;
  Size size;
};

export struct HeadingConfig {
  Pixel x;
  Pixel y;
  Pixel width;
  Size font;
};

export struct WarningsConfig {
  Pixel x;
  Pixel y;
  Size font;
};

export struct HudConfig {
  HudLadderConfig ladder;
  RocConfig roc;
  SpeedometerConfig speedometer;
  HeightIndicatorConfig height;
  BoresightConfig boresight;
  HeadingConfig heading;
  WarningsConfig warnings;
};

export struct RadarConfig {
  Pixel size;
  std::array<Meter, 3> ranges = {1000.0f, 5000.0f, 25000.0f};
};

// screens

export struct SplashScreenConfig {
  std::string bg_tex_path = "res/images/splash.png";
  std::string bg_sound_path = "res/audio/splash-1.mp3";
};

// categories
// -------------------------------------------------
export struct GlobalConfig {
  // window
  std::string title;
  std::string resources;

  Pixel width;
  Pixel height;

  // player tilt down and field of view
  AngleRad tilt;
  AngleDeg fov;

  // where to clip planes
  Meter near_plane;
  Meter far_plane;
};

export struct WindowConfig {
  std::string title;
  Pixel width;
  Pixel height;
  Meter near_plane;
  Meter far_plane;
};

export struct PlayerConfig {
  PlayerCameraConfig camera;
  PlayerControlsConfig controls;
  PlayerPhysicsConfig aircraft;
  PlayerPositionConfig position;
};

// todo temporary -> till moving to world streaming
export struct SceneConfig {
  std::string map_texture;
  std::string map_heightmap;
  Vector3 map_size;
  std::string fog_shader_vs;
  std::string fog_shader_fs;
};

export struct ViewsConfig {
  HudConfig hud;
  MinimapConfig minimap;
  CockpitConfig cockpit;
  RadarConfig radar;
};

export struct ScreensConfig {
  SplashScreenConfig splash;
};

// full tree
// -------------------------------------------------

export struct AppConfig {
  GlobalConfig global;
  PlayerConfig player;
  SceneConfig scene;
  ViewsConfig views;
  ScreensConfig screens;
};

export {
  // player - camera
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerCameraConfig, tilt, fov);

  // player - controls
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerControlsConfig, pitch_ratio, roll_ratio, yaw_ratio);

  // player - aircraft/physics
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerPhysicsConfig, weight, engine_thrust, max_speed, stall_speed, vle_speed, lift_coefficient, drag_coefficient,
                                     bank_induce_yaw_ratio, lift_loss_pitch_ratio, air_damping_factor, min_authority, friction_coefficient, pitch_ratio, roll_ratio,
                                     yaw_ratio);

  // player - position
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerPositionConfig, threshold, height_path, max_relative_height, height_map_size_ratio, height_above_ground);

  // views - cockpit
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CockpitConfig, texture_path, shader_path, tint_color);

  // views - minimap
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MinimapConfig, map_texture, size, maps_ratio, default_map_zoom);

  // views - hud
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HudLadderConfig, x, y, width, height, offset);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RocConfig, x, y, width, height);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SpeedometerConfig, x, y, font);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HeightIndicatorConfig, x, y, font);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BoresightConfig, x, y, size);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HeadingConfig, x, y, width, font);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarningsConfig, x, y, font);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HudConfig, ladder, roc, speedometer, height, boresight, heading, warnings);

  // views - radar
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RadarConfig, size, ranges);

  // screens
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SplashScreenConfig, bg_tex_path, bg_sound_path);

  // categories
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WindowConfig, title, width, height, near_plane, far_plane);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GlobalConfig, title, resources, width, height, tilt, fov, near_plane, far_plane);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneConfig, map_texture, map_heightmap, map_size, fog_shader_vs, fog_shader_fs);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewsConfig, hud, minimap, cockpit, radar);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScreensConfig, splash);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerConfig, camera, controls, aircraft, position);

  // root
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AppConfig, global, player, scene, views, screens);
}
