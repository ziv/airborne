#include "GameData.h"

#include "raymath.h"

GameData::GameData(const AppConfig &config) : heightAboveGround(config.get<float>("/airplane/heightAboveGround")),
                                              stallSpeed(config.get<float>("/airplane/stallSpeed")),
                                              aircraftControls(config),
                                              aircraftPhysics(config),
                                              aircraftTransformation(config),
                                              aircraftCamera(config) {
}

GameData::GameData(const AppConfig &config, const Scenario &scenario)
    : heightAboveGround(config.get<float>("/airplane/heightAboveGround")),
      stallSpeed(config.get<float>("/airplane/stallSpeed")),
      scenario(scenario),
      aircraftControls(config),
      aircraftPhysics(config),
      aircraftTransformation(config),
      aircraftCamera(config) {
    state.position = scenario.start.position;
    state.fuel = scenario.start.fuel;
    spawnInitialEntities();
}

void GameData::spawnInitialEntities() {
    for (const auto &def : scenario.entityDefinitions) {
        if (def.type == EntityType::STRUCTURE) {
            auto gt = std::make_unique<GroundTarget>();
            static_cast<EntityBase &>(*gt) = def;
            gt->strategicTarget = true;
            entities.spawn(std::move(gt));
        } else {
            auto e = std::make_unique<EntityBase>(def);
            entities.spawn(std::move(e));
        }
    }
}

void GameData::update(const float dt) {
    const auto actualHeight = state.position.y - state.groundHeight;
    state.flying = actualHeight > heightAboveGround;;

    if (isStableLanding() && actualHeight <= heightAboveGround) {
        state.flying = false;
    } else {
        state.crushed = true;
    }

    aircraftControls.update(state, dt);
    aircraftPhysics.update(state, dt);
    aircraftTransformation.update(state, dt);
    aircraftCamera.update(state, dt);
    entities.update(state, dt);
}

bool GameData::isStableLanding() {
    // todo numbers should come from config
    const Vector3 angles = QuaternionToEuler(state.orientation.rotation);
    const float pitch = angles.x * RAD2DEG;
    const float roll = angles.z * RAD2DEG;

    constexpr float tolerance = 5.0f;

    const bool wingsLevel = fabsf(roll) < tolerance;
    const bool noseUp = pitch > 0.0f && pitch < 10.0f;

    return wingsLevel && noseUp;
}
