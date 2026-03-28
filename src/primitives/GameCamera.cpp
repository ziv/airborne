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

    // don't work for some reason
    // const Vector3 fwd = Vector3Normalize(Vector3Subtract(lookAt, position));
    // const Vector3 right = Vector3Normalize(Vector3CrossProduct(up, fwd));
    // const Vector3 left = {-right.x, -right.y, -right.z};
    // const Vector3 trueUp = up; // Vector3CrossProduct(fwd, right);
    // rotation matrix
    // Matrix rotMatrix = { 0 };
    // rotMatrix.m0 = left.x;   rotMatrix.m4 = trueUp.x;   rotMatrix.m8 = fwd.x;    rotMatrix.m12 = 0.0f;
    // rotMatrix.m1 = left.y;   rotMatrix.m5 = trueUp.y;   rotMatrix.m9 = fwd.y;    rotMatrix.m13 = 0.0f;
    // rotMatrix.m2 = left.z;   rotMatrix.m6 = trueUp.z;   rotMatrix.m10 = fwd.z;   rotMatrix.m14 = 0.0f;
    // rotMatrix.m3 = 0.0f;     rotMatrix.m7 = 0.0f;       rotMatrix.m11 = 0.0f;    rotMatrix.m15 = 1.0f;
    // rotation = QuaternionFromMatrix(rotMatrix);

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
