#pragma once
#include "raylib.h"

struct Orientation {
    float Pitch;
    float Yaw;
    float Roll;
    float Speed;
};



class GameCamera {
    float TiltDown = 0.45f;
    float FieldOfView = 85.0f;

    Camera camera = {0};
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};

public:
    GameCamera();

    [[nodiscard]] const Camera& GetRaylibCamera() const { return camera; }

    void place(const Vector3 &position, const Vector3 &lookAt, const Vector3 &up);

    void move(const Orientation &orientation);
};
