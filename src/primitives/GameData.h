#pragma once
#include "AppConfig.h"
#include "raylib.h"

enum AircraftState {
    Ground,
    Flying,
    Crushed
};

enum GearState {
    Closed,
    Opened
};

struct PilotControls {
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Roll = 0.0f;
    float Throttle = 0.0f;
};

class GameData {
    AppConfig &config;

    // the pilot view
    Camera camera = {0};

    // the position/direction airplane in 3d space
    // and the current velocity
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};
    Vector3 velocity = {0.0f, 0.0f, 0.0f};

    // just another representation of the rotation with R3 vectors
    Vector3 forward{0};
    Vector3 up{0};
    Vector3 right{0};


    void recalcVectors();

    void applyState();

    void applyForces();

    void applyPosition();

    [[nodiscard]] bool isStableLanding() const;

public:
    explicit GameData(AppConfig &config);
    ~GameData();

    PilotControls controls = {};
    AircraftState planeState = AircraftState::Ground;
    GearState gearState = GearState::Opened;

    float throttle = 0.0f;
    float deltaTime = 0.0f;
    float speed = 0.0f;
    bool autoPiloting = false;
    bool breaks = false;
    bool gear = true;
    bool paused = false;

    // display
    float width = 0.0f;
    float height = 0.0f;

    float tick();

    void update();

    void toggleAutopilot();

    void toggleBreaks();

    void resetControls();

    void setPosition(const Vector3 &position);

    [[nodiscard]] Quaternion getRotation() const { return rotation; }
    [[nodiscard]] Camera getCamera() const { return camera; }
    [[nodiscard]] Vector3 getPosition() const { return camera.position; }
    [[nodiscard]] Vector3 getForward() const { return forward; }
    [[nodiscard]] Vector3 getUp() const { return up; }
    [[nodiscard]] Vector3 getRight() const { return right; }
};
