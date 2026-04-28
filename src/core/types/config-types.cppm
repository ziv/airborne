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
  float pitchRatio = 2.0f;  ///< Pitch sensitivity (degrees per second at full deflection).
  float rollRatio = 3.0f;   ///< Roll sensitivity.
  float yawRatio = 1.0f;    ///< Yaw (rudder) sensitivity.
};

export struct PlayerPhysicsConfig {
  Newton weight = 120000.0f;          ///< Aircraft empty weight (Newtons).
  Newton engineThrust = 130000.0f;    ///< Maximum engine thrust at full military power (N).
  MeterPerSecond maxSpeed = 600.0f;   ///< Hard speed cap (m/s) — safety limit.
  MeterPerSecond stallSpeed = 65.0f;  ///< Speed below which lift drops sharply.
  MeterPerSecond vleSpeed = 150.0f;   ///< Max gear-extended speed; above this → turbulence.
  Ratio liftCoefficient = 1.93f;      ///< Base lift coefficient (proportional to v²).
  Ratio dragCoefficient = 0.36f;      ///< Base aerodynamic drag coefficient.
  Ratio bankInduceYawRatio = 0.4f;    ///< Adverse yaw factor from bank angle.
  Ratio liftLossPitchRatio = 0.9f;    ///< Nose-down pitch tendency when lift vector tilts.
  Ratio airDumpingFactor = 2.5f;      ///< Additional damping factor for aerodynamic stability.
  Ratio minAuthority = 0.1f;          ///< Minimum control authority at stall speed (0–1).
  Ratio frictionCoefficient = 2.0f;   // todo complete
  Ratio pitchRatio = 4.0f;            ///< Pitch acceleration sensitivity
  Ratio rollRatio = 6.0f;             ///< Roll acceleration sensitivity.
  Ratio yawRatio = 1.0f;              ///< Yaw acceleration sensitivity.
};

export struct PlayerPositionConfig {
  float threshold = 5000.0f;
  std::string heightPath = "assets/images/north-hm.png";
  float maxRelativeHeight = 5000.f;
  float heightMapSizeRatio = 125.0f;  // ratio between the large area and the map we check the height
  float heightAboveGround = 3.0f;     // the pilot is not sitting on "0"
};

// views
// --------------------------

export struct CockpitConfig {
  std::string texturePath;
  std::string shaderPath;
  Color tintColor{};
};

export struct MinimapConfig {
  std::string mapTexture;   ///< Path to the satellite map texture.
  Pixel size = 150;         ///< Size (width and height) of the square minimap widget.
  Ratio mapsRatio = 62.5f;  ///< Ratio between the world coordinates and the map
                            ///< texture coordinates
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
  Meter nearPlane;
  Meter farPlane;
};

export struct WindowConfig {
  std::string title;
  Pixel width;
  Pixel height;
  Meter nearPlane;
  Meter farPlane;
};

export struct PlayerConfig {
  PlayerCameraConfig camera;
  PlayerControlsConfig controls;
  PlayerPhysicsConfig aircraft;
  PlayerPositionConfig position;
};

// todo temporary -> till moving to world streaming
export struct SceneConfig {
  std::string mapTexture;
  std::string mapHeightmap;
  Vector3 mapSize;
  std::string fogShaderVs;
  std::string fogShaderFs;
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
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerControlsConfig, pitchRatio, rollRatio, yawRatio);

  // player - aircraft/physics
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerPhysicsConfig, weight, engineThrust, maxSpeed, stallSpeed, vleSpeed, liftCoefficient, dragCoefficient,
                                     bankInduceYawRatio, liftLossPitchRatio, airDumpingFactor, minAuthority, frictionCoefficient, pitchRatio, rollRatio,
                                     yawRatio);

  // player - position
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerPositionConfig, threshold, heightPath, maxRelativeHeight, heightMapSizeRatio, heightAboveGround);

  // views - cockpit
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CockpitConfig, texturePath, shaderPath, tintColor);

  // views - minimap
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MinimapConfig, mapTexture, size, mapsRatio);

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
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WindowConfig, title, width, height, nearPlane, farPlane);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GlobalConfig, title, resources, width, height, tilt, fov, nearPlane, farPlane);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SceneConfig, mapTexture, mapHeightmap, mapSize, fogShaderVs, fogShaderFs);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ViewsConfig, hud, minimap, cockpit, radar);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ScreensConfig, splash);
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerConfig, camera, controls, aircraft, position);

  // root
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AppConfig, global, player, scene, views, screens);
}
