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
    // world coordinates
    constexpr Vector3 worldForward = {0.0f, 0.0f, 1.0f};
    constexpr Vector3 worldUp      = {0.0f, 1.0f, 0.0f};
    constexpr Vector3 worldRight   = {-1.0f, 0.0f, 0.0f};

    // extract local vertices from local quaternion
    Vector3 forward = Vector3RotateByQuaternion(worldForward, rotation);
    Vector3 up      = Vector3RotateByQuaternion(worldUp, rotation);
    Vector3 right   = Vector3RotateByQuaternion(worldRight, rotation);

    // roll effect (some "physics")
    // todo check the commented lines in real world rendered, use - and +
    float bankInducedYaw = -right.y * 1.0f * GetFrameTime();
    // float liftLossPitch = (1.0f - up.y) * 0.5f * GetFrameTime();

    // apply the changes
    // const Quaternion qPitch = QuaternionFromAxisAngle(right, orientation.Pitch + liftLossPitch);
    const Quaternion qPitch = QuaternionFromAxisAngle(right, orientation.Pitch);
    const Quaternion qYaw   = QuaternionFromAxisAngle(up, orientation.Yaw - bankInducedYaw);
    const Quaternion qRoll  = QuaternionFromAxisAngle(forward, orientation.Roll);

    // all of them together
    const Quaternion qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update and normalize
    rotation = QuaternionMultiply(qDelta, rotation);
    rotation = QuaternionNormalize(rotation);

    // the new vertices after the change
    forward = Vector3RotateByQuaternion(worldForward, rotation);
    up      = Vector3RotateByQuaternion(worldUp, rotation);
    right   = Vector3RotateByQuaternion(worldRight, rotation);

    // place camera
    camera.position = Vector3Add(camera.position, Vector3Scale(forward, orientation.Speed * GetFrameTime()));

    // the pilot view a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(right, -TiltDown);
    const Vector3 pilotLookDir = Vector3RotateByQuaternion(forward, qTilt);
    const Vector3 pilotUpDir   = Vector3RotateByQuaternion(up, qTilt);

    camera.target = Vector3Add(camera.position, pilotLookDir);
    camera.up = pilotUpDir;
}
