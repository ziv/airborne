#include "Autopilot.h"

void Autopilot::AddWaypoint(const Vector3& position, float targetSpeed) {
    route.push_back({position, targetSpeed});
}

bool Autopilot::IsActive() const {
    return currentWaypointIndex < route.size();
}

Orientation Autopilot::CalculateSteering(const GameCamera& camera, const float currentSpeed, const float deltaTime) {
    Orientation input = {0.0f, 0.0f, 0.0f, currentSpeed, deltaTime};
    if (!IsActive()) return input;

    const Waypoint& target = route[currentWaypointIndex];
    const Vector3 camPos = camera.GetRaylibCamera().position;

    if (Vector3Distance(camPos, target.Position) < arrivalRadius) {
        TraceLog(LOG_INFO, "AUTOPILOT---------: Reached Waypoint %zu!", currentWaypointIndex);
        currentWaypointIndex++;
        return input;
    }

    const Vector3 forward = camera.GetForward();
    const Vector3 right = camera.GetRight();
    const Vector3 up = camera.GetUp();

    // ==========================================
    // 1. Horizontal steering via bank-and-turn
    // ==========================================
    // The physics engine already converts bank angle into yaw via bankInducedYaw
    // in GameCamera::move(). We steer by banking, not by commanding Yaw directly.

    Vector3 flatForward = {forward.x, 0.0f, forward.z};
    if (Vector3Length(flatForward) < 0.001f) flatForward = {up.x, 0.0f, up.z};
    flatForward = Vector3Normalize(flatForward);

    const Vector3 toTarget = {target.Position.x - camPos.x, 0.0f, target.Position.z - camPos.z};
    const float horizDist = Vector3Length(toTarget);

    float desiredBank = 0.0f;

    if (horizDist > 0.001f) {
        constexpr float maxBankAngle = 0.5f;
        const Vector3 dirToTarget = Vector3Scale(toTarget, 1.0f / horizDist);

        // Signed heading error via cross/dot — positive crossY = target is to our left
        const float crossY = flatForward.x * dirToTarget.z - flatForward.z * dirToTarget.x;
        const float dot = flatForward.x * dirToTarget.x + flatForward.z * dirToTarget.z;
        const float turnAngle = atan2f(crossY, dot);

        // Desired bank proportional to turn needed (same sign: left target → left bank)
        desiredBank = Clamp(turnAngle * 1.5f, -maxBankAngle, maxBankAngle);
    }

    // Current bank: right.y > 0 = left bank, right.y < 0 = right bank
    const float currentBank = asinf(Clamp(right.y, -1.0f, 1.0f));

    // Roll to reach desired bank (negative Roll input → left bank in our engine)
    const float bankError = desiredBank - currentBank;
    input.Roll = Clamp(-bankError * 3.0f, -2.0f, 2.0f) * deltaTime;

    // ==========================================
    // 2. Vertical steering (pitch)
    // ==========================================
    // In our engine positive Pitch = nose UP:
    //   QuaternionFromAxisAngle(WorldRight(-1,0,0), +angle) rotates
    //   WorldForward(0,0,1) toward +Y.
    const float heightDiff = target.Position.y - camPos.y;
    const float targetPitch = atan2f(heightDiff, fmaxf(horizDist, 1.0f));
    const float currentPitch = asinf(Clamp(forward.y, -1.0f, 1.0f));
    const float pitchError = targetPitch - currentPitch;

    input.Pitch = Clamp(pitchError * 2.0f, -1.5f, 1.5f) * deltaTime;

    // ==========================================
    // 3. Speed management
    // ==========================================
    const float speedDiff = target.TargetSpeed - currentSpeed;
    if (speedDiff > 5.0f) {
        input.Speed += 20.0f * deltaTime;
    } else if (speedDiff < -5.0f) {
        input.Speed -= 20.0f * deltaTime;
    }

    return input;
}