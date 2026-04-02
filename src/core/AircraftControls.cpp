#include "AircraftControls.h"
#include "raylib.h"
#include "raymath.h"

AircraftControls::AircraftControls(const AppConfig &config) : pitchRatio(config.get<float>("/airplane/pitchRatio")),
                                                              rollRatio(config.get<float>("/airplane/rollRatio")),
                                                              yawRatio(config.get<float>("/airplane/yawRatio")) {
}

PilotControls &AircraftControls::getControls() {
    return controls;
}

void AircraftControls::update(const float dt) {
    controls.pitch = 0.0f;
    controls.roll = 0.0f;
    controls.yaw = 0.0f;

    // steering
    if (IsKeyDown(KEY_UP)) controls.pitch = -pitchRatio * dt;
    if (IsKeyDown(KEY_DOWN)) controls.pitch = pitchRatio * dt;

    if (IsKeyDown(KEY_LEFT)) controls.roll = -rollRatio * dt;
    if (IsKeyDown(KEY_RIGHT)) controls.roll = rollRatio * dt;

    if (IsKeyDown(KEY_Q)) controls.yaw = yawRatio * dt;
    if (IsKeyDown(KEY_E)) controls.yaw = -yawRatio * dt;

    // throttling
    if (IsKeyDown(KEY_ZERO)) controls.throttle = 0.0f;
    if (IsKeyDown(KEY_ONE)) controls.throttle = 0.1f;
    if (IsKeyDown(KEY_TWO)) controls.throttle = 0.2f;
    if (IsKeyDown(KEY_THREE)) controls.throttle = 0.3f;
    if (IsKeyDown(KEY_FOUR)) controls.throttle = 0.4f;
    if (IsKeyDown(KEY_FIVE)) controls.throttle = 0.5f;
    if (IsKeyDown(KEY_SIX)) controls.throttle = 0.6f;
    if (IsKeyDown(KEY_SEVEN)) controls.throttle = 0.7f;
    if (IsKeyDown(KEY_EIGHT)) controls.throttle = 0.8f;
    if (IsKeyDown(KEY_NINE)) controls.throttle = 0.9f;
    if (IsKeyDown(KEY_A)) controls.throttle = 1.2f; // after burners

    // increase/decrease throttle
    if (IsKeyDown(KEY_MINUS)) controls.throttle -= 0.05f * dt;
    if (IsKeyDown(KEY_EQUAL)) controls.throttle += 0.05f * dt;

    controls.throttle = Clamp(controls.throttle, 0.0f, 1.2f);

    // brakes & gear
    if (IsKeyPressed(KEY_B)) controls.brakes = !controls.brakes;
    if (IsKeyPressed(KEY_G)) controls.gear = !controls.gear;
}
