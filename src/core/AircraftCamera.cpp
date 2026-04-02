#include "AircraftCamera.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftCamera::AircraftCamera(const AppConfig &config) : pilotTilt(config.get<float>("/pilot/tilt")),
                                                          heightAboveGround(config.get<float>("/airplane/heightAboveGround")) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.get<float>("/pilot/fov");
    camera.projection = CAMERA_PERSPECTIVE;
}

void AircraftCamera::update(const float dt, const Directions &directions, const ForcesState &forces) {
    camera.position = Vector3Add(camera.position, Vector3Scale(forces.velocity, dt));

    // todo temporary solution to not go under ground
    // todo should we use "flying" param?
    // todo the crash/landing system should solve this
    if (camera.position.y <= heightAboveGround) {
        camera.position.y = heightAboveGround;
        camera.up = GamePhysics::WorldUp;
    }

    const Quaternion qTilt = QuaternionFromAxisAngle(directions.right, -pilotTilt);
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(directions.forward, qTilt));
    camera.up = Vector3RotateByQuaternion(directions.up, qTilt);
}

void AircraftCamera::setPosition(const Vector3 &position) {
    camera.position = position;
}
