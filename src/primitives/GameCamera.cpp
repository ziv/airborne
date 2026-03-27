#include "GameCamera.h"
#include "raymath.h"

GameCamera::GameCamera() {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = FieldOfView;
    camera.projection = CAMERA_PERSPECTIVE;
}

void GameCamera::place(const Vector3 &position, const Vector3 &lookAt, const Vector3 &up) {
    camera.position = position;
    camera.target = lookAt;
    camera.up = up;
    rotation = QuaternionIdentity(); // todo should be built from lookAt and up?!
}

void GameCamera::move(const Orientation &orientation) {
    // extract local vertices from local quaternion
    const Vector3 localUp = GetUp();
    const Vector3 localRight = GetRight();

    // roll effect (some "physics")
    const float bankInducedYaw = orientation.Speed == 0 ? 0 : localRight.y * .2f * orientation.DeltaTime;
    const float liftLossPitch = orientation.Speed == 0 ? 0 : (1.0f - localUp.y) * 0.1f * orientation.DeltaTime;

    // apply the changes
    const Quaternion qPitch = QuaternionFromAxisAngle(localRight, orientation.Pitch + liftLossPitch);
    const Quaternion qYaw = QuaternionFromAxisAngle(localUp, orientation.Yaw + bankInducedYaw);
    const Quaternion qRoll = QuaternionFromAxisAngle(GetForward(), orientation.Roll);

    // all of them together
    const Quaternion qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update and normalize
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));

    placeCamera(orientation);
}

void GameCamera::levelOut(const Orientation &orientation) {
    const Vector3 currentForward = GetForward();
    Vector3 flatForward = {currentForward.x, 0.0f, currentForward.z};

    // edge case guard
    if (Vector3Length(flatForward) < 0.001f) {
        const Vector3 currentUp = Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation);
        flatForward = {currentUp.x, 0.0f, currentUp.z};
    }

    const float targetYaw = atan2f(flatForward.x, flatForward.z);
    const Quaternion targetRotation = QuaternionFromAxisAngle(GamePhysics::WorldUp, targetYaw);

    rotation = QuaternionNormalize(QuaternionSlerp(rotation, targetRotation, GameConfig::AUTO_LEVEL_SPEED * orientation.DeltaTime));

    placeCamera(orientation);
}

void GameCamera::placeCamera(const Orientation &orientation) {
    // pilot look a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(GetRight(), -TiltDown);
    const Vector3 forward = GetForward();

    camera.position = Vector3Add(camera.position, Vector3Scale(forward, orientation.Speed * orientation.DeltaTime));
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(forward, qTilt));
    camera.up = Vector3RotateByQuaternion(GetUp(), qTilt);
}
