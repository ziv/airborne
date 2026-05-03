module;
#include <entt/entt.hpp>
#include <string>
#include "../lib/ray.hpp"

export module Components;
export import :Widgets;
export import :Player;
export import :Global;

import RaylibResource;
import Types;
import ResourceManager;
import JsonConfig;

// widgets components

export enum class RadarMode { AIR_TO_AIR, AIR_TO_GROUND };

export struct RadarState {
  RadarMode mode = RadarMode::AIR_TO_AIR;
  entt::entity locked_target = entt::null;
  float max_range = 20000.0f;
};

export struct EngineState {
  entt::entity engine_stream;
};

export struct NpcTag {};

export struct WithModel {
  entt::hashed_string model;
};

export struct WithTexture {
  entt::hashed_string texture;
  // entt::resource<TextureResourceLoader> handle;
};

export struct WithFsShader {
  entt::hashed_string shader;
  // entt::resource<ShaderLoader> handle;
};

// area we can land on
export struct Landable {
  bool carrier;
};

// tag aircraft as crashed
export struct Crashed {};

// events/states

export struct Grounded {};

export struct Flying {};

export struct TouchDown {};

export struct Autopilot {};

export struct LandingZoneDef {
  bool isLandingZone;
  bool isCarrier;
  float surfaceY;
};

// entities

export struct Identify {
  std::string id;
  std::string name;
};

export struct IdentifyType {
  EntityType type{};
};

export struct Identity {
  std::string name;
  std::string origin;
  std::string type;
};

export struct Heading {
  float heading;
};

export struct Health {
  float max;
  float current;
};

export struct PhysicsProfile {
  float mass;
  float thrust;
  float drag_coefficient;
  float lift_coefficient;
  float max_speed;
  float max_g;
  float turn_rate;
};

export struct Radar {
  float detection_range_m;
  float engagement_range_m;
};

export struct WeaponLauncher {
  std::string weapon_prefab_id;
  int fire_rate_rpm;
  int capacity;
};

export struct Weapons {
  std::map<std::string, int> weapons;
};

export struct LandingZone {
  float runway_length_m;
  float runway_width_m;
  float elevation_m;
};

export struct Waypoints {
  std::vector<Vector3> waypoints;
};

// position

export struct Position3D {
  Vector3 pos;
};

export struct Position2D {
  Vector2 pos;
};

// velocity

export struct Velocity3D {
  Vector3 velocity;
};

// units

export struct FriendFoe {
  Faction faction{};
};

export struct Carrier {};

export struct Forces {
  float thrust;
  float drag;
  float lift;
  float mass;
  Vector3 acceleration;
};

export enum class AiState { PATROL, ENGAGE, DISENGAGE };

export struct AiController {
  AiState state = AiState::PATROL;
  int waypoint_index = 0;
  entt::entity target = entt::null;
  float fire_cooldown = 0.0f;
  float disengage_timer = 0.0f;
  // Desired world-space direction set by autopilot; physics rotates toward it.
  Vector3 desired_dir = {0.0f, 0.0f, 1.0f};
  float desired_speed = 0.0f;
};

// NPC aircraft orientation (mirrors Player's forward/up/right/rotation fields).
export struct AircraftOrientation {
  Quaternion rotation = QuaternionIdentity();
  Vector3 forward = {0.0f, 0.0f, 1.0f};
  Vector3 up = {0.0f, 1.0f, 0.0f};
  Vector3 right = {-1.0f, 0.0f, 0.0f};
};
