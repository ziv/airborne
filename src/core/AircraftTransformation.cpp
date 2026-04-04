#include "AircraftTransformation.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftTransformation::AircraftTransformation(const AppConfig &config) : maxSpeed(config.get<float>("/airplane/maxSpeed")),
                                                                          vleSpeed(config.get<float>("/airplane/vleSpeed")),
                                                                          stallSpeed(config.get<float>("/airplane/stallSpeed")),
                                                                          bankInduceYawRatio(config.get<float>("/airplane/bankInduceYawRatio")),
                                                                          liftLossPitchRatio(config.get<float>("/airplane/liftLossPitchRatio")) {
}

void AircraftTransformation::update(AircraftState &state, const float dt) const {
    if (state.flying) flyingOrientation(state, dt);
    else groundOrientation(state, dt);
}

void AircraftTransformation::flyingOrientation(AircraftState &state, const float dt) const {
    const auto speedRatio = state.forces.speed / maxSpeed;

    // some effects (arcade style)

    // https://en.wikipedia.org/wiki/Adverse_yaw
    const auto bankInducedYaw = state.forces.speed == 0 ? 0.0f : state.orientation.right.y * bankInduceYawRatio * dt;
    // https://en.wikipedia.org/wiki/Stall_(fluid_dynamics)
    const auto liftLossPitch = state.forces.speed == 0 ? 0.0f : (1.0f - state.orientation.up.y) * liftLossPitchRatio * dt;

    // more speed equals more steering except yaw
    const auto totalPitch = (state.controls.pitch + liftLossPitch) * speedRatio;
    const auto totalYaw = state.controls.yaw + bankInducedYaw;
    const auto totalRoll = state.controls.roll * speedRatio;

    // apply the changes
    const auto qPitch = QuaternionFromAxisAngle(state.orientation.right, totalPitch);
    const auto qYaw = QuaternionFromAxisAngle(state.orientation.up, totalYaw);
    const auto qRoll = QuaternionFromAxisAngle(state.orientation.forward, totalRoll);

    // create turbulence when above VLE speed and gear is open
    // https://en.wikipedia.org/wiki/V_speeds#VLE
    auto qTurbulence = QuaternionIdentity();
    if (state.controls.gear && state.forces.speed > vleSpeed) {
        const float overSpeed = state.forces.speed - vleSpeed;
        const float turbulenceIntensity = Clamp((overSpeed * overSpeed) * 0.0001f * dt, 0.0f, 0.03f);

        const float noisePitch = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
        const float noiseYaw = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity;
        const float noiseRoll = (static_cast<float>(GetRandomValue(-100, 100)) / 100.0f) * turbulenceIntensity * 0.5f;

        const auto qTPitch = QuaternionFromAxisAngle(state.orientation.right, noisePitch);
        const auto qTYaw = QuaternionFromAxisAngle(state.orientation.up, noiseYaw);
        const auto qTRoll = QuaternionFromAxisAngle(state.orientation.forward, noiseRoll);

        qTurbulence = QuaternionMultiply(qTYaw, QuaternionMultiply(qTPitch, qTRoll));
    }

    // all of them together (the order is important!) adding the turbulence last
    const auto qDelta = QuaternionMultiply(qTurbulence, QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll)));

    // update the quaternion and normalize, then recalculate vectors
    state.orientation.rotation = QuaternionNormalize(QuaternionMultiply(qDelta, state.orientation.rotation));
    state.orientation.forward = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldForward, state.orientation.rotation));
    state.orientation.up = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldUp, state.orientation.rotation));
    state.orientation.right = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldRight, state.orientation.rotation));
}

void AircraftTransformation::groundOrientation(AircraftState &state, const float dt) const {
    const auto speedRatio = state.forces.speed / maxSpeed;

    // apply the changes (more speed equals more steering except yaw)
    // on ground pitch can be positive only
    const auto pitch = state.controls.pitch > 0.0f ? state.controls.pitch : 0.0f;
    const auto qPitch = QuaternionFromAxisAngle(state.orientation.right, pitch * speedRatio);
    const auto qYaw = QuaternionFromAxisAngle(state.orientation.up, state.controls.yaw);
    const auto qRoll = QuaternionFromAxisAngle(state.orientation.forward, 0);

    // update the quaternion and normalize, then recalculate vectors
    const auto qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));
    state.orientation.rotation = QuaternionNormalize(QuaternionMultiply(qDelta, state.orientation.rotation));
    state.orientation.forward = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldForward, state.orientation.rotation));
    state.orientation.up = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldUp, state.orientation.rotation));
    state.orientation.right = Vector3Normalize(Vector3RotateByQuaternion(GamePhysics::WorldRight, state.orientation.rotation));
}
