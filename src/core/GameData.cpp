#include "GameData.h"

GameData::GameData(const AppConfig &config) : heightAboveGround(config.get<float>("/airplane/heightAboveGround")),
                                              stallSpeed(config.get<float>("/airplane/stallSpeed")),
                                              aircraftControls(config),
                                              aircraftPhysics(config),
                                              aircraftTransformation(config),
                                              aircraftCamera(config) {
}

void GameData::update(const float dt) {
    aircraftControls.update(state, dt);
    aircraftPhysics.update(state, dt);
    aircraftTransformation.update(state, dt);
    aircraftCamera.update(state, dt);

    // is flying?
    state.flying =  state.position.y > heightAboveGround;

    // todo is landed? is crashed?
}

bool GameData::isStableLanding() {
    // const auto position = aircraftCamera.getCamera().position;
    //
    // // todo numbers should come from config
    // const Vector3 angles = QuaternionToEuler(aircraftTransformation.getRotation());
    // const float pitch = angles.x * RAD2DEG;
    // const float roll = angles.z * RAD2DEG;
    //
    // constexpr float tolerance = 5.0f;
    //
    // const bool wingsLevel = fabsf(roll) < tolerance;
    // const bool noseUp = pitch > 0.0f && pitch < 10.0f;
    //
    // return wingsLevel && noseUp;
    return true;
}
