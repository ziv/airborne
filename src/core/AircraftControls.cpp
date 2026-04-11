/**
 * @file AircraftControls.cpp
 * @brief Keyboard-to-PilotControls mapping implementation.
 */
#include "AircraftControls.h"
#include "raylib.h"
#include "raymath.h"

AircraftControls::AircraftControls(const AppConfig &config) : conf(config.get<AircraftControlsConfig>("/airplane")) {
}

void AircraftControls::update(AircraftState &state, const float dt) const {
    state.controls.pitch = 0.0f;
    state.controls.roll = 0.0f;
    state.controls.yaw = 0.0f;

    // steering
    if (IsKeyDown(KEY_UP)) state.controls.pitch = -conf.pitchRatio * dt;
    if (IsKeyDown(KEY_DOWN)) state.controls.pitch = conf.pitchRatio * dt;

    if (IsKeyDown(KEY_LEFT)) state.controls.roll = -conf.rollRatio * dt;
    if (IsKeyDown(KEY_RIGHT)) state.controls.roll = conf.rollRatio * dt;

    if (IsKeyDown(KEY_Q)) state.controls.yaw = conf.yawRatio * dt;
    if (IsKeyDown(KEY_E)) state.controls.yaw = -conf.yawRatio * dt;

    // throttling
    if (IsKeyDown(KEY_ZERO)) state.controls.throttle = 0.0f;
    if (IsKeyDown(KEY_ONE)) state.controls.throttle = 0.1f;
    if (IsKeyDown(KEY_TWO)) state.controls.throttle = 0.2f;
    if (IsKeyDown(KEY_THREE)) state.controls.throttle = 0.3f;
    if (IsKeyDown(KEY_FOUR)) state.controls.throttle = 0.4f;
    if (IsKeyDown(KEY_FIVE)) state.controls.throttle = 0.5f;
    if (IsKeyDown(KEY_SIX)) state.controls.throttle = 0.6f;
    if (IsKeyDown(KEY_SEVEN)) state.controls.throttle = 0.7f;
    if (IsKeyDown(KEY_EIGHT)) state.controls.throttle = 0.8f;
    if (IsKeyDown(KEY_NINE)) state.controls.throttle = 0.9f;
    if (IsKeyDown(KEY_A)) state.controls.throttle = 1.2f; // after burners

    // increase/decrease throttle
    if (IsKeyDown(KEY_MINUS)) state.controls.throttle -= 0.2f * dt;
    if (IsKeyDown(KEY_EQUAL)) state.controls.throttle += 0.2f * dt;

    state.controls.throttle = Clamp(state.controls.throttle, 0.0f, 1.2f);

    // brakes & gear
    if (IsKeyPressed(KEY_B)) state.controls.brakes = !state.controls.brakes;

    // gear toggle: deploying is always allowed, but retracting is blocked
    // while still on the ground (state.flying == false) to prevent
    // collapsing the landing gear during taxi / takeoff roll.
    if (IsKeyPressed(KEY_G)) {
        if (state.controls.gear) {
            // trying to retract — only permitted once airborne
            if (state.flying) state.controls.gear = false;
        } else {
            // deploying gear — always allowed
            state.controls.gear = true;
        }
    }
}
