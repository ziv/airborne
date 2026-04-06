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
}