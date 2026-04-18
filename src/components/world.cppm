module;
#include <entt/entt.hpp>
#include "../lib/ray.hpp"

export module Components:World;


import RaylibResource;
import Types;
import ResourceManager;

// area we can land on
export struct Landable {
    bool carrier;
};

// tag aircraft as crashed
export struct Crashed {
};

export struct ChromaRender {
    TextureHandle tex;
    ShaderHandle shader;
    Color tint = WHITE;
    int priority = 0;
};


export struct Aircraft {
    // drag
    float cd;
    float inducedDragCoefficient;

    // lift
    float cl;
    float liftSlopeCoefficient;
    float stallAngle;

    // weight
    float weight;

    // agility
    float pitchRatio;
    float rollRatio;
    float yawRatio;
};

export struct AircraftUtils {
    bool brake; // aircraft brakes status
    bool gear; // aircraft landing gear status
};

export struct Engine {
    float thrust;
    float throttle;
};

export struct AircraftControls {
    float roll;
    float pitch;
    float yaw;
    bool brake; // pressed or not
    bool gear; // pressed or not
    float throttle;
    bool afterBurner;
};

// events/states

export struct Grounded {
};

export struct Flying {
};

export struct TouchDown {
};

export struct Autopilot {
};

export struct LandingZone {
    bool isLandingZone;
    bool isCarrier;
    float surfaceY;
};

// specific player/world structures

export struct World {
    entt::resource<ModelResourceLoader> surface;
    entt::resource<ModelResourceLoader> clouds;
    entt::resource<MusicStreamResourceLoader> streams;
};

export struct Identify {
    std::string name;
};

export struct IdentifyType {
    EntityType type{};
};


export struct Heading {
    float heading;
};


// position

export struct Position3D {
    Vector3 pos;
    Vector3 offset;
};

export struct Position2D {
    Vector2 pos;
};

// dimensions

export struct Dimensions3D {
    Vector3 size;
};

export struct Dimensions2D {
    Vector2 size;
};

// movement

export struct LinearVelocity {
    Vector3 velocity;
    float speed;
};

export struct AngularVelocity {
    Vector3 velocity;
};

export struct LinerAcceleration {
    Vector3 acc;
};

export struct AngularAcceleration {
    Vector3 acc;
};

export struct Rotation {
    Quaternion rotation;
};

export struct Orientation {
    Vector3 forward;
    Vector3 up;
    Vector3 right;
};

// units

export struct FriendFoe {
    Faction faction{};
};

export struct Carrier {
};

export struct Forces {
    float thrust;
    float drag;
    float lift;
    float mass;
    Vector3 acceleration;
};
