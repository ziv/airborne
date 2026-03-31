#include "AircraftControls.h"
#include "raylib.h"
#include "raymath.h"

AircraftControls::AircraftControls(const AppConfig &config) : pitchRatio(config.pitchRatio),
                                                              rollRatio(config.rollRatio),
                                                              yawRatio(config.yawRatio) {
}

PilotControls AircraftControls::getControls() const {
    return (PilotControls){pitch, yaw, roll, throttle, brakes};
}

void AircraftControls::update(const float dt) {
    pitch = 0.0f;
    roll = 0.0f;
    yaw = 0.0f;

    // steering
    if (IsKeyDown(KEY_UP)) pitch = -pitchRatio * dt;
    if (IsKeyDown(KEY_DOWN)) pitch = pitchRatio * dt;

    if (IsKeyDown(KEY_LEFT)) roll = -rollRatio * dt;
    if (IsKeyDown(KEY_RIGHT)) roll = rollRatio * dt;

    if (IsKeyDown(KEY_Q)) yaw = yawRatio * dt;
    if (IsKeyDown(KEY_E)) yaw = -yawRatio * dt;

    // throttling
    if (IsKeyDown(KEY_ZERO)) throttle = 0.0f;
    if (IsKeyDown(KEY_ONE)) throttle = 0.1f;
    if (IsKeyDown(KEY_TWO)) throttle = 0.2f;
    if (IsKeyDown(KEY_THREE)) throttle = 0.3f;
    if (IsKeyDown(KEY_FOUR)) throttle = 0.4f;
    if (IsKeyDown(KEY_FIVE)) throttle = 0.5f;
    if (IsKeyDown(KEY_SIX)) throttle = 0.6f;
    if (IsKeyDown(KEY_SEVEN)) throttle = 0.7f;
    if (IsKeyDown(KEY_EIGHT)) throttle = 0.8f;
    if (IsKeyDown(KEY_NINE)) throttle = 0.9f;
    if (IsKeyDown(KEY_A)) throttle = 1.2f; // after burners

    // increase/decrease throttle
    if (IsKeyDown(KEY_MINUS)) throttle -= 0.05f * dt;
    if (IsKeyDown(KEY_EQUAL)) throttle += 0.05f * dt;

    throttle = Clamp(throttle, 0.0f, 1.2f);

    // brakes & gear
    if (IsKeyPressed(KEY_B)) brakes = !brakes;
    if (IsKeyPressed(KEY_G)) gear = !gear;
}
