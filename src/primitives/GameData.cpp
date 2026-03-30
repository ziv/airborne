#include "GameData.h"
#include "raymath.h"
#include "../Constants.h"

GameData::GameData(AppConfig &config) : config(config) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.pilotFov();
    camera.projection = CAMERA_PERSPECTIVE;

    recalcVectors();
}

void GameData::Update() {
    applyState();
    applyForces();
    applyPosition();
    // check colision/landing
}

void GameData::recalcVectors() {
    forward = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldForward, rotation));
    up = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation));
    right = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldRight, rotation));
}

float GameData::Tick() {
    deltaTime = GetFrameTime();
    width = static_cast<float>(GetScreenWidth());
    height = static_cast<float>(GetScreenHeight());
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

bool GameData::isStableLanding() const {
    const Vector3 angles = QuaternionToEuler(rotation);
    const float pitch = angles.x * RAD2DEG;
    const float roll = angles.z * RAD2DEG;

    constexpr float tolerance = 5.0f;

    const bool wingsLevel = fabsf(roll) < tolerance;
    const bool noseUp = pitch > 0.0f && pitch < 10.0f;

    return wingsLevel && noseUp;
}

void GameData::applyState() {
    constexpr auto aircraftHeightFromGround = 3.0f;
    if (planeState == Flying && GetPosition().y <= aircraftHeightFromGround) {
        // in order to land you have to fulfill all those condition
        // note, this condition does not cover what you landed on
        if (gearState == Opened && isStableLanding()) {
            planeState = Ground;
        } else {
            planeState = Crushed;
        }
    }
    // just leave the fround and the aircraft is flying
    else if (planeState == Ground && GetPosition().y > aircraftHeightFromGround) {
        planeState = Flying;
    }
}

void GameData::applyForces() {
    // some effects (arcade style)
    const auto speed = Speed();
    const auto speedRatio = speed / config.maxSpeed();
    const auto bankInducedYaw = speed == 0 ? 0.0f : right.y * config.bankInduceYawRatio() * deltaTime;
    const auto liftLossPitch = speed == 0 ? 0.0f : (1.0f - up.y) * config.liftLossPitchRatio() * deltaTime;

    // apply the changes (more speed equals more steering except yaw)
    const auto qPitch = QuaternionFromAxisAngle(right, (controls.Pitch + liftLossPitch) * speedRatio);
    const auto qYaw = QuaternionFromAxisAngle(up, controls.Yaw + bankInducedYaw);
    const auto qRoll = QuaternionFromAxisAngle(forward, controls.Roll * speedRatio);

    // all of them together (the order is important!)
    const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update the quaternion and normalize, then recalculate vectors
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalcVectors();

    // some "physics"
    const auto isStalling = speed < config.stallSpeed();
    const auto thrust = throttle * config.engineThrust();
    auto drag = (speed * speed) * config.dragCoefficient();
    auto lift = (speed * speed) * config.liftCoefficient();

    // breaks increase drag by 300%
    // stall reduce lift by 90%
    if (breaks) drag *= 3;
    if (isStalling) lift *= 0.1;

    // forces vectors
    const auto thrustForce = Vector3Scale(GetForward(), thrust);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, config.weight());
    const auto liftForce = Vector3Scale(GetUp(), lift);
    const auto dragForce = Vector3Scale(GetForward(), drag);

    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);

    // acceleration
    velocity = Vector3Add(velocity, Vector3Scale(total, deltaTime * deltaTime));

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (!isStalling) {
        auto [x, y, z] = Vector3Scale(GetForward(), speed);
        velocity.x = Lerp(velocity.x, x, 2.0f * deltaTime);
        velocity.y = Lerp(velocity.y, y, 2.0f * deltaTime);
        velocity.z = Lerp(velocity.z, z, 2.0f * deltaTime);
    }

    // limit velocity
    if (speed > config.maxSpeed() && speed != 0.0f) {
        velocity = Vector3Scale(velocity, config.maxSpeed() / speed);
    }
}

void GameData::applyPosition() {
    auto newPosition = Vector3Add(camera.position, Vector3Scale(velocity, deltaTime));

    // without gear this is not a landing but crushing...
    // if (gear && isStableLanding() && newPosition.y <= 10.0f) {
    // }
    // if (!gear && newPosition.y < 3.0f) {
    //     crashed = true;
    // }
    // if (gear && newPosition.y < 10.0f && isStableLanding()) {
    //
    // }

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
