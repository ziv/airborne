#include "GameData.h"
#include "raymath.h"
#include "Constants.h"

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
    throttle += controls.Throttle;
    throttle = Clamp(throttle, 0.0f, 1.2f);

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
    // todo should take the real height from map
    // and should be replace in more complicated
    // method that check landing status
    // if the aircraft pass the stall barrier it become
    // airborne even if its on the ground
    if (camera.position.y <= 10 && speed < config.stallSpeed()) applyGroundPhysics();
    else applyFlightPhysics();

    // const auto currentSpeed = speed;
    const float mass = config.weight() / 9.81f;

    // forces vectors
    const auto thrustForce = Vector3Scale(getForward(), thrust);
    const auto dragForce = Vector3Scale(getForward(), -drag);
    const auto liftForce = Vector3Scale(getUp(), lift);
    const auto weightForce = Vector3Scale(GamePhysics::Gravity, mass);

    const auto total = Vector3Add(Vector3Add(Vector3Add(thrustForce, dragForce), weightForce), liftForce);
    const auto acceleration = Vector3Scale(total, 1.0f / mass);

    // acceleration
    velocity = Vector3Add(velocity, Vector3Scale(acceleration, deltaTime));
    speed = Vector3Length(velocity);

    // limit velocity
    if (speed > config.maxSpeed() && speed != 0.0f) {
        velocity = Vector3Scale(velocity, config.maxSpeed() / speed);
        speed = Vector3Length(velocity);
    }
}

void GameData::applyGroundPhysics() {
    const auto speedRatio = speed / config.maxSpeed();

    // apply the changes (more speed equals more steering except yaw)
    const auto p = controls.Pitch > 0.0f ? controls.Pitch : 0.0f;
    const auto qPitch = QuaternionFromAxisAngle(right, p * speedRatio);
    const auto qYaw = QuaternionFromAxisAngle(up, controls.Yaw);
    const auto qRoll = QuaternionFromAxisAngle(forward, 0);

    // update the quaternion and normalize, then recalculate vectors
    const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalcVectors();

    // some "physics"
    thrust = throttle * config.engineThrust();
    drag = (speed * speed) * config.dragCoefficient();
    lift = (speed * speed) * config.liftCoefficient();

    // on ground drag is also the wheels break
    if (breaks) drag *= 1000;
    if (breaks && speed < 10) velocity = Vector3Scale(velocity, 0.9f);

    // ground is always stall...
    lift *= 0.1;
}

void GameData::applyFlightPhysics() {
    const auto currentSpeed = speed;
    const auto speedRatio = currentSpeed / config.maxSpeed();

    // some effects (arcade style)
    const auto bankInducedYaw = currentSpeed == 0 ? 0.0f : right.y * config.bankInduceYawRatio() * deltaTime;
    const auto liftLossPitch = currentSpeed == 0 ? 0.0f : (1.0f - up.y) * config.liftLossPitchRatio() * deltaTime;

    // apply the changes (more speed equals more steering except yaw)
    const auto qPitch = QuaternionFromAxisAngle(right, (controls.Pitch + liftLossPitch) * speedRatio);
    const auto qYaw = QuaternionFromAxisAngle(up, controls.Yaw + bankInducedYaw);
    const auto qRoll = QuaternionFromAxisAngle(forward, controls.Roll * speedRatio);

    // create turbulence when above VLE speed and gear is open
    auto qTurbulence = QuaternionIdentity();
    if (gear && currentSpeed > config.vleSpeed()) {
        const float overSpeed = currentSpeed - config.vleSpeed();
        const float turbulenceIntensity = Clamp((overSpeed * overSpeed) * 0.0001f * deltaTime, 0.0f, 0.03f);

        const float noisePitch = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
        const float noiseYaw = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
        const float noiseRoll = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity * 0.5f;

        const auto qTPitch = QuaternionFromAxisAngle(right, noisePitch);
        const auto qTYaw = QuaternionFromAxisAngle(up, noiseYaw);
        const auto qTRoll = QuaternionFromAxisAngle(forward, noiseRoll);

        qTurbulence = QuaternionMultiply(qTYaw, QuaternionMultiply(qTPitch, qTRoll));
    }

    // all of them together (the order is important!)
    // adding the turbulence last
    const auto qDelta = QuaternionMultiply(qTurbulence, QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll)));

    // update the quaternion and normalize, then recalculate vectors
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalcVectors();

    // some "physics"
    const auto isStalling = currentSpeed < config.stallSpeed();

    // weathervaning
    // https://en.wikipedia.org/wiki/Weathervane_effect
    if (!isStalling) {
        auto [x, y, z] = Vector3Scale(getForward(), currentSpeed);
        velocity.x = Lerp(velocity.x, x, 2.0f * deltaTime);
        velocity.y = Lerp(velocity.y, y, 2.0f * deltaTime);
        velocity.z = Lerp(velocity.z, z, 2.0f * deltaTime);
    }

    thrust = throttle * config.engineThrust();
    drag = (currentSpeed * currentSpeed) * config.dragCoefficient();
    lift = (currentSpeed * currentSpeed) * config.liftCoefficient();

    // breaks increase drag by 600%
    if (breaks) drag *= 6.0;

    // gear generate drag
    if (gear) drag *= 1.8f;

    // stall reduce lift by 90%
    if (isStalling) lift *= 0.1;
}

void GameData::applyPosition() {
    auto newPosition = Vector3Add(camera.position, Vector3Scale(velocity, deltaTime));
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
