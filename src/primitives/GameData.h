#pragma once
#include "raylib.h"

enum PlaneState {
    Ground,
    Takeoff,
    Flying,
    Landing,
    Crushing,
    Crushed
};

struct PilotControls {
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Roll = 0.0f;
    float Throttle = 0.0f;
};

class GameData {
    // the pilot view
    Camera camera = {0};

    // the position/direction airplane in 3d space
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};

public:
    // pilot field of view (deg)
    static constexpr float FieldOfView = 85.0f;

    // the direction the pilot look at the world (a little bit down)
    static constexpr float TiltDown = 0.45f;

    PlaneState planeState = PlaneState::Ground;
    PilotControls controls = {};
    float velocity = 0.0f;
    float deltaTime = 0.0f;
};
