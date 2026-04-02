#include "AircraftTransformation.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftTransformation::AircraftTransformation(const AppConfig &config) : maxSpeed(config.get<float>("/airplane/maxSpeed")),
                                                                          vleSpeed(config.get<float>("/airplane/vleSpeed")),
                                                                          stallSpeed(config.get<float>("/airplane/stallSpeed")),
                                                                          bankInduceYawRatio(config.get<float>("/airplane/bankInduceYawRatio")),
                                                                          liftLossPitchRatio(config.get<float>("/airplane/liftLossPitchRatio")) {
}

void AircraftTransformation::update(const float dt, const bool flying, const PilotControls &controls, const ForcesState &forces) {
    if (flying) flyingOrientation(dt, forces.speed, controls);
    else groundOrientation(forces.speed, controls);
}

void AircraftTransformation::flyingOrientation(const float dt, const MeterPerSecond speed, const PilotControls &controls) {
    const auto speedRatio = speed / maxSpeed;

    // some effects (arcade style)

    // https://en.wikipedia.org/wiki/Adverse_yaw
    const auto bankInducedYaw = speed == 0 ? 0.0f : dir.right.y * bankInduceYawRatio * dt;
    // https://en.wikipedia.org/wiki/Stall_(fluid_dynamics)
    const auto liftLossPitch = speed == 0 ? 0.0f : (1.0f - dir.up.y) * liftLossPitchRatio * dt;

    // more speed equals more steering except yaw
    const auto totalPitch = (controls.pitch + liftLossPitch) * speedRatio;
    const auto totalYaw = controls.yaw + bankInducedYaw;
    const auto totalRoll = controls.roll * speedRatio;

    // apply the changes
    const auto qPitch = QuaternionFromAxisAngle(dir.right, totalPitch);
    const auto qYaw = QuaternionFromAxisAngle(dir.up, totalYaw);
    const auto qRoll = QuaternionFromAxisAngle(dir.forward, totalRoll);

    // create turbulence when above VLE speed and gear is open
    // https://en.wikipedia.org/wiki/V_speeds#VLE
    auto qTurbulence = QuaternionIdentity();
    if (controls.gear && speed > vleSpeed) {
        const float overSpeed = speed - vleSpeed;
        const float turbulenceIntensity = Clamp((overSpeed * overSpeed) * 0.0001f * dt, 0.0f, 0.03f);

        const float noisePitch = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
        const float noiseYaw = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
        const float noiseRoll = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity * 0.5f;

        const auto qTPitch = QuaternionFromAxisAngle(dir.right, noisePitch);
        const auto qTYaw = QuaternionFromAxisAngle(dir.up, noiseYaw);
        const auto qTRoll = QuaternionFromAxisAngle(dir.forward, noiseRoll);

        qTurbulence = QuaternionMultiply(qTYaw, QuaternionMultiply(qTPitch, qTRoll));
    }

    // all of them together (the order is important!) adding the turbulence last
    const auto qDelta = QuaternionMultiply(qTurbulence, QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll)));

    // update the quaternion and normalize, then recalculate vectors
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalculateDirectionVectors();
}

void AircraftTransformation::groundOrientation(const MeterPerSecond speed, const PilotControls &controls) {
    const auto speedRatio = speed / maxSpeed;

    // apply the changes (more speed equals more steering except yaw)
    // on ground pitch can be positive only
    const auto pitch = controls.pitch > 0.0f ? controls.pitch : 0.0f;
    const auto qPitch = QuaternionFromAxisAngle(dir.right, pitch * speedRatio);
    const auto qYaw = QuaternionFromAxisAngle(dir.up, controls.yaw);
    const auto qRoll = QuaternionFromAxisAngle(dir.forward, 0);

    // update the quaternion and normalize, then recalculate vectors
    const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));
    recalculateDirectionVectors();
}

void AircraftTransformation::recalculateDirectionVectors() {
    dir.forward = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldForward, rotation));
    dir.up = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation));
    dir.right = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldRight, rotation));
}

Quaternion &AircraftTransformation::getRotation() {
    return rotation;
}

Directions &AircraftTransformation::getDirections() {
    return dir;
}
