#include "GameData.h"
#include "raymath.h"
#include "Utils.h"
#include "../Constants.h"

GameData::GameData(AppConfig &config) : config(config) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.pilotFov();
    camera.projection = CAMERA_PERSPECTIVE;

    recalcVectors();
}

void GameData::Update() {
    Tick();
    // some effects (arcade style)
    const auto speed = Speed();
    const float bankInducedYaw = speed == 0 ? 0 : right.y * .2f * deltaTime;
    const float liftLossPitch = speed == 0 ? 0 : (1.0f - up.y) * 0.1f * deltaTime;

    // apply the changes
    const auto qPitch = QuaternionFromAxisAngle(right, controls.Pitch + liftLossPitch);
    const auto qYaw = QuaternionFromAxisAngle(up, controls.Yaw + bankInducedYaw);
    const auto qRoll = QuaternionFromAxisAngle(forward, controls.Roll);

    // all of them together (the order is important!)
    const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update the quaternion and normalize, then recalculate vectors
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalcVectors();

    // pilot look a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(right, -config.pilotTilt());

    velocity = UpdatePhysics(*this, config);

    // position the pilot
    // camera.position = Vector3Add(camera.position, Vector3Scale(forward, velocity * deltaTime));
    camera.position = Vector3Add(camera.position, Vector3Scale(velocity, deltaTime));
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(forward, qTilt));
    camera.up = Vector3RotateByQuaternion(up, qTilt);
}

void GameData::recalcVectors() {
    forward = Vector3RotateByQuaternion(GamePhysics::WorldForward, rotation);
    up = Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation);
    right = Vector3RotateByQuaternion(GamePhysics::WorldRight, rotation);
}

float GameData::Tick() {
    deltaTime = GetFrameTime();
    return deltaTime;
}

void GameData::ToggleAutopilot() {
    autoPiloting = !autoPiloting;
}

void GameData::ToggleBreaks() {
    breaks = !breaks;
}

PilotControls GameData::ResetControls() {
    controls = {0.0f, 0.0f, 0.0f, 0.0f};
    return controls;
}
