#pragma once
#include "raymath.h"

struct Player {
};

struct Parent {
    entt::entity parentId;
};

struct Child {
    entt::entity childId;
};

struct View3D {
    float fov;
    float tilt;
};

struct Position3D {
    Vector3 position;
};

struct Position2D {
    Vector2 position;
};

struct Dimensions3D {
    Vector3 size;
};

struct Dimensions2D {
    Vector2 size;
};

struct Velocity {
    Vector3 velocity;
};

struct Rotation {
    Quaternion rotation;
};

struct Orientation {
    Vector3 forward;
    Vector3 up;
    Vector3 right;
};

struct RigidBody {
    float mass;
    Vector3 velocity;
    Vector3 acceleration;
};
