#pragma once
#include "AppConfig.h"
#include "raylib.h"
#include "raymath.h"

enum PlaneState {
    Ground,
    Takeoff,
    Flying,
    Landing,
    Crushing,
    Crushed
};

enum GearState {
    Close,
    Opening,
    Opened,
    Closing
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

    Vector3 forward{0};
    Vector3 up{0};
    Vector3 right{0};

    AppConfig config;

    void recalcVectors();

public:
    explicit GameData(AppConfig &config);

    PilotControls controls = {};
    PlaneState planeState = PlaneState::Ground;
    GearState gearState = GearState::Opened;

    Vector3 velocity = {0.0f, 0.0f, 0.0f};
    float throttle = 0.0f;
    float deltaTime = 0.0f;
    bool autoPiloting = false;
    bool breaks = false;

    void Update();

    float Tick();

    void ToggleAutopilot();

    void ToggleBreaks();

    void SetPosition(const Vector3 &position);

    PilotControls ResetControls();

    // todo is the camera by ref....?
    [[nodiscard]] Quaternion GetRotation() const { return rotation; }
    [[nodiscard]] Camera GetCamera() const { return camera; }
    [[nodiscard]] Vector3 GetPosition() const { return camera.position; }
    [[nodiscard]] Vector3 GetForward() const { return forward; }
    [[nodiscard]] Vector3 GetUp() const { return up; }
    [[nodiscard]] Vector3 GetRight() const { return right; }
    [[nodiscard]] float Speed() const { return Vector3Length(velocity); }
};
