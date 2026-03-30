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
    // some effects (arcade style)
    const auto speed = Speed();
    const auto bankInducedYaw = speed == 0 ? 0.0f : right.y * config.bankInduceYawRatio() * deltaTime;
    const auto liftLossPitch = speed == 0 ? 0.0f : (1.0f - up.y) * config.liftLossPitchRatio() * deltaTime;

    // apply the changes
    const auto qPitch = QuaternionFromAxisAngle(right, controls.Pitch + liftLossPitch);
    const auto qYaw = QuaternionFromAxisAngle(up, controls.Yaw + bankInducedYaw);
    const auto qRoll = QuaternionFromAxisAngle(forward, controls.Roll);

    // all of them together (the order is important!)
    const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update the quaternion and normalize, then recalculate vectors
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalcVectors();

    // some "physics"

    // thrust
    const auto engineThrust = throttle * config.engineThrust();
    const auto thrustForce = Vector3Scale(GetForward(), engineThrust);

    // lift
    auto liftMagnitude = (speed * speed) * config.liftCoefficient();
    auto isStalling = false;
    if (speed < config.stallSpeed()) {
        // too slow, cut the lift by 90%
        liftMagnitude *= 0.1f;
        isStalling = true;
    }
    const auto liftForce = Vector3Scale(GetUp(), liftMagnitude);

    // drag force
    const auto dragMagnitude = speed * config.dragCoefficient();
    const auto dragForce = Vector3Scale(GetForward(), -dragMagnitude);

    // combining all forces
    const auto totalForce = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), GamePhysics::Gravity), liftForce);

    // acceleration results
    velocity = Vector3Add(velocity, Vector3Scale(totalForce, deltaTime));

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (!isStalling) {
        auto [x, y, z] = Vector3Scale(GetForward(), speed);
        velocity.x = Lerp(velocity.x, x, 2.0f * deltaTime);
        velocity.y = Lerp(velocity.y, y, 2.0f * deltaTime);
        velocity.z = Lerp(velocity.z, z, 2.0f * deltaTime);
    }

    // limit velocity
    if (const float current = Vector3Length(velocity); current > config.maxSpeed()) {
        velocity = Vector3Scale(velocity, config.maxSpeed() / speed);
    }

    // todo breaks on/off, zero height, landing
    auto newPosition = Vector3Add(camera.position, Vector3Scale(velocity, deltaTime));

    // do not go under the ground
    if (newPosition.y <= 10.0f) {
        velocity.y = 0.0f;
        newPosition.y = 10.0f;
    }

    // pilot look a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(right, -config.pilotTilt());

    // position the pilot
    camera.position = newPosition;
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

void GameData::SetPosition(const Vector3 &position) {
    camera.position = position;
}