#include "GameData.h"
#include "raymath.h"
#include "../Constants.h"

GameData::GameData(AppConfig &config) : config(config) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.pilotFov();
    camera.projection = CAMERA_PERSPECTIVE;

    recalcVectors();
}

GameData::~GameData() {
    // nothing to unload yet
}

void GameData::update() {
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

float GameData::tick() {
    deltaTime = GetFrameTime();
    width = static_cast<float>(GetScreenWidth());
    height = static_cast<float>(GetScreenHeight());
    return deltaTime;
}

void GameData::toggleAutopilot() {
    autoPiloting = !autoPiloting;
}

void GameData::toggleBreaks() {
    breaks = !breaks;
}

void GameData::resetControls() {
    controls = {0.0f, 0.0f, 0.0f, 0.0f};
}

void GameData::setPosition(const Vector3 &position) {
    camera.position = position;
    if (camera.position.y < config.heightAboveGround()) {
        camera.position.y = config.heightAboveGround();
        velocity.y = 0.0f;
        speed = Vector3Length(velocity);
    }
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
    if (planeState == Flying && getPosition().y <= config.heightAboveGround()) {
        // in order to land you have to fulfill all those condition
        // note, this condition does not cover what you landed on
        if (gearState == Opened && isStableLanding()) {
            planeState = Ground;
        } else {
            planeState = Crushed;
        }
    }
    // just leave the fround and the aircraft is flying
    else if (planeState == Ground && getPosition().y > config.heightAboveGround()) {
        planeState = Flying;
    }
}

void GameData::applyForces() {
    throttle += controls.Throttle;
    throttle = Clamp(throttle, 0.0f, 1.2f);

    const auto currentSpeed = speed;
    const auto speedRatio = currentSpeed / config.maxSpeed();

    // some effects (arcade style)
    const auto bankInducedYaw = currentSpeed == 0 ? 0.0f : right.y * config.bankInduceYawRatio() * deltaTime;
    const auto liftLossPitch = currentSpeed == 0 ? 0.0f : (1.0f - up.y) * config.liftLossPitchRatio() * deltaTime;

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
    const auto isStalling = currentSpeed < config.stallSpeed();
    const auto thrust = throttle * config.engineThrust();
    auto drag = (currentSpeed * currentSpeed) * config.dragCoefficient();
    auto lift = (currentSpeed * currentSpeed) * config.liftCoefficient();

    // breaks increase drag by 300%
    // stall reduce lift by 90%
    if (breaks) drag *= 3;
    if (isStalling) lift *= 0.1;

    const float gravityMagnitude = 9.81f;
    const float mass = config.weight() / gravityMagnitude;

    // forces vectors
    const auto thrustForce = Vector3Scale(getForward(), thrust);
    const auto dragForce = Vector3Scale(getForward(), -drag);
    const auto liftForce = Vector3Scale(getUp(), lift);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, mass);

    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
    const auto acceleration = Vector3Scale(total, 1.0f / mass);

    // acceleration
    velocity = Vector3Add(velocity, Vector3Scale(acceleration, deltaTime));

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (!isStalling) {
        auto [x, y, z] = Vector3Scale(getForward(), currentSpeed);
        velocity.x = Lerp(velocity.x, x, 2.0f * deltaTime);
        velocity.y = Lerp(velocity.y, y, 2.0f * deltaTime);
        velocity.z = Lerp(velocity.z, z, 2.0f * deltaTime);
    }

    // limit velocity
    if (currentSpeed > config.maxSpeed() && currentSpeed != 0.0f) {
        velocity = Vector3Scale(velocity, config.maxSpeed() / currentSpeed);
    }

    speed = Vector3Length(velocity);
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
        speed = Vector3Length(velocity);
        newPosition.y = 10.0f;
    }

    // pilot look a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(right, -config.pilotTilt());

    // position the pilot
    camera.position = newPosition;
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(forward, qTilt));
    camera.up = Vector3RotateByQuaternion(up, qTilt);
}
