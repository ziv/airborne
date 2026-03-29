#include "Autopilot.h"
#include "Utils.h"

Autopilot::Autopilot(const float maxBankAngle,
                     const float maxPullRatio,
                     const float speedRatio) : maxBankAngle(maxBankAngle),
                                               maxPullRatio(maxPullRatio),
                                               speedRatio(speedRatio) {
}

void Autopilot::AddWaypoint(const Vector3 &position, const float targetSpeed, const float arrivalRadius) {
    route.push_back({position, targetSpeed, arrivalRadius});
}

bool Autopilot::IsActive() const {
    return currentWaypointIndex < route.size() && active;
}

PilotControls Autopilot::AutoSteer(GameData &game) {
    return Steer(game.GetPosition(), game.GetForward(), game.GetRight(), game.GetUp(), game.deltaTime, game.Speed());
}

PilotControls Autopilot::Steer(const Vector3 &position,
                               const Vector3 &forward,
                               const Vector3 &right,
                               const Vector3 &up,
                               const float deltaTime,
                               const float speed) {
    PilotControls input = {0.0f, 0.0f, 0.0f, 0.0f};

    // nothing changed
    if (!IsActive()) return input;

    // current target
    const auto target = route[currentWaypointIndex];

    // did we reach the target?
    if (Vector3Distance(target.Position, position) < target.ArrivalRadius) {
        TraceLog(LOG_INFO, "[Autopilot] reached waypoint %zu!", currentWaypointIndex);
        // move to the next waypoint if any, let the next tick
        // to handle to steering
        currentWaypointIndex++;
        return input;
    }

    // this is basically a "BANK NAD ROLL" maneuver

    // normalized line of sight to the target
    const auto dirToTarget = Vector3Normalize(Vector3Subtract(target.Position, position));

    // normalize flat forward vectors
    const auto flatForward = GetFlatForward(forward, up);
    const auto flatDirToTarget = GetFlatForward(dirToTarget, up);

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
    const float targetBank = Clamp(headingError * 1.5f, -maxBankAngleRad, maxBankAngleRad);

    // the required roll
    const float currentBank = atan2f(right.y, up.y);
    float rollError = currentBank - targetBank;

    // 360 deg guard
    while (rollError > PI) rollError -= 2.0f * PI;
    while (rollError < -PI) rollError += 2.0f * PI;

    // todo why 2.0?
    input.Roll = rollError * 2.0f * deltaTime;

    // vertical distance
    const Vector2 sourcePosXZ = {position.x, position.z};
    const Vector2 targetPosXZ = {target.Position.x, target.Position.z};
    const float distanceXZ = Vector2Distance(sourcePosXZ, targetPosXZ);
    const float heightDiff = target.Position.y - position.y;

    // the pitch angle with guard
    const float targetPitchAngle = (distanceXZ > 0.1f) ? atan2f(heightDiff, distanceXZ) : 0.0f;

    // plane pitch angle
    const float currentPitchAngle = asinf(Clamp(forward.y, -1.0f, 1.0f));

    // height error angle
    const float pitchError = targetPitchAngle - currentPitchAngle;

    // the pull with aggression factor
    const float turnPull = fabsf(currentBank) * maxPullRatio;

    // pitch results
    const float desiredPitchInput = pitchError + turnPull;

    // limiting the result to not "break" the stick
    input.Pitch = Clamp(desiredPitchInput, -1.0f, 1.0f) * deltaTime;

    // update throttle direction
    if (speed < target.TargetSpeed) input.Throttle = speedRatio * deltaTime;
    else if (speed > target.TargetSpeed) input.Throttle = -speedRatio * deltaTime;

    input.Yaw = 0.0f;

    return input;
}
