#pragma once
#include "raylib.h"
#include "raymath.h"

struct Orientation {
    float Pitch;
    float Yaw;
    float Roll;
    float Speed;
};

class GameCamera {
    Camera camera = {0};
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};

public:
    const Vector3 worldForward = {0.0f, 0.0f, 1.0f};
    const Vector3 worldUp = {0.0f, 1.0f, 0.0f};
    const Vector3 worldRight = {-1.0f, 0.0f, 0.0f};

    const float TiltDown = 0.45f;
    const float FieldOfView = 85.0f;

    GameCamera();

    [[nodiscard]] const Camera &GetRaylibCamera() const { return camera; }
    [[nodiscard]] Vector3 GetForward() const { return Vector3RotateByQuaternion(worldForward, rotation); }
    [[nodiscard]] Vector3 GetUp() const { return Vector3RotateByQuaternion(worldUp, rotation); }
    [[nodiscard]] Vector3 GetRight() const { return Vector3RotateByQuaternion(worldRight, rotation); }

    void place(const Vector3 &position, const Vector3 &lookAt, const Vector3 &up);

    void move(const Orientation &orientation);

    void levelOut(float levelingSpeed, float flightSpeed);
};
