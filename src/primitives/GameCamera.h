#pragma once
#include "raylib.h"
#include "raymath.h"
#include "../Constants.h"

struct Orientation {
    float Pitch = 0.0f;
    float Yaw = 0.0f;
    float Roll = 0.0f;
    float Speed = 0.0f;
    float DeltaTime = 0.0f;
};

class GameCamera {
    Camera camera = {0};
    Quaternion rotation = {0.0f, 0.0f, 0.0f, 1.0f};

    void placeCamera(const Orientation &orientation);

public:
    /**
     * The direction we look at the world (a little bit down)
     */
    static constexpr float TiltDown = 0.45f;

    /**
     * Pilot field of view (deg)
     */
    static constexpr float FieldOfView = 85.0f;

    GameCamera();

    [[nodiscard]] const Camera &GetRaylibCamera() const { return camera; }
    [[nodiscard]] Vector3 GetForward() const { return Vector3RotateByQuaternion(GamePhysics::WorldForward, rotation); }
    [[nodiscard]] Vector3 GetUp() const { return Vector3RotateByQuaternion(GamePhysics::WorldUp, rotation); }
    [[nodiscard]] Vector3 GetRight() const { return Vector3RotateByQuaternion(GamePhysics::WorldRight, rotation); }

    /**
     * Initial place of the plane
     * @param position
     * @param lookAt
     * @param up
     */
    void place(const Vector3 &position, const Vector3 &lookAt, const Vector3 &up);

    /**
     * Moving the plane by the orientation
     * @param orientation
     */
    void move(const Orientation &orientation);

    /**
     * Bringing the plane to horizontal flight
     * @param orientation
     */
    void levelOut(const Orientation &orientation);
};
