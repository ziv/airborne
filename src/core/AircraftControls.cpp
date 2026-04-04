#include "AircraftControls.h"
#include "raylib.h"
#include "raymath.h"

AircraftControls::AircraftControls(const AppConfig &config) : pitchRatio(config.get<float>("/airplane/pitchRatio")),
                                                              rollRatio(config.get<float>("/airplane/rollRatio")),
                                                              yawRatio(config.get<float>("/airplane/yawRatio")) {
}

void AircraftControls::update(AircraftState &state, const float dt) const {
    state.controls.pitch = 0.0f;
    state.controls.roll = 0.0f;
    state.controls.yaw = 0.0f;

    // steering
    if (IsKeyDown(KEY_UP)) state.controls.pitch = -pitchRatio * dt;
    if (IsKeyDown(KEY_DOWN)) state.controls.pitch = pitchRatio * dt;

    if (IsKeyDown(KEY_LEFT)) state.controls.roll = -rollRatio * dt;
    if (IsKeyDown(KEY_RIGHT)) state.controls.roll = rollRatio * dt;

    if (IsKeyDown(KEY_Q)) state.controls.yaw = yawRatio * dt;
    if (IsKeyDown(KEY_E)) state.controls.yaw = -yawRatio * dt;

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
    if (IsKeyDown(KEY_MINUS)) state.controls.throttle -= 0.05f * dt;
    if (IsKeyDown(KEY_EQUAL)) state.controls.throttle += 0.05f * dt;

    state.controls.throttle = Clamp(state.controls.throttle, 0.0f, 1.2f);

    // brakes & gear
    if (IsKeyPressed(KEY_B)) state.controls.brakes = !state.controls.brakes;
    if (IsKeyPressed(KEY_G)) state.controls.gear = !state.controls.gear;
}
