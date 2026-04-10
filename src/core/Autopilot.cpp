/**
 * @file Autopilot.cpp
 * @brief Waypoint-following autopilot — bank-and-pull steering implementation.
 */
#include "Autopilot.h"
#include "../primitives/Utils.h"

Autopilot::Autopilot(const AppConfig &config, const Scenario &scenario) : maxBankAngle(config.get<float>("/autoPilot/maxBankAngle")),
                                                                          maxPullRatio(config.get<float>("/autoPilot/pullRatio")),
                                                                          speedRatio(config.get<float>("/autoPilot/speedRatio")) {
    // todo currently take all items and put them without any order or filter
    // for (const auto &def: scenario.entityDefinitions) {
    //     const auto pos = def.position + (Vector3){0.0f, 2000.0f, 0.0f};
    //     addWaypoint(pos, 400.0f, 200.0f);
    //     TraceLog(LOG_INFO, "[Autopilot] target %f,%f,%f", pos.x, pos.y, pos.z);
    // }
    addWaypoint("kineret", (Vector3){87000.0f, 3000.0f, 86000.0f}, 400.0f, 200.0f);
    addWaypoint("yesod", (Vector3){89000.0f, 4000.0f, 50000.0f}, 400.0f, 200.0f);
    addWaypoint("bint gbel", (Vector3){73000.0f, 5000.0f, 42000.0f}, 400.0f, 200.0f);
    addWaypoint("tyre", (Vector3){52000.0f, 2000.0f, 26000.0f}, 400.0f, 200.0f);
}

void Autopilot::addWaypoint(const std::string &name, const Vector3 &position, const float targetSpeed, const float arrivalRadius) {
    route.push_back((Waypoint){name, position, targetSpeed, arrivalRadius});
}

void Autopilot::steer(AircraftState &state, const float deltaTime) {
    state.controls.pitch = 0.0f;
    state.controls.roll = 0.0f;
    state.controls.yaw = 0.0f;

    // current target
    if (currentWaypointIndex >= route.size()) {
        TraceLog(LOG_INFO, "[Autopilot] route complete!");
        return;
    }
    const auto target = route[currentWaypointIndex];

    // current absolute position (so we can compare positions)
    const auto position = state.pos();

    // did we reach the target?
    if (Vector3Distance(target.position, position) < target.arrivalRadius) {
        TraceLog(LOG_INFO, "[Autopilot] reached waypoint %zu!", currentWaypointIndex);
        // move to the next waypoint if any, let the next tick
        // to handle to steering
        currentWaypointIndex += 1;
        return;
    }

    // --- Bank-and-pull maneuver ---
    // 1. Compute heading error on the XZ plane
    // 2. Roll to align the lift vector with the desired turn direction
    // 3. Pitch toward the target altitude
    // 4. Adjust throttle toward the target speed

    // normalized line of sight to the target
    const auto dirToTarget = Vector3Normalize(Vector3Subtract(target.position, position));

    // normalize flat forward vectors
    const auto flatForward = GetFlatForward(state.orientation.forward, state.orientation.up);
    const auto flatDirToTarget = GetFlatForward(dirToTarget, state.orientation.up);

    // angle in rad (on XZ space)
    const float currentHeading = atan2f(flatForward.x, flatForward.z);
    const float targetHeading = atan2f(flatDirToTarget.x, flatDirToTarget.z);

    // angle diff
    float headingError = targetHeading - currentHeading;

    // 360 deg guard
    while (headingError > PI) headingError -= 2.0f * PI;
    while (headingError < -PI) headingError += 2.0f * PI;

    // our target angle is limited by autopilot restrictions
    const float maxBankAngleRad = maxBankAngle * PI / 180.0f;
    const float targetBank = Clamp(headingError, -maxBankAngleRad, maxBankAngleRad);

    // the required roll
    const float currentBank = atan2f(state.orientation.right.y, state.orientation.up.y);
    float rollError = currentBank - targetBank;

    // 360 deg guard
    while (rollError > PI) rollError -= 2.0f * PI;
    while (rollError < -PI) rollError += 2.0f * PI;

    // todo why 2.0?
    state.controls.roll = rollError * deltaTime;

    // vertical distance
    const Vector2 sourcePosXZ = {position.x, position.z};
    const Vector2 targetPosXZ = {target.position.x, target.position.z};
    const float distanceXZ = Vector2Distance(sourcePosXZ, targetPosXZ);
    const float heightDiff = target.position.y - position.y;

    // the pitch angle with guard
    const float targetPitchAngle = (distanceXZ > 0.1f) ? atan2f(heightDiff, distanceXZ) : 0.0f;

    // plane pitch angle
    const float currentPitchAngle = asinf(Clamp(state.orientation.forward.y, -1.0f, 1.0f));

    // height error angle
    const float pitchError = targetPitchAngle - currentPitchAngle;

    // the pull with aggression factor
    const float turnPull = fabsf(targetBank) * maxPullRatio;

    // pitch results
    const float desiredPitchInput = pitchError + turnPull;

    // limiting the result to not "break" the stick
    state.controls.pitch = Clamp(desiredPitchInput, -1.0f, 1.0f) * deltaTime;

    // update throttle direction
    if (state.forces.speed < target.targetSpeed) state.controls.throttle += speedRatio * deltaTime;
    else if (state.forces.speed > target.targetSpeed) state.controls.throttle -= speedRatio * deltaTime;

    state.controls.throttle = Clamp(state.controls.throttle, 0.0f, 1.0f);

    state.controls.yaw = 0.0f;
}


bool Autopilot::isActive() const {
    return currentWaypointIndex < route.size() && active;
}

void Autopilot::toggle() {
    active = !active;
    if (active) {
        TraceLog(LOG_INFO, "[Autopilot] activated, heading to waypoint %zu: %s", currentWaypointIndex, route[currentWaypointIndex].name.c_str());
    }
}
