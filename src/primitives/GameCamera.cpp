#include "GameCamera.h"
#include "raymath.h"
#include "Utils.h"

GameCamera::GameCamera() {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = FieldOfView;
    camera.projection = CAMERA_PERSPECTIVE;
}

void GameCamera::Place(const Vector3 &position, const Vector3 &lookAt, const Vector3 &up) {
    camera.position = position;
    camera.target = lookAt;
    camera.up = up;
    rotation = QuaternionIdentity(); // todo should be built from lookAt and up?!
}

void GameCamera::Move(const Orientation &orientation) {
    // extract local vertices from local quaternion
    const Vector3 localForward = GetForward();
    const Vector3 localUp = GetUp();
    const Vector3 localRight = GetRight();

    // roll effect (some "physics")
    // todo missing speed as dependant of climbing or diving
    const float bankInducedYaw = orientation.Speed == 0 ? 0 : localRight.y * .2f * orientation.DeltaTime;
    const float liftLossPitch = orientation.Speed == 0 ? 0 : (1.0f - localUp.y) * 0.1f * orientation.DeltaTime;

    // apply the changes
    const Quaternion qPitch = QuaternionFromAxisAngle(localRight, orientation.Pitch + liftLossPitch);
    const Quaternion qYaw = QuaternionFromAxisAngle(localUp, orientation.Yaw + bankInducedYaw);
    const Quaternion qRoll = QuaternionFromAxisAngle(localForward, orientation.Roll);

    // all of them together (the order is important!)
    const Quaternion qDelta = QuaternionMultiply(qYaw, QuaternionMultiply(qPitch, qRoll));

    // update and normalize
    rotation = QuaternionNormalize(QuaternionMultiply(qDelta, rotation));

    PlaceCamera(orientation, localForward, localUp, localRight);
}

void GameCamera::LevelOut(const Orientation &orientation) {
    const Vector3 localForward = GetForward();
    const Vector3 localUp = GetUp();
    const Vector3 localRight = GetRight();
    const Vector3 flatForward = GetFlatForward(localForward, localUp);

    const float targetYaw = atan2f(flatForward.x, flatForward.z);
    const Quaternion targetRotation = QuaternionFromAxisAngle(GamePhysics::WorldUp, targetYaw);

    rotation = QuaternionNormalize(QuaternionSlerp(rotation, targetRotation,
                                                   GameConfig::AUTO_LEVEL_SPEED * orientation.DeltaTime));

    PlaceCamera(orientation, localForward, localUp, localRight);
}

void GameCamera::PlaceCamera(const Orientation &orientation,
                             const Vector3 &forward,
                             const Vector3 &up,
                             const Vector3 &right) {
    // pilot look a little bit down
    const Quaternion qTilt = QuaternionFromAxisAngle(right, -TiltDown);

    camera.position = Vector3Add(camera.position, Vector3Scale(forward, orientation.Speed * orientation.DeltaTime));
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(forward, qTilt));
    camera.up = Vector3RotateByQuaternion(up, qTilt);
}
