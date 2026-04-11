/**
 * @file AircraftCamera.cpp
 * @brief Implementation of the first-person pilot camera.
 */
#include "AircraftCamera.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftCamera::AircraftCamera(const AppConfig &config) : conf(config.get<AircraftCameraConfig>("/pilot")) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = conf.fov;
    camera.projection = CAMERA_PERSPECTIVE;
}

void AircraftCamera::update(const AircraftState &state, const float dt) {
    // default first-person view: apply downward tilt around the right axis
    camera.position = state.position;
    const Quaternion qTilt = QuaternionFromAxisAngle(state.orientation.right, -conf.tilt);
    camera.target = camera.position + Vector3RotateByQuaternion(state.orientation.forward, qTilt);
    camera.up = Vector3RotateByQuaternion(state.orientation.up, qTilt);

    // F1 overrides with an external chase camera positioned behind and to the left
    if (IsKeyDown(KEY_F1)) {
        const Vector3 offsetRight = state.orientation.right * -100.0f;
        const Vector3 offsetUp = state.orientation.up * 0.0f;
        const Vector3 offsetForward = state.orientation.forward * -100.0f;

        const Vector3 totalOffset = offsetRight + offsetUp + offsetForward;

        camera.position = state.position + totalOffset;
        camera.target = state.position;
        camera.up = state.orientation.up;
    }
}
