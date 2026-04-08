#include "AircraftCamera.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftCamera::AircraftCamera(const AppConfig &config) : pilotTilt(config.get<float>("/pilot/tilt")) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.get<float>("/pilot/fov");
    camera.projection = CAMERA_PERSPECTIVE;
}

void AircraftCamera::update(const AircraftState &state, const float dt) {
    camera.position = state.position;
    const Quaternion qTilt = QuaternionFromAxisAngle(state.orientation.right, -pilotTilt);
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(state.orientation.forward, qTilt));
    camera.up = Vector3RotateByQuaternion(state.orientation.up, qTilt);

    if (IsKeyDown(KEY_F1)) {
        const Vector3 offsetRight =  Vector3Scale(state.orientation.right, -100.0f);
        const Vector3 offsetUp = Vector3Scale(state.orientation.up, 0.0f);
        const Vector3 offsetForward = Vector3Scale(state.orientation.forward, -100.0f);

        const Vector3 totalOffset = Vector3Add(offsetRight, Vector3Add(offsetUp, offsetForward));

        camera.position = Vector3Add(state.position, totalOffset);
        camera.target = state.position;
        camera.up = state.orientation.up;
    }
}