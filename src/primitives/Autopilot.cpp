#include "Autopilot.h"

void Autopilot::AddWaypoint(const Vector3 &position, float targetSpeed) {
    route.push_back({position, targetSpeed});
}

bool Autopilot::IsActive() const {
    return currentWaypointIndex < route.size();
}

Orientation Autopilot::CalculateSteering(const Vector3 &position,
                                         const Vector3 &forward,
                                         const Vector3 &up,
                                         const Vector3 &right,
                                         const float currentSpeed,
                                         const float deltaTime) {
    Orientation input = {0.0f, 0.0f, 0.0f, currentSpeed, deltaTime};
    if (!IsActive()) return input;

    const Waypoint target = route[currentWaypointIndex];

    if (Vector3Distance(target.Position, position) < arrivalRadius) {
        TraceLog(LOG_INFO, "AUTOPILOT---------: Reached Waypoint %zu!", currentWaypointIndex);
        currentWaypointIndex++;
        return input;
    }

    // normalized line of sight to the target
    const Vector3 dirToTarget = Vector3Normalize(Vector3Subtract(target.Position, position));

    // normalize flat vectors
    const Vector3 flatForward = Vector3Normalize({forward.x, 0.0f, forward.z});
    const Vector3 flatDirToTarget = Vector3Normalize({dirToTarget.x, 0.0f, dirToTarget.z});

    // angle in rad (on XZ space)
    const float currentHeading = atan2f(flatForward.x, flatForward.z);
    const float targetHeading = atan2f(flatDirToTarget.x, flatDirToTarget.z);

    // angle diff
    float headingError = targetHeading - currentHeading;

    // 360 deg guard
    while (headingError > PI) headingError -= 2.0f * PI;
    while (headingError < -PI) headingError += 2.0f * PI;

    // our target angle limited
    constexpr float maxBankAngle = GameConfig::AUTO_PILOT_MAX_BANK_ANGLE * PI / 180.0f;
    const float targetBank = Clamp(headingError * 1.5f, -maxBankAngle, maxBankAngle);

    const float currentBank = atan2f(right.y, up.y);
    float rollError = currentBank - targetBank;

    // 360 deg guard
    while (rollError > PI) rollError -= 2.0f * PI;
    while (rollError < -PI) rollError += 2.0f * PI;

    input.Roll = rollError * 2.0f * GetFrameTime();

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
    const float turnPull = fabsf(currentBank) * GameConfig::AUTO_PILOT_PULL_RATIO;

    // pitch results
    const float desiredPitchInput = pitchError + turnPull;

    // limiting the result to not "break" the stick
    input.Pitch = Clamp(desiredPitchInput, -1.0f, 1.0f) * deltaTime;

    if (currentSpeed < target.TargetSpeed) input.Speed += 5 * deltaTime;
    else if (currentSpeed > target.TargetSpeed) input.Speed -= 5 * deltaTime;

    input.Yaw = 0.0f;

    return input;
}
