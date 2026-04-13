#pragma once
#include <entt/entt.hpp>
#include "raymath.h"
#include "../components/world.h"
#include "../components/aircraft.h"
#include "../primitives/Constants.h"

inline void ApplyAircraftPhysics(entt::registry &registry, float dt) {
    auto view = registry.view<Position3D, Velocity, Aerodynamics, Engine, Orientation, Rotation, Aircraft>();

    for (auto [entity, position, velocity, aerodynamics, engine, orientation, rotation, aircraft]: view.each()) {
        const auto cl = aerodynamics.liftCoefficient;
        auto cd = aerodynamics.dragCoefficient;

        if (aircraft.brakes) {
            cd += aerodynamics.brakesDragCoefficient;
        }

        if (aircraft.gear) {
            cd += aerodynamics.gearDragCoefficient;
        }

        const auto speed = Vector3Length(velocity.velocity);
        const auto squaredSpeed = speed * speed;

        const auto thrust = engine.maxThrust * engine.currentThrottle;
        const auto drag = squaredSpeed * cd;
        const auto lift = squaredSpeed * cl;
        const float mass = aerodynamics.weight / GamePhysics::g;

        auto moveDirection = Vector3Normalize(velocity.velocity);

        // --- Force vectors (world space) ---
        // the * operator is Vector3Scale
        const auto thrustForce = orientation.forward * thrust;
        const auto dragForce = moveDirection * -drag; // drag is always oppose the movement direction
        const auto liftForce = orientation.up * lift;
        const auto weightForce = GamePhysics::Gravity * mass;

        const auto total = thrustForce + dragForce + weightForce + liftForce;
        const auto acceleration = total / mass;

        // the + operator is Vector3Add
        // the * operator is Vector3Scale
        velocity.velocity = velocity.velocity + (acceleration * dt);
        position.position = position.position + (velocity.velocity * dt);
    }
}
