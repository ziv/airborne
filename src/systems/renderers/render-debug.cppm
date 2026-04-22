module;
#include <entt/entt.hpp>

#include "../../lib/ray.hpp"

export module RenderSystem:Debug;

import Components;
import Accessors;
import ResourceManager;
import TerrainStreaming;

export void RenderDebug(entt::registry& registry) {
  // auto view = registry.view<Player, PlayerInputs, GroundHeight>();
  //
  // if (view.begin() == view.end()) {
  //     return;
  // }

  // const entt::entity entity = view.front();
  // auto [player, inputs, gh] = view.get<const Player, const PlayerInputs, const GroundHeight>(entity);

  // const auto f = registry.ctx().get<Forces>();
  // auto [pos, utils, engine, controls, acc, angVel, vel, orient] = view.get<Player, Position3D, AircraftUtils, Engine, AircraftControls, LinerAcceleration,
  // AngularVelocity, LinearVelocity, Orientation>(entity);
  //
  //
  const auto& player = get_player(registry);
  const auto& inputs = get_player_inputs(registry);
  const auto& radar = registry.get<RadarState>(get_player_entity(registry));

  int y = 10;
  constexpr int margin = 15;
  constexpr int fs = 10;
  DrawRectangle(5, 5, 150, 400, BEIGE);

  // relative position
  DrawText(TextFormat("Px: %0.00f", player.pos.x), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("Pz: %0.00f", player.pos.z), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("Py: %0.00f", player.pos.y), margin, y, fs, BLACK);
  y += margin;

  // absolute position
  const Vector3 absPos = player.pos - player.offset;
  DrawText(TextFormat("Ax: %0.00f", absPos.x), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("Az: %0.00f", absPos.z), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("Ay: %0.00f", absPos.y), margin, y, fs, BLACK);
  y += margin;

  y += margin;
  DrawText(TextFormat("Spd: %f", player.speed), margin, y, fs, BLACK);

  y += margin;
  DrawText(TextFormat("Ct: %f", inputs.throttle), margin, y, fs, BLACK);
  // y += margin;
  // DrawText(TextFormat("Et: %f", player.throttle), margin, y, shaders, BLACK);

  y += margin;
  DrawText(TextFormat("LVx: %f", player.velocity.x), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("LVz: %f", player.velocity.z), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("LVy: %f", player.velocity.y), margin, y, fs, BLACK);

  y += margin;
  DrawText(TextFormat("Ra: %d", radar.mode == RadarMode::AIR_TO_AIR), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("Rg: %d", radar.mode == RadarMode::AIR_TO_GROUND), margin, y, fs, BLACK);
  y += margin;
  DrawText(TextFormat("Tr: %d", radar.locked_target != entt::null), margin, y, fs, BLACK);
  y += margin;
  if (radar.locked_target != entt::null) {
    DrawText(TextFormat("Te: %d", static_cast<int>(radar.locked_target)), margin, y, fs, BLACK);
    y += margin;
    const auto target_position = registry.get<Position3D>(radar.locked_target);
    DrawText(TextFormat("TPx: %0.00f", static_cast<float>(target_position.pos.x)), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("TPz: %0.00f", target_position.pos.z), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("TPy: %0.00f", target_position.pos.y), margin, y, fs, BLACK);
  }

  const auto& rm = get_resource_manager(registry);
  for (const auto view = registry.view<TerrainChunk, Position3D>(); const auto entity : view) {
    const auto& [chunk, pos] = view.get<const TerrainChunk, const Position3D>(entity);
    y += margin;
    DrawText(TextFormat("CHUNK %.2f,%.2f,%.2f", pos.pos.x, pos.pos.y, pos.pos.z), margin, y, fs, BLACK);
  }

  // ground height
  // y += margin;
  // DrawText(TextFormat("GH: %f", gh.height), margin, y, fs, BLACK);
  //
  // y += margin;
  // DrawText(TextFormat("TH: %f", f.thrust), margin, y, fs, BLUE);
  // y += margin;
  // DrawText(TextFormat("DR: %f", f.drag), margin, y, fs, BLUE);
  // y += margin;
  // DrawText(TextFormat("LT: %f", f.lift), margin, y, fs, BLUE);
  // y += margin;
  // DrawText(TextFormat("MS: %f", f.mass), margin, y, fs, BLUE);
  //
  // y += margin;
  // DrawText(TextFormat("FAx: %f", f.acceleration.x), margin, y, fs, BLUE);
  // y += margin;
  // DrawText(TextFormat("FAz: %f", f.acceleration.z), margin, y, fs, BLUE);
  // y += margin;
  // DrawText(TextFormat("FAy: %f", f.acceleration.y), margin, y, fs, BLUE);

  // y += margin;
  // DrawText(TextFormat("AVz: %f", angVel.velocity.z), margin, y, shaders, BLACK);
  // y += margin;
  // DrawText(TextFormat("AVy: %f", angVel.velocity.y), margin, y, shaders, BLACK);
  //
  // y += margin;
  // const float speed = Vector3Length(vel.velocity);
  // DrawText(TextFormat("Spd: %.1f", speed), margin, y, shaders, BLACK);
  //
  // y += margin;
  // float heading = atan2f(orient.forward.x, orient.forward.z) * RAD2DEG;
  // if (heading < 0.0f) heading += 360.0f;
  // DrawText(TextFormat("Hdg: %.0f", heading), margin, y, shaders, BLACK);
  //
  // y += margin;
  // const float pitch = asinf(-orient.forward.y) * RAD2DEG;
  // DrawText(TextFormat("Pit: %.1f", pitch), margin, y, shaders, BLACK);
  //
  // y += margin;
  // const float roll = atan2f(orient.right.y, orient.up.y) * RAD2DEG;
  // DrawText(TextFormat("Rol: %.1f", roll), margin, y, shaders, BLACK);
  //
  // y += margin;
  // DrawText(TextFormat("Cq: %.1f", controls.yaw), margin, y, shaders, BLACK);
}
