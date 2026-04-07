#include "AircraftCamera.h"
#include "../primitives/Constants.h"
#include "raymath.h"

AircraftCamera::AircraftCamera(const AppConfig &config) : pilotTilt(config.get<float>("/pilot/tilt")) {
    camera.up = GamePhysics::WorldUp;
    camera.fovy = config.get<float>("/pilot/fov");
    camera.projection = CAMERA_PERSPECTIVE;
}

void AircraftCamera::update(const AircraftState &state, const float dt) {
    camera.position = state.position;
    const Quaternion qTilt = QuaternionFromAxisAngle(state.orientation.right, -pilotTilt);
    camera.target = Vector3Add(camera.position, Vector3RotateByQuaternion(state.orientation.forward, qTilt));
    camera.up = Vector3RotateByQuaternion(state.orientation.up, qTilt);

    if (IsKeyDown(KEY_F1)) {
        // 1. קובעים את המרחקים הרצויים מהמטוס (נניח למבט מהצד הימני)
        float rightDistance = -100.0f; // 50 מטר ימינה
        float upDistance = 0.0f;     // 5 מטר למעלה
        float forwardDistance = -100.0f; // 0 כדי להיות בדיוק בקו אחד מהצד

        // 2. מכפילים את וקטורי הכיוון של המטוס במרחקים שלנו (Scaling)
        Vector3 offsetRight = Vector3Scale(state.orientation.right, rightDistance);
        Vector3 offsetUp = Vector3Scale(state.orientation.up, upDistance);
        Vector3 offsetForward = Vector3Scale(state.orientation.forward, forwardDistance);

        Vector3 totalOffset = Vector3Add(offsetRight, Vector3Add(offsetUp, offsetForward));

        camera.position = Vector3Add(state.position, totalOffset);
        camera.target = state.position;
        camera.up = state.orientation.up;
    }
}