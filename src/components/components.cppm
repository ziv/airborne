module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Components;

import RaylibResource;
import Types;
import ResourceManager;
import JsonConfig;

export struct Configuration {
  AppConfig conf{};
  // JsonConfig scenario;
};

// global game state
// todo check if in use
export enum class GameStatus { INITIALIZE, LOADING, PLAYING, PAUSED, GAME_OVER };

export struct GameState {
  GameStatus status = GameStatus::INITIALIZE;
};

// global tag of the player
export struct PlayerEntity {
  entt::entity id;
};

// the player state
export struct Player {
  Quaternion rotation;
  Vector3 pos;
  Vector3 offset;
  Vector3 velocity;
  Vector3 angular_velocity;  // angular velocity
  Vector3 forward;
  Vector3 up;
  Vector3 right;
  float speed;
  float ground_height;
  float effective_ground_height;

  [[nodiscard]] Vector3 absolute_position() const { return pos - offset; }
};

// player inputs state
export struct PlayerInputs {
  float pitch = 0.0f;
  float yaw = 0.0f;
  float roll = 0.0f;
  float throttle = 0.0f;
  bool gear = true;
  bool brakes = true;
  bool autopilot = false;
};

// player ground state
export struct GroundHeight {
  float height;
  float effectiveGroundHeight;
};

// global tiles

export enum class RadarMode { AIR_TO_AIR, AIR_TO_GROUND };

export struct RadarState {
  RadarMode mode = RadarMode::AIR_TO_AIR;
  entt::entity locked_target = entt::null;
  float max_range = 20000.0f;
};

export struct EngineState {
  entt::entity engine_stream;
};

// ---------------- todo need to make a hell of an order in this mess...

export struct LoadingContext {
  JsonConfig items_to_load;
  size_t current_index = 0;
  size_t total_items = 0;
  bool is_finished = false;
};

export struct WithModel {
  entt::hashed_string model;
  // entt::resource<ModelResourceLoader> handle;
};

export struct WithTexture {
  entt::resource<TextureResourceLoader> handle;
};

export struct WithImage {
  entt::resource<ImageResourceLoader> handle;
};

export struct WithFsShader {
  entt::resource<ShaderLoader> handle;
};

// screen slots

export struct DashboardSlot {
  int slot_index;
};

// widgets tags

export struct CockpitWidget {};

export struct HudWidget {
  HudConfig cfg{};
  Pixel centerX;
  Pixel centerY;
  Pixel pixelsPerDegree;
  float ppd;
  float tilt;
  int colorIndex;
};

export struct RadarWidget {
  RadarConfig cfg{};
  int rangeCount = 0;
  int rangeIndex = 0;
};

export struct MinimapWidget {
  MinimapConfig cfg{};
  int map_zoom = 14;     // slippy-map tile zoom level (1–20); X = zoom in, Z = zoom out
};

export struct EngineWidget {};

export struct CameraWidget {};
export struct TargetCameraWidget {};

// area we can land on
export struct Landable {
  bool carrier;
};

// tag aircraft as crashed
export struct Crashed {};

export struct ChromaRender {
  TextureHandle tex;
  ShaderHandle shader;
  Color tint = WHITE;
  int priority = 0;
};

export struct Aircraft {
  // drag
  float cd;
  float inducedDragCoefficient;

  // lift
  float cl;
  float liftSlopeCoefficient;
  float stallAngle;

  // weight
  float weight;

  // agility
  float pitchRatio;
  float rollRatio;
  float yawRatio;
};

export struct AircraftUtils {
  bool brake;  // aircraft brakes status
  bool gear;   // aircraft landing gear status
};

export struct Engine {
  float thrust;
  float throttle;
};

export struct AircraftControls {
  float roll;
  float pitch;
  float yaw;
  bool brake;  // pressed or not
  bool gear;   // pressed or not
  float throttle;
  bool afterBurner;
};

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

// specific player/world structures

export struct World {
  // entt::resource<ModelResourceLoader> surface;
  // entt::resource<ModelResourceLoader> clouds;
  // entt::resource<MusicStreamResourceLoader> streams;
};

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
  int runway_length_m;
  int runway_width_m;
  int elevation_m;
};

export struct Waypoints {
  std::vector<Vector3> waypoints;
};

// position

export struct Position3D {
  Vector3 pos;
  Vector3 offset;
};

export struct Position2D {
  Vector2 pos;
};

// dimensions

export struct Dimensions3D {
  Vector3 size;
};

export struct Dimensions2D {
  Vector2 size;
};

// movement

export struct LinearVelocity {
  Vector3 velocity;
  float speed;
};

export struct AngularVelocity {
  Vector3 velocity;
};

export struct LinerAcceleration {
  Vector3 acc;
};

export struct AngularAcceleration {
  Vector3 acc;
};

export struct Rotation {
  Quaternion rotation;
};

export struct Orientation {
  Vector3 forward;
  Vector3 up;
  Vector3 right;
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
