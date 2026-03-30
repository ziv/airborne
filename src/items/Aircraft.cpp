#include "Aircraft.h"

#include "../primitives/Constants.h"
#include "../primitives/GameData.h"

Aircraft::Aircraft(const std::string_view name,
                   const std::string_view modelPath) : name(name),
                                                       model(LoadModel(modelPath.data())) {
}

// void Aircraft::update(const GameData &gameData) {
//     auto forward = Vector3RotateByQuaternion(GamePhysics::WorldForward, rotation);
//     auto up = Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation);
//     auto right = Vector3RotateByQuaternion(GamePhysics::WorldRight, rotation);
//
//
// }
