#include "GameCamera.h"
#include "raymath.h"

GameCamera::GameCamera() {
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = FieldOfView;
    camera.projection = CAMERA_PERSPECTIVE;
}

void GameCamera::place(const Vector3 &position, const Vector3 &lookAt, const Vector3 &up) {
    camera.position = position;
    camera.target = lookAt;
    camera.up = up;
    rotation = QuaternionIdentity();
}

void GameCamera::move(const Orientation &orientation) {
    // extract local vertices from local quaternion
    Vector3 forward = Vector3RotateByQuaternion(worldForward, rotation);
    Vector3 up = Vector3RotateByQuaternion(worldUp, rotation);
    Vector3 right = Vector3RotateByQuaternion(worldRight, rotation);

    // roll effect (some "physics")
    // todo check the effects of those items:
    const float bankInducedYaw = orientation.Speed == 0 ? 0 : right.y * .2f * GetFrameTime();
    const float liftLossPitch = orientation.Speed == 0 ? 0 : (1.0f - up.y) * 0.1f * GetFrameTime();

    // apply the changes
    const Quaternion qPitch = QuaternionFromAxisAngle(right, orientation.Pitch + liftLossPitch);
    const Quaternion qYaw = QuaternionFromAxisAngle(up, orientation.Yaw + bankInducedYaw);
    const Quaternion qRoll = QuaternionFromAxisAngle(forward, orientation.Roll);

    // all of them together
    const Quaternion qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update and normalize
    rotation = QuaternionMultiply(qDelta, rotation);
    rotation = QuaternionNormalize(rotation);

    // the new vertices after the change
    forward = Vector3RotateByQuaternion(worldForward, rotation);
    up = Vector3RotateByQuaternion(worldUp, rotation);
    right = Vector3RotateByQuaternion(worldRight, rotation);

    // place camera
    camera.position = Vector3Add(camera.position, Vector3Scale(forward, orientation.Speed * GetFrameTime()));

    // the pilot view a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(right, -TiltDown);
    const Vector3 pilotLookDir = Vector3RotateByQuaternion(forward, qTilt);
    const Vector3 pilotUpDir = Vector3RotateByQuaternion(up, qTilt);

    camera.target = Vector3Add(camera.position, pilotLookDir);
    camera.up = pilotUpDir;
}

void GameCamera::levelOut(const float levelingSpeed, const float flightSpeed) {
    const Vector3 currentForward = Vector3RotateByQuaternion(worldForward, rotation);
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};

    // end state
    if (Vector3Length(flatForward) < 0.001f) {
        const Vector3 currentUp = Vector3RotateByQuaternion({0.0f, 1.0f, 0.0f}, rotation);
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }

    const float targetYaw = atan2f(flatForward.x, flatForward.z);
    const Quaternion targetRotation = QuaternionFromAxisAngle({0.0f, 1.0f, 0.0f}, targetYaw);

    rotation = QuaternionSlerp(rotation, targetRotation, levelingSpeed * GetFrameTime());
    rotation = QuaternionNormalize(rotation);

    const Vector3 forward = Vector3RotateByQuaternion(worldForward, rotation);
    const Vector3 up = Vector3RotateByQuaternion(worldUp, rotation);
    const Vector3 right = Vector3RotateByQuaternion(worldRight, rotation);

    camera.position = Vector3Add(camera.position, Vector3Scale(forward, flightSpeed * GetFrameTime()));

    Quaternion qTilt = QuaternionFromAxisAngle(right, -TiltDown);
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(forward, qTilt));
    camera.up = Vector3RotateByQuaternion(up, qTilt);
}
