#include "Aircraft.h"
#include "raymath.h"
#include "../primitives/Utils.h"


Aircraft::Aircraft(): aircraft(LoadModel("res/f_15_c_pixy.glb")) {
    // waypoints.push_back((AircraftWaypoint){{100.0, 1000.0, 100.0}, 50.0, 10.0});
    // waypoints.push_back((AircraftWaypoint){{500.0, 1000.0, 500.0}, 50.0, 10.0});
    // waypoints.push_back((AircraftWaypoint){{10000.0, 1000.0, 10000.0}, 20.0, 10.0});
    // waypoints.push_back((AircraftWaypoint){{500.0, 1000.0, 10000.0}, 20.0, 10.0});
}


void Aircraft::draw(const AircraftState &state) {
    const Vector3 &position = {state.position.x, state.position.y - 20, state.position.z};
    aircraft->transform = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), QuaternionToMatrix(state.orientation.rotation));
    DrawModel(aircraft, position, 0.001f, RED);
}


// void Aircraft::update(const AircraftState &state, float dt) {
//     // steer(dt);
//     // simple aircraft AI
//     // a. if there are no more missiles, change status to flee
//     // b. if a missile is shot at you, change status to evade
//
//     //
//     // 1. flee mode
//     //    1.1. navigate to the closest friendly airbase
//     //    1.2. when arrive to airbase, reload missiles and change status to patrol
//     //
//     // 2. evade mode
//     //    1.1. remember last status
//     //    1.2. try to evade maneuver (turn towards the threat) (1.5 seconds)
//     //    1.3. if threat still exist fire chaf/flare
//     //    1.4. change status back
//     //
//     // 1. patrol mode
//     //    1.1. follow the autopilot waypoints
//     //    1.2. when reach the last, start from beginning
//     //    1.3. if user in range and there are missiles in stock, change status to engaged
//     // 2. engaged mode
//     //    2.1. steer to put the user on target
//     //    2.2. when user on target for 2 seconds (lock) and there are missiles - fire missile
//     //
// }

    // Vector3 defaultForward = GamePhysics::WorldForward;
    // Vector3 axis = Vector3CrossProduct(defaultForward, forward);
    // if (Vector3Length(axis) < 0.001f) {
    //     axis = {0.0f, 1.0f, 0.0f};
    // } else {
    //     axis = Vector3Normalize(axis);
    // }
    // float angle = acosf(Vector3DotProduct(defaultForward, forward)) * RAD2DEG;
    //
    // // DrawModelEx(model, position, axis, angle, {25.0f, 25.0f, 25.0f}, WHITE);
    // // TraceLog(LOG_INFO, TextFormat("%f %f", position.x, position.z));
    // DrawCube(position, 100.0f, 100.0f, 100.0f, RED);
    // DrawCubeWires(position, 14.0f, 5.0f, 16.7f, WHITE);
// }
//
// void Aircraft::steer(float dt) {
//     // const auto target = waypoints[currentWaypointIndex];
//     //
//     // if (Vector3Distance(target.position, position) < target.arrivalRadius) {
//     //     TraceLog(LOG_INFO, "[Autopilot] reached waypoint %zu!", currentWaypointIndex);
//     //     currentWaypointIndex++;
//     //     if (currentWaypointIndex >= waypoints.size()) {
//     //         currentWaypointIndex = 0;
//     //     }
//     //     return;
//     // }
//     //
//     // const Vector3 targetDir = Vector3Normalize(Vector3Subtract(target.position, position));
//     //
//     // if (const float angleToTarget = acosf(Vector3DotProduct(forward, targetDir)); angleToTarget > 0.001f) {
//     //     const Vector3 rotationAxis = Vector3Normalize(Vector3CrossProduct(forward, targetDir));
//     //     const float turnStep = maxTurnRate * dt;
//     //     const float actualTurn = (angleToTarget < turnStep) ? angleToTarget : turnStep;
//     //     const Quaternion q = QuaternionFromAxisAngle(rotationAxis, actualTurn);
//     //     forward = Vector3RotateByQuaternion(forward, q);
//     //     forward = Vector3Normalize(forward);
//     // }
//     // const Vector3 velocity = Vector3Scale(forward, target.speed * dt);
//     // position = Vector3Add(position, velocity);
// }
