#include "AircraftCamera.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftCamera::AircraftCamera(const AppConfig &config) : pilotTilt(config.get<float>("/pilot/tilt")),
                                                          heightAboveGround(config.get<float>("/airplane/heightAboveGround")) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.get<float>("/pilot/fov");
    camera.projection = CAMERA_PERSPECTIVE;
}

void AircraftCamera::update(AircraftState &state, const float dt) {
    state.position = Vector3Add(camera.position, Vector3Scale(state.forces.velocity, dt));

    // todo temporary solution to not go under ground
    // todo should we use "flying" param?
    // todo the crash/landing system should solve this
    if (state.position.y <= heightAboveGround) {
        state.position.y = heightAboveGround;
        camera.up = GamePhysics::WorldUp;
    }

    camera.position = state.position;
    const Quaternion qTilt = QuaternionFromAxisAngle(state.orientation.right, -pilotTilt);
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(state.orientation.forward, qTilt));
    camera.up = Vector3RotateByQuaternion(state.orientation.up, qTilt);
}

void AircraftCamera::setPosition(const Vector3 &position) {
    camera.position = position;
}
