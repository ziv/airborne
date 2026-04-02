#include "GameData.h"

GameData::GameData(const AppConfig &config) : heightAboveGround(config.get<float>("/airplane/heightAboveGround")),
                                              stallSpeed(config.get<float>("/airplane/stallSpeed")),
                                              aircraftControls(config),
                                              aircraftPhysics(config),
                                              aircraftTransformation(config),
                                              aircraftCamera(config) {
}

void GameData::update() {
    const auto dt = GetFrameTime();
    const auto position = aircraftCamera.getCamera().position;
    const auto forces = aircraftPhysics.getForces();
    const auto controls = aircraftControls.getControls();
    const auto directions = aircraftTransformation.getDirections();

    // todo this is a naive solution, improve it
    // todo check for crashing
    const auto flying = position.y > heightAboveGround || forces.speed > stallSpeed;

    // first, read controls state
    aircraftControls.update(dt);

    // change the aircraft orientation
    aircraftTransformation.update(dt, flying, controls, forces);

    // let the physics shine on the aircraft
    aircraftPhysics.update(dt, flying, controls, directions);

    // update its location
    aircraftCamera.update(dt, directions, forces);
}

// bool GameData::isStableLanding() const {
//     const Vector3 angles = QuaternionToEuler(rotation);
//     const float pitch = angles.x * RAD2DEG;
//     const float roll = angles.z * RAD2DEG;
//
//     constexpr float tolerance = 5.0f;
//
//     const bool wingsLevel = fabsf(roll) < tolerance;
//     const bool noseUp = pitch > 0.0f && pitch < 10.0f;
//
//     return wingsLevel && noseUp;
// }
