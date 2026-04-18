module;
#include <entt/entt.hpp>
#include "../../lib/ray.hpp"

export module RenderSystem:Debug;

import Components;

export void RenderDebug(entt::registry &registry) {
    auto view = registry.view<Player, PlayerInputs, GroundHeight>();

    if (view.begin() == view.end()) {
        return;
    }

    const entt::entity entity = view.front();
    auto [player, inputs, gh] = view.get<Player, PlayerInputs, GroundHeight>(entity);


    const auto f = registry.ctx().get<Forces>();
    // auto [pos, utils, engine, controls, acc, angVel, vel, orient] = view.get<Player, Position3D, AircraftUtils, Engine, AircraftControls, LinerAcceleration, AngularVelocity, LinearVelocity, Orientation>(entity);
    //
    //
    const Vector3 absPos = player.pos - player.offset;
    int y = 10;
    const int margin = 15;
    const int fs = 10;
    DrawRectangle(5, 5, 150, 340, BEIGE);

    // relative position
    DrawText(TextFormat("Px: %0.00f", player.pos.x), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("Pz: %0.00f", player.pos.z), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("Py: %0.00f", player.pos.y), margin, y, fs, BLACK);
    y += margin;

    // absolute position
    DrawText(TextFormat("Ax: %0.00f", absPos.x), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("Az: %0.00f", absPos.z), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("Ay: %0.00f", absPos.y), margin, y, fs, BLACK);
    y += margin;

    DrawText(TextFormat("Brk: %s", inputs.brakes ? "ON" : "OFF"), margin, y, fs, BLACK);
    y += margin;
    DrawText(TextFormat("Ger: %s", inputs.gear ? "DN" : "UP"), margin, y, fs, BLACK);
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

    // ground height
    y += margin;
    DrawText(TextFormat("GH: %f", gh.height), margin, y, fs, BLACK);

    y += margin;
    DrawText(TextFormat("TH: %f", f.thrust), margin, y, fs, BLUE);
    y += margin;
    DrawText(TextFormat("DR: %f", f.drag), margin, y, fs, BLUE);
    y += margin;
    DrawText(TextFormat("LT: %f", f.lift), margin, y, fs, BLUE);
    y += margin;
    DrawText(TextFormat("MS: %f", f.mass), margin, y, fs, BLUE);

    y += margin;
    DrawText(TextFormat("FAx: %f", f.acceleration.x), margin, y, fs, BLUE);
    y += margin;
    DrawText(TextFormat("FAz: %f", f.acceleration.z), margin, y, fs, BLUE);
    y += margin;
    DrawText(TextFormat("FAy: %f", f.acceleration.y), margin, y, fs, BLUE);

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
