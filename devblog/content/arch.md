# Scenario & Mission System — Architecture

## Overview

The scenario system brings structured gameplay to Airborne. A **Scenario** is a self-contained combat mission: the player receives a briefing with objectives, flies into hostile territory populated with threats and targets, completes (or fails) objectives, and receives a debrief with scoring. Scenarios are **data-driven** — defined entirely in JSON files following the existing `AppConfig` pattern — so new missions can be authored without code changes.

---

## 1. Scenario Lifecycle

The mission flows through a state machine that extends the existing screen system:

```
┌──────────┐    ┌───────────┐    ┌───────────────┐    ┌──────────┐
│ BRIEFING │───▶│  GAMEPLAY  │───▶│   DEBRIEF     │───▶│MAIN_MENU │
│  Screen  │    │  (active   │    │   Screen      │    │          │
│          │    │   mission) │    │               │    │          │
└──────────┘    └─────┬──────┘    └───────────────┘    └──────────┘
                      │
                      ▼
               Mission failed /
               Player ejected /
               Aircraft destroyed
                      │
                      ▼
               ┌──────────────┐
               │   DEBRIEF    │
               │  (failure)   │
               └──────────────┘
```

### New Screen States

```cpp
enum class ScreenState {
    SPLASH,
    MAIN_MENU,
    BRIEFING,      // NEW — mission briefing & loadout
    GAMEPLAY,      // existing — now mission-aware
    DEBRIEF,       // NEW — results & scoring
    EXIT
};
```

- **BriefingScreen** — displays mission objectives, map preview, threat overview, and weapon loadout selection. Player presses a key to launch.
- **GameplayScreen** — existing screen, now receives a `Scenario` and creates a `MissionManager` that orchestrates the mission during gameplay.
- **DebriefScreen** — shows completed/failed objectives, score breakdown, and flight stats (time, fuel used, weapons expended).

---

## 2. Scenario Definition (JSON)

Scenarios live in `res/scenarios/` as `.jsonc` files. The format:

```jsonc
{
  "id": "strike-bridges-01",
  "name": "Bridge Busters",
  "description": "Destroy two key bridges on the Euphrates to halt enemy supply lines.",
  "difficulty": "medium",          // "easy" | "medium" | "hard" | "ace"
  "theater": "north",              // references a map/terrain set

  // where the player starts
  "start": {
    "position": [2000, 100, 1500],
    "heading": 90,                 // degrees
    "speed": 0,                    // m/s (0 = start on carrier/runway)
    "altitude": 100,               // meters
    "fuel": 3500,                  // kg
    "carrier": true                // start on aircraft carrier
  },

  // available weapon loadout (player picks in briefing)
  "loadout": {
    "slots": 6,
    "available": ["aim9", "aim7", "mk82", "mk84", "agm65", "camera_pod"],
    "default": ["aim9", "aim9", "mk82", "mk82", "mk84", "mk84"]
  },

  // mission objectives — ordered list, can be sequential or parallel
  "objectives": [
    {
      "id": "obj-bridge-north",
      "type": "destroy",
      "target": "bridge-north",     // references entity id
      "label": "Destroy northern bridge",
      "required": true,             // must complete to succeed
      "order": 1                    // display order
    },
    {
      "id": "obj-bridge-south",
      "type": "destroy",
      "target": "bridge-south",
      "label": "Destroy southern bridge",
      "required": true,
      "order": 2
    },
    {
      "id": "obj-recon-depot",
      "type": "photograph",
      "target": "supply-depot",
      "label": "Photograph enemy supply depot",
      "required": false,            // bonus objective
      "order": 3,
      "params": {
        "minAltitude": 300,         // must be below this altitude
        "maxAltitude": 2000,        // must be above this altitude
        "maxDistance": 500,          // meters from target
        "requiredAngle": 30         // degrees — must be looking at target
      }
    },
    {
      "id": "obj-rtb",
      "type": "navigate",
      "target": "home-base",
      "label": "Return to base",
      "required": true,
      "order": 99,
      "params": {
        "arrivalRadius": 2000
      }
    }
  ],

  // all world entities for this scenario
  "entities": [
    // ── Targets ──
    {
      "id": "bridge-north",
      "type": "structure",
      "subtype": "bridge",
      "position": [45000, 0, 32000],
      "heading": 0,
      "health": 100,
      "model": "bridge_01",
      "faction": "enemy"
    },
    {
      "id": "bridge-south",
      "type": "structure",
      "subtype": "bridge",
      "position": [45000, 0, 28000],
      "heading": 0,
      "health": 100,
      "model": "bridge_01",
      "faction": "enemy"
    },
    {
      "id": "supply-depot",
      "type": "structure",
      "subtype": "depot",
      "position": [47000, 0, 30000],
      "heading": 45,
      "health": 200,
      "model": "depot_01",
      "faction": "enemy"
    },

    // ── Air Defenses ──
    {
      "id": "sam-site-alpha",
      "type": "sam",
      "subtype": "sa6",
      "position": [43000, 0, 31000],
      "heading": 0,
      "health": 80,
      "faction": "enemy",
      "params": {
        "detectionRange": 25000,
        "engagementRange": 18000,
        "minAltitude": 50,
        "maxAltitude": 12000,
        "missileSpeed": 900,
        "reloadTime": 8.0,
        "maxMissiles": 4,
        "accuracy": 0.7
      }
    },
    {
      "id": "aaa-battery-01",
      "type": "aaa",
      "subtype": "zsu23",
      "position": [44500, 0, 31500],
      "heading": 0,
      "health": 50,
      "faction": "enemy",
      "params": {
        "detectionRange": 5000,
        "engagementRange": 3000,
        "maxAltitude": 3000,
        "fireRate": 10.0,
        "accuracy": 0.3,
        "burstDuration": 2.0
      }
    },

    // ── Enemy Aircraft ──
    {
      "id": "mig29-patrol-01",
      "type": "aircraft",
      "subtype": "mig29",
      "position": [50000, 5000, 35000],
      "heading": 180,
      "health": 100,
      "faction": "enemy",
      "params": {
        "speed": 250,
        "behavior": "patrol",
        "engageRange": 30000,
        "disengageRange": 50000,
        "weaponRange": 15000,
        "waypoints": [
          [50000, 5000, 35000],
          [40000, 5000, 30000],
          [50000, 5000, 25000]
        ],
        "skill": "veteran"         // "rookie" | "regular" | "veteran" | "ace"
      }
    },
    {
      "id": "mig21-cap-01",
      "type": "aircraft",
      "subtype": "mig21",
      "position": [48000, 4000, 33000],
      "heading": 270,
      "health": 80,
      "faction": "enemy",
      "params": {
        "speed": 200,
        "behavior": "cap",         // combat air patrol
        "engageRange": 20000,
        "weaponRange": 8000,
        "waypoints": [
          [48000, 4000, 33000],
          [44000, 4000, 29000]
        ],
        "skill": "regular"
      }
    },

    // ── Friendly / Neutral ──
    {
      "id": "home-base",
      "type": "airbase",
      "subtype": "carrier",
      "position": [2000, 0, 1500],
      "heading": 90,
      "faction": "friendly",
      "model": "carrier"
    },
    {
      "id": "awacs",
      "type": "aircraft",
      "subtype": "e3",
      "position": [10000, 8000, 10000],
      "heading": 0,
      "faction": "friendly",
      "params": {
        "speed": 150,
        "behavior": "orbit",
        "waypoints": [
          [10000, 8000, 10000],
          [15000, 8000, 15000]
        ]
      }
    }
  ],

  // events triggered by conditions during gameplay
  "triggers": [
    {
      "id": "trg-scramble",
      "type": "zone_enter",
      "params": {
        "zone": [40000, 0, 25000],   // center
        "radius": 15000
      },
      "action": {
        "type": "spawn_entity",
        "entityId": "mig29-scramble-01",
        "entity": {
          "type": "aircraft",
          "subtype": "mig29",
          "position": [55000, 1000, 35000],
          "heading": 240,
          "health": 100,
          "faction": "enemy",
          "params": {
            "speed": 300,
            "behavior": "intercept",
            "skill": "ace"
          }
        }
      },
      "once": true
    },
    {
      "id": "trg-mission-warning",
      "type": "time_elapsed",
      "params": { "seconds": 600 },
      "action": {
        "type": "radio_message",
        "message": "Viper 1, fuel advisory. RTB in 10 mikes."
      },
      "once": true
    },
    {
      "id": "trg-sam-alert",
      "type": "entity_detects_player",
      "params": { "entityId": "sam-site-alpha" },
      "action": {
        "type": "radio_message",
        "message": "WARNING — SAM radar lock detected!"
      },
      "once": false
    }
  ],

  // when is the mission over?
  "completion": {
    "success": "all_required_objectives",   // all required=true objectives done
    "failure": [
      "player_destroyed",
      "player_ejected",
      { "type": "time_exceeded", "seconds": 1800 }
    ]
  },

  // scoring
  "scoring": {
    "objectiveComplete": 1000,
    "bonusObjective": 500,
    "enemyAircraftKill": 250,
    "samDestroyed": 200,
    "aaaDestroyed": 100,
    "timeBonus": true,              // bonus for finishing fast
    "fuelBonus": true,              // bonus for fuel remaining
    "noDamageBonus": 500
  }
}
```

---

## 3. Entity System

### 3.1 Entity Type Hierarchy

All world objects (enemies, targets, friendlies) are **entities** managed by a central registry. They share a common interface but differ in behavior.

```
src/entities/
├── Entity.h              // base struct + EntityId, EntityState, Faction
├── EntityRegistry.h      // central container: spawn, destroy, query by type/faction/range
├── AirEntity.h/.cpp      // enemy/friendly aircraft — extends existing Aircraft class
├── SamSite.h/.cpp        // surface-to-air missile battery
├── AAABattery.h/.cpp     // anti-aircraft artillery
├── GroundTarget.h/.cpp   // static structures: bridges, depots, radars, bunkers
├── NavalEntity.h/.cpp    // ships: carriers, destroyers
└── Airbase.h/.cpp        // friendly/enemy airfields — spawn points & RTB targets
```

### 3.2 Entity Base

```cpp
// src/entities/Entity.h
#pragma once
#include "raylib.h"
#include "../primitives/Types.h"
#include <string>

using EntityId = std::string;

enum class Faction { FRIENDLY, ENEMY, NEUTRAL };

enum class EntityState {
    INACTIVE,      // not yet spawned / out of range
    ACTIVE,        // alive and participating
    DAMAGED,       // still functional but impaired
    DESTROYED,     // dead — wreckage may remain
    DESPAWNED      // removed from world entirely
};

enum class EntityType {
    AIRCRAFT,
    SAM,
    AAA,
    STRUCTURE,     // bridges, depots, radars, bunkers
    NAVAL,
    AIRBASE,
    WAYPOINT       // navigation marker (invisible)
};

struct EntityBase {
    EntityId id;
    EntityType type;
    std::string subtype;          // "mig29", "sa6", "bridge", etc.
    Faction faction = Faction::ENEMY;
    EntityState state = EntityState::ACTIVE;

    Vector3 position = {0, 0, 0};
    float heading = 0.0f;         // degrees
    float health = 100.0f;
    float maxHealth = 100.0f;

    // rendering
    std::string modelId;          // key into a model cache

    [[nodiscard]] bool isAlive() const {
        return state == EntityState::ACTIVE || state == EntityState::DAMAGED;
    }

    [[nodiscard]] bool isEnemy() const { return faction == Faction::ENEMY; }
};
```

### 3.3 Entity Registry

The registry owns all entities and provides fast queries. It is the single source of truth for "what exists in the world."

```cpp
// src/entities/EntityRegistry.h
#pragma once
#include "Entity.h"
#include <vector>
#include <unordered_map>
#include <functional>

class EntityRegistry {
    std::unordered_map<EntityId, size_t> idIndex;  // id → vector index
    std::vector<std::unique_ptr<EntityBase>> entities;

public:
    // lifecycle
    EntityBase* spawn(std::unique_ptr<EntityBase> entity);
    void destroy(const EntityId& id);                      // sets state to DESTROYED
    void despawn(const EntityId& id);                      // removes entirely

    // queries
    EntityBase* get(const EntityId& id);
    std::vector<EntityBase*> getByType(EntityType type);
    std::vector<EntityBase*> getByFaction(Faction faction);
    std::vector<EntityBase*> getInRadius(Vector3 center, float radius);
    std::vector<EntityBase*> getEnemiesInRadius(Vector3 center, float radius);

    // iteration
    void forEach(std::function<void(EntityBase&)> fn);
    void forEachAlive(std::function<void(EntityBase&)> fn);

    // frame update — calls update on all active entities
    void update(const AircraftState& playerState, float dt);

    // draw all visible entities
    void draw(const AircraftState& playerState);
};
```

### 3.4 Air Entity (Enemy / Friendly Aircraft)

Evolves from the existing `Aircraft` class. Adds AI behavior modes, weapons, and threat response.

```cpp
// src/entities/AirEntity.h
#pragma once
#include "Entity.h"
#include "../views/Aircraft.h"   // reuse waypoint/steer logic

enum class AirBehavior {
    PATROL,       // fly waypoints, scan for player
    CAP,          // combat air patrol — orbit and engage if player enters zone
    INTERCEPT,    // beeline toward player
    ENGAGE,       // dogfight — track and fire
    EVADE,        // defensive maneuvers (chaff/flare, break turns)
    FLEE,         // disengage and run
    ORBIT,        // hold pattern (AWACS, tanker)
    RTB           // return to base
};

enum class AirSkill { ROOKIE, REGULAR, VETERAN, ACE };

struct AirEntity : EntityBase {
    // navigation
    std::vector<AircraftWaypoint> waypoints;
    size_t currentWaypointIndex = 0;
    Vector3 forward = {1, 0, 0};
    MeterPerSecond speed = 0;

    // AI
    AirBehavior behavior = AirBehavior::PATROL;
    AirBehavior initialBehavior = AirBehavior::PATROL;
    AirSkill skill = AirSkill::REGULAR;

    // engagement parameters
    Meter engageRange = 30000;     // switch to ENGAGE when player this close
    Meter disengageRange = 50000;  // switch back to PATROL when player this far
    Meter weaponRange = 15000;     // can fire weapons
    float lockOnTime = 2.0f;      // seconds to achieve lock
    float currentLockTime = 0.0f;

    // steering limits (tuned by skill)
    float maxTurnRate = 0.5f;
    float maxBankAngle = 60.0f;

    // weapons
    int missiles = 4;
    float missileReloadTimer = 0.0f;

    void update(const AircraftState& playerState, float dt);
    void steer(float dt);
    void evaluateThreat(const AircraftState& playerState);
};
```

### 3.5 SAM Site

```cpp
// src/entities/SamSite.h
#pragma once
#include "Entity.h"

enum class SamState {
    IDLE,          // scanning
    TRACKING,      // radar lock — player gets RWR warning
    FIRING,        // missile in flight
    RELOADING,     // between salvos
    DESTROYED
};

struct SamSite : EntityBase {
    SamState samState = SamState::IDLE;

    Meter detectionRange = 25000;
    Meter engagementRange = 18000;
    Meter minAltitude = 50;
    Meter maxAltitude = 12000;
    MeterPerSecond missileSpeed = 900;

    float reloadTime = 8.0f;
    float reloadTimer = 0.0f;
    int maxMissiles = 4;
    int remainingMissiles = 4;
    float accuracy = 0.7f;

    float trackingTime = 0.0f;       // how long target has been tracked
    float lockOnThreshold = 3.0f;    // seconds to achieve lock

    void update(const AircraftState& playerState, float dt);
    bool canEngage(const AircraftState& playerState) const;
    bool isPlayerInEnvelope(const AircraftState& playerState) const;
};
```

### 3.6 AAA Battery

```cpp
// src/entities/AAABattery.h
#pragma once
#include "Entity.h"

struct AAABattery : EntityBase {
    Meter detectionRange = 5000;
    Meter engagementRange = 3000;
    Meter maxAltitude = 3000;          // effective ceiling

    float fireRate = 10.0f;            // rounds per second
    float accuracy = 0.3f;
    float burstDuration = 2.0f;        // seconds per burst
    float burstCooldown = 1.5f;        // seconds between bursts
    float burstTimer = 0.0f;
    bool firing = false;

    void update(const AircraftState& playerState, float dt);
    bool isPlayerInRange(const AircraftState& playerState) const;
};
```

### 3.7 Ground Target (Structures)

```cpp
// src/entities/GroundTarget.h
#pragma once
#include "Entity.h"

// Structures are static — no AI, just health + position + model.
// Subtypes: "bridge", "depot", "radar", "bunker", "fuel_tank", "runway"
struct GroundTarget : EntityBase {
    bool strategicTarget = true;  // shows on map/HUD when true

    void takeDamage(float amount);
};
```

---

## 4. Objective System

### 4.1 Objective Types

```cpp
// src/scenario/Objective.h
#pragma once
#include "../entities/Entity.h"
#include <string>

enum class ObjectiveType {
    DESTROY,          // reduce target health to 0
    PHOTOGRAPH,       // fly near target with camera pod equipped, within parameters
    NAVIGATE,         // reach a waypoint within arrival radius
    INTERCEPT,        // destroy a specific aircraft
    ESCORT,           // keep a friendly entity alive until it reaches destination
    SEAD,             // destroy all SAM/AAA in a zone (Suppression of Enemy Air Defense)
    SURVIVE           // stay alive for a duration
};

enum class ObjectiveStatus {
    PENDING,          // not yet attempted
    ACTIVE,           // currently trackable (in range or triggered)
    COMPLETED,        // done
    FAILED            // no longer achievable
};

struct ObjectiveParams {
    // for PHOTOGRAPH
    Meter minAltitude = 0;
    Meter maxAltitude = 99999;
    Meter maxDistance = 500;
    Degree requiredAngle = 30;

    // for NAVIGATE
    Meter arrivalRadius = 2000;

    // for SURVIVE
    float durationSeconds = 0;

    // for ESCORT
    EntityId escortTarget;
    EntityId escortDestination;

    // for SEAD
    Vector3 zoneCenter = {0, 0, 0};
    Meter zoneRadius = 0;
};

struct Objective {
    std::string id;
    ObjectiveType type;
    std::string label;           // human-readable description
    EntityId targetEntityId;     // what entity this relates to
    bool required = true;        // must complete for mission success
    int displayOrder = 0;
    ObjectiveStatus status = ObjectiveStatus::PENDING;
    ObjectiveParams params;
};
```

### 4.2 Objective Evaluator

Each objective type has evaluation logic. This is a pure function that checks whether conditions are met each frame.

```cpp
// src/scenario/ObjectiveEvaluator.h
#pragma once
#include "Objective.h"
#include "../entities/EntityRegistry.h"
#include "../core/AircraftStructs.h"

namespace ObjectiveEvaluator {

    // called every frame for each active objective
    bool evaluate(
        const Objective& objective,
        const AircraftState& playerState,
        const EntityRegistry& entities,
        float missionElapsed
    );

    // type-specific checks:
    bool checkDestroy(const Objective& obj, const EntityRegistry& entities);
    bool checkPhotograph(const Objective& obj, const AircraftState& player, const EntityRegistry& entities);
    bool checkNavigate(const Objective& obj, const AircraftState& player, const EntityRegistry& entities);
    bool checkIntercept(const Objective& obj, const EntityRegistry& entities);
    bool checkSead(const Objective& obj, const EntityRegistry& entities);
    bool checkSurvive(const Objective& obj, float missionElapsed);
}
```

---

## 5. Trigger System

Triggers allow dynamic mission events — reinforcement spawns, radio messages, environmental changes — driven by in-game conditions.

### 5.1 Trigger Types

```cpp
// src/scenario/Trigger.h
#pragma once
#include "../entities/Entity.h"
#include <variant>
#include <string>

enum class TriggerType {
    ZONE_ENTER,           // player enters a sphere
    ZONE_EXIT,            // player leaves a sphere
    ENTITY_DESTROYED,     // a specific entity dies
    ENTITY_DETECTS_PLAYER,// an entity's sensor detects the player
    TIME_ELAPSED,         // mission clock reaches N seconds
    OBJECTIVE_COMPLETE,   // a specific objective is completed
    ALTITUDE_BELOW,       // player drops below altitude
    ALTITUDE_ABOVE,       // player climbs above altitude
    SPEED_BELOW,          // player slows below speed
    DAMAGE_TAKEN          // player takes damage
};

enum class TriggerActionType {
    SPAWN_ENTITY,         // add a new entity to the world
    DESTROY_ENTITY,       // remove an entity
    RADIO_MESSAGE,        // display a radio comms message
    UPDATE_OBJECTIVE,     // change an objective's status or label
    PLAY_SOUND,           // play an audio cue
    SET_WEATHER,          // change weather conditions (future)
    ACTIVATE_TRIGGER      // chain to another trigger
};

struct TriggerCondition {
    TriggerType type;
    // parameters vary by type — stored as generic map or variant
    EntityId entityId;          // for entity-related triggers
    Vector3 zoneCenter;         // for zone triggers
    float radius;               // for zone triggers
    float value;                // for time/altitude/speed triggers
    std::string objectiveId;    // for objective triggers
};

struct TriggerAction {
    TriggerActionType type;
    std::string message;        // for radio messages
    EntityId entityId;          // for entity actions
    std::string objectiveId;    // for objective updates
    std::string soundFile;      // for audio
    // for spawn — full entity definition loaded from JSON
    // (stored as parsed data, spawned on activation)
};

struct Trigger {
    std::string id;
    TriggerCondition condition;
    TriggerAction action;
    bool once = true;           // fire only once, or repeating?
    bool fired = false;
};
```

### 5.2 Trigger Evaluator

```cpp
// src/scenario/TriggerEvaluator.h
#pragma once
#include "Trigger.h"

namespace TriggerEvaluator {
    bool evaluate(
        const Trigger& trigger,
        const AircraftState& playerState,
        const EntityRegistry& entities,
        const std::vector<Objective>& objectives,
        float missionElapsed
    );
}
```

---

## 6. Mission Manager

The **MissionManager** is the top-level orchestrator that ties everything together. It lives inside `GameplayScreen` as a composed component, following the existing pattern.

```cpp
// src/scenario/MissionManager.h
#pragma once
#include "Scenario.h"
#include "Objective.h"
#include "Trigger.h"
#include "ObjectiveEvaluator.h"
#include "TriggerEvaluator.h"
#include "../entities/EntityRegistry.h"
#include "../core/AircraftStructs.h"

enum class MissionState {
    STARTING,        // initial setup, countdown
    ACTIVE,          // mission in progress
    SUCCEEDED,       // all required objectives met
    FAILED,          // failure condition triggered
    ABORTED          // player quit
};

struct MissionResult {
    MissionState finalState;
    float elapsedTime;
    float fuelRemaining;
    int score;
    std::vector<Objective> objectives;    // with final statuses
    int enemyAircraftKills;
    int samDestroyed;
    int aaaDestroyed;
    // ... extend as needed
};

class MissionManager {
    Scenario scenario;
    MissionState missionState = MissionState::STARTING;
    float missionElapsed = 0.0f;

    // owned systems
    EntityRegistry entities;
    std::vector<Objective> objectives;
    std::vector<Trigger> triggers;

    // scoring
    int currentScore = 0;

    // internal
    void evaluateObjectives(const AircraftState& playerState);
    void evaluateTriggers(const AircraftState& playerState);
    void executeTriggerAction(const TriggerAction& action);
    void checkMissionCompletion(const AircraftState& playerState);
    void spawnInitialEntities();

public:
    explicit MissionManager(const Scenario& scenario);

    void update(const AircraftState& playerState, float dt);

    // queries for views
    [[nodiscard]] MissionState getState() const;
    [[nodiscard]] const std::vector<Objective>& getObjectives() const;
    [[nodiscard]] const EntityRegistry& getEntities() const;
    [[nodiscard]] EntityRegistry& getEntitiesMut();
    [[nodiscard]] float getElapsedTime() const;
    [[nodiscard]] int getScore() const;
    [[nodiscard]] MissionResult buildResult() const;

    // get the next objective waypoint for HUD/map display
    [[nodiscard]] const Objective* getActiveObjective() const;
    [[nodiscard]] Vector3 getObjectiveWorldPosition(const Objective& obj) const;
};
```

### Update Flow

```
MissionManager::update(playerState, dt)
│
├─ missionElapsed += dt
│
├─ entities.update(playerState, dt)       // AI, movement, firing
│   ├─ each AirEntity::update()           // patrol/engage/evade
│   ├─ each SamSite::update()             // detect/track/fire
│   └─ each AAABattery::update()          // detect/fire
│
├─ evaluateTriggers(playerState)          // check all trigger conditions
│   └─ executeTriggerAction()             // spawn, message, etc.
│
├─ evaluateObjectives(playerState)        // check completion criteria
│
└─ checkMissionCompletion(playerState)    // success? failure? continue?
```

---

## 7. Scenario Loader

Parses scenario JSON files into the `Scenario` struct using the existing `nlohmann::json` library.

```cpp
// src/scenario/Scenario.h
#pragma once
#include "Objective.h"
#include "Trigger.h"
#include "../entities/Entity.h"
#include <vector>
#include <string>

struct WeaponLoadout {
    int slots;
    std::vector<std::string> available;
    std::vector<std::string> defaultLoadout;
    std::vector<std::string> selectedLoadout;  // player's choice from briefing
};

struct StartConditions {
    Vector3 position;
    float heading;
    MeterPerSecond speed;
    Meter altitude;
    float fuel;
    bool onCarrier;
};

struct ScenarioScoring {
    int objectiveComplete = 1000;
    int bonusObjective = 500;
    int enemyAircraftKill = 250;
    int samDestroyed = 200;
    int aaaDestroyed = 100;
    bool timeBonus = true;
    bool fuelBonus = true;
    int noDamageBonus = 500;
};

struct Scenario {
    std::string id;
    std::string name;
    std::string description;
    std::string difficulty;
    std::string theater;

    StartConditions start;
    WeaponLoadout loadout;

    std::vector<Objective> objectives;
    std::vector<EntityBase> entityDefinitions;  // templates — spawned at mission start
    std::vector<Trigger> triggers;
    ScenarioScoring scoring;
};

// src/scenario/ScenarioLoader.h
#pragma once
#include "Scenario.h"
#include "../lib/json.hpp"
#include <string>

class ScenarioLoader {
public:
    static Scenario load(const std::string& path);
    static std::vector<std::string> listAvailable(const std::string& scenariosDir);
};
```

---

## 8. Weapon System

Weapons are carried by the player and some AI entities. They integrate with the entity/damage system.

```cpp
// src/weapons/Weapon.h
#pragma once
#include "../primitives/Types.h"
#include <string>

enum class WeaponType {
    AIR_TO_AIR_IR,     // AIM-9 Sidewinder — heat-seeking
    AIR_TO_AIR_RADAR,  // AIM-7 Sparrow — radar-guided
    BOMB_UNGUIDED,     // Mk-82/Mk-84 — dumb bombs
    AIR_TO_GROUND,     // AGM-65 Maverick — guided
    GUN,               // M61 Vulcan cannon
    CAMERA_POD         // reconnaissance camera (no damage, for photograph objectives)
};

enum class WeaponState {
    READY,
    LOCKED,            // has target lock (for guided weapons)
    FIRED,
    EXPENDED
};

struct WeaponDefinition {
    std::string id;             // "aim9", "mk82", etc.
    std::string name;           // "AIM-9 Sidewinder"
    WeaponType type;
    float damage = 100.0f;
    Meter range = 10000;
    MeterPerSecond speed = 800; // for missiles
    float lockTime = 2.0f;     // seconds to lock (guided weapons)
    float blastRadius = 0.0f;  // for bombs — area damage
};

// Loaded from config — all weapon stats in res/config/weapons.jsonc
```

```cpp
// src/weapons/WeaponManager.h
#pragma once
#include "Weapon.h"
#include <vector>

struct WeaponSlot {
    WeaponDefinition definition;
    WeaponState state = WeaponState::READY;
    int count = 1;                 // how many of this weapon on this pylon
};

class WeaponManager {
    std::vector<WeaponSlot> pylons;
    int selectedPylon = 0;
    float lockTimer = 0.0f;
    EntityId lockedTarget;

public:
    void loadFromLoadout(const std::vector<std::string>& loadout);
    void selectNext();
    void selectPrevious();

    // attempt to fire currently selected weapon
    bool fire(const AircraftState& playerState, const EntityRegistry& entities);

    // update lock-on timers
    void update(const AircraftState& playerState, const EntityRegistry& entities, float dt);

    [[nodiscard]] const WeaponSlot& getCurrentWeapon() const;
    [[nodiscard]] int getRemainingCount() const;
    [[nodiscard]] bool hasLock() const;
    [[nodiscard]] float getLockProgress() const;  // 0.0 → 1.0
};
```

---

## 9. Damage & Projectile System

When weapons are fired, they become **projectiles** that travel through the world and resolve hits.

```cpp
// src/weapons/Projectile.h
#pragma once
#include "../entities/Entity.h"

enum class ProjectileSource { PLAYER, ENEMY_AIRCRAFT, SAM, AAA };

struct Projectile {
    Vector3 position;
    Vector3 velocity;
    ProjectileSource source;
    float damage;
    float blastRadius;
    float lifetime;          // seconds remaining before self-destruct
    EntityId targetId;       // for guided missiles — empty for unguided
    bool active = true;
};

class ProjectileManager {
    std::vector<Projectile> projectiles;

public:
    void spawn(const Projectile& p);

    // move projectiles, check for hits, apply damage
    void update(
        AircraftState& playerState,
        EntityRegistry& entities,
        float dt
    );

    void draw();

    [[nodiscard]] const std::vector<Projectile>& getAll() const;
};
```

---

## 10. Player Damage Model

The player aircraft needs a damage model to receive hits from enemy weapons.

```cpp
// src/core/PlayerDamage.h
#pragma once
#include "AircraftStructs.h"

struct DamageState {
    float hullIntegrity = 100.0f;     // 0 = destroyed
    bool engineDamaged = false;        // reduced thrust
    bool avionicsDamaged = false;      // radar/HUD flicker
    bool controlDamaged = false;       // sluggish controls
    bool fuelLeak = false;             // fuel drains faster

    int chaff = 30;                    // chaff countermeasures
    int flares = 30;                   // flare countermeasures

    [[nodiscard]] bool isDestroyed() const { return hullIntegrity <= 0; }
    [[nodiscard]] float thrustMultiplier() const { return engineDamaged ? 0.5f : 1.0f; }
    [[nodiscard]] float controlMultiplier() const { return controlDamaged ? 0.6f : 1.0f; }
    [[nodiscard]] float fuelDrainMultiplier() const { return fuelLeak ? 3.0f : 1.0f; }
};

class PlayerDamage {
    DamageState damage;

public:
    void takeDamage(float amount);
    void deployCountermeasure(bool chaff);  // true=chaff, false=flare
    void update(AircraftState& state, float dt);  // apply damage effects

    [[nodiscard]] const DamageState& getState() const;
};
```

---

## 11. Radio / Comms System

Triggers and events produce radio messages displayed as a comms feed.

```cpp
// src/scenario/RadioComms.h
#pragma once
#include <string>
#include <deque>

struct RadioMessage {
    std::string sender;       // "AWACS", "Wingman", "Tower", "RWR"
    std::string text;
    float timestamp;          // mission elapsed time
    float displayDuration;    // how long to show (seconds)
    float age = 0.0f;        // time since displayed
};

class RadioComms {
    std::deque<RadioMessage> messages;     // recent messages
    static constexpr int MAX_VISIBLE = 4;  // lines on screen

public:
    void push(const std::string& sender, const std::string& text, float timestamp);
    void update(float dt);
    [[nodiscard]] const std::deque<RadioMessage>& getVisible() const;
};
```

---

## 12. New & Modified Views

### 12.1 ObjectiveView (NEW)

HUD overlay showing current objectives with status icons.

```
┌─────────────────────────────────────────┐
│  OBJECTIVES                             │
│  ✓ Destroy northern bridge      [DONE]  │
│  ► Destroy southern bridge    [ACTIVE]  │
│  ○ Photograph supply depot   [PENDING]  │
│  ○ Return to base            [PENDING]  │
└─────────────────────────────────────────┘
```

### 12.2 ThreatView (NEW — RWR: Radar Warning Receiver)

Circular display showing bearing of active radar threats (SAMs tracking the player).

```
        N
    ╭───┼───╮
    │   │   │
  W─┼───●───┼─E     ● = player
    │  ▲│   │        ▲ = SAM radar (bearing)
    ╰───┼───╯        ★ = missile inbound
        S
```

### 12.3 WeaponView (NEW)

Shows current weapon selection, remaining count, and lock status.

```
  ┌──────────────────────┐
  │ [AIM-9] ×2  READY    │
  │  MK-84  ×4           │
  │  AGM-65 ×2           │
  │  CAM    ×1           │
  └──────────────────────┘
```

### 12.4 CommsView (NEW)

Displays recent radio messages from AWACS, wingman, tower.

```
  ┌──────────────────────────────────────────┐
  │ AWACS: Bandit, bearing 045, angels 15    │
  │ RWR: WARNING — SAM radar lock detected!  │
  └──────────────────────────────────────────┘
```

### 12.5 BriefingView (NEW — used in BriefingScreen)

Full-screen mission briefing with map, objectives list, threat overview, and loadout selection.

### 12.6 DebriefView (NEW — used in DebriefScreen)

Score breakdown, objective results, flight statistics.

### 12.7 Modified: RadarView

Extended to show **all** entities from `EntityRegistry` within radar range, not just a single enemy. Different blip shapes for aircraft vs ground threats.

### 12.8 Modified: MapView

Extended to render:
- Objective markers (diamonds for targets, circles for waypoints)
- Known threat circles (SAM detection radii)
- Entity positions (friendly/enemy icons)
- Player flight path breadcrumbs

---

## 13. Integration with Existing Systems

### 13.1 Modified GameplayScreen

```cpp
class GameplayScreen : public GameScreen {
    GameData game;
    SceneManager scene;
    MissionManager mission;        // NEW
    WeaponManager weapons;         // NEW
    ProjectileManager projectiles; // NEW
    PlayerDamage playerDamage;     // NEW

    // existing views
    CockpitView cockpitView;
    DebugView debugView;
    MinihudView minihudView;
    MapView mapView;
    RadarView radarView;

    // new views
    ObjectiveView objectiveView;   // NEW
    ThreatView threatView;         // NEW
    WeaponView weaponView;         // NEW
    CommsView commsView;           // NEW

public:
    explicit GameplayScreen(AppConfig& config, const Scenario& scenario);

    ScreenState update() override;
    void run() override;
};
```

### 13.2 Modified Update Flow

```
GameplayScreen::update()
│
├─ handle pause (existing)
├─ dt = GetFrameTime()
│
├─ game.update(dt)                    // player flight physics (existing)
├─ scene.update(state, dt)            // terrain & audio (existing)
│
├─ mission.update(state, dt)          // NEW — entities, objectives, triggers
│   ├─ entities.update()              //   AI aircraft, SAMs, AAA
│   ├─ evaluateTriggers()             //   check trigger conditions
│   ├─ evaluateObjectives()           //   check completion
│   └─ checkMissionCompletion()       //   success/failure
│
├─ weapons.update(state, entities, dt)// NEW — lock-on, weapon cycling
├─ projectiles.update(state, entities, dt) // NEW — flight, hit detection
├─ playerDamage.update(state, dt)     // NEW — apply damage effects
│
├─ views update (existing + new)
│
└─ if mission.getState() == SUCCEEDED or FAILED
       return ScreenState::DEBRIEF
```

### 13.3 Modified main.cpp Screen Transitions

```cpp
case ScreenState::BRIEFING:
    currentScreen = std::make_unique<BriefingScreen>(*config, selectedScenario);
    break;
case ScreenState::GAMEPLAY:
    currentScreen = std::make_unique<GameplayScreen>(*config, selectedScenario);
    break;
case ScreenState::DEBRIEF:
    currentScreen = std::make_unique<DebriefScreen>(*config, missionResult);
    break;
```

---

## 14. Directory Structure

```
src/
├── main.cpp
├── GameScreen.h
├── GameplayScreen.h/.cpp          // modified
├── BriefingScreen.h/.cpp          // NEW
├── DebriefScreen.h/.cpp           // NEW
├── MainMenuScreen.h/.cpp          // modified (scenario selection)
├── SplashScreen.h/.cpp
│
├── core/                          // existing — player aircraft sim
│   ├── AircraftStructs.h          // add DamageState integration
│   ├── AircraftControls.h/.cpp
│   ├── AircraftPhysics.h/.cpp     // modified — damage multipliers
│   ├── AircraftTransformation.h/.cpp
│   ├── AircraftCamera.h/.cpp
│   ├── GameData.h/.cpp
│   ├── PlayerDamage.h/.cpp        // NEW
│   └── SceneManager.h/.cpp
│
├── entities/                      // NEW — world entity system
│   ├── Entity.h
│   ├── EntityRegistry.h/.cpp
│   ├── AirEntity.h/.cpp
│   ├── SamSite.h/.cpp
│   ├── AAABattery.h/.cpp
│   ├── GroundTarget.h/.cpp
│   ├── NavalEntity.h/.cpp
│   └── Airbase.h/.cpp
│
├── scenario/                      // NEW — mission logic
│   ├── Scenario.h
│   ├── ScenarioLoader.h/.cpp
│   ├── MissionManager.h/.cpp
│   ├── Objective.h
│   ├── ObjectiveEvaluator.h/.cpp
│   ├── Trigger.h
│   ├── TriggerEvaluator.h/.cpp
│   └── RadioComms.h/.cpp
│
├── weapons/                       // NEW — weapon & projectile system
│   ├── Weapon.h
│   ├── WeaponManager.h/.cpp
│   ├── Projectile.h
│   └── ProjectileManager.h/.cpp
│
├── views/                         // existing + new overlays
│   ├── CockpitView.h/.cpp
│   ├── DebugView.h/.cpp
│   ├── MinihudView.h/.cpp
│   ├── MapView.h/.cpp             // modified — entity/objective markers
│   ├── RadarView.h/.cpp           // modified — multi-entity blips
│   ├── ObjectiveView.h/.cpp       // NEW
│   ├── ThreatView.h/.cpp          // NEW — RWR display
│   ├── WeaponView.h/.cpp          // NEW
│   ├── CommsView.h/.cpp           // NEW
│   ├── BriefingView.h/.cpp        // NEW
│   └── DebriefView.h/.cpp         // NEW
│
├── primitives/                    // existing
│   ├── AppConfig.h/.cpp
│   ├── Constants.h
│   ├── Types.h                    // add weapon/entity type aliases
│   ├── Resource.h
│   └── Utils.h
│
├── services/
│   └── TileManager.h/.cpp
│
└── lib/
    ├── json.hpp
    └── raygui.h

res/
├── config/
│   ├── app.jsonc                  // existing
│   └── weapons.jsonc              // NEW — weapon definitions
│
├── scenarios/                     // NEW — mission files
│   ├── tutorial-01.jsonc
│   ├── strike-bridges-01.jsonc
│   ├── recon-depot.jsonc
│   ├── air-superiority-01.jsonc
│   └── deep-strike-01.jsonc
│
├── models/                        // NEW (or expand existing)
│   ├── mig29.glb
│   ├── mig21.glb
│   ├── sam_launcher.glb
│   ├── aaa_gun.glb
│   ├── bridge.glb
│   ├── depot.glb
│   └── ...
│
└── ... (existing assets)
```

---

## 15. Scenario Ideas (Variety)

| # | Name | Primary Objective | Key Threats | Special |
|---|------|-------------------|-------------|---------|
| 1 | **First Strike** | Destroy radar installation | 2× AAA | Tutorial mission, simple |
| 2 | **Bridge Busters** | Destroy 2 bridges | 1× SA-6, 2× AAA, 2× MiG-21 | Combined arms |
| 3 | **Photo Recon** | Photograph 3 targets | 1× SA-2, 4× MiG-29 | Camera pod required, no bombs |
| 4 | **Air Superiority** | Shoot down 4 bandits | 4× MiG-29, 2× MiG-21 | Air-to-air only |
| 5 | **Deep Strike** | Destroy chemical plant | 2× SA-6, 1× SA-2, 4× AAA, 6× MiG | Full SEAD + strike |
| 6 | **Carrier Defense** | Intercept inbound bombers | 6× Tu-22M bombers + 4× escorts | Time pressure |
| 7 | **SEAD Sweep** | Destroy all SAM sites in sector | 4× SA-6, 2× SA-2, AAA | Specialized loadout |
| 8 | **Escort Duty** | Protect recon aircraft to target | Ambush fighters + SAMs | Must keep friendly alive |
| 9 | **Night Raid** | Destroy fuel depot at night | SA-2, AAA, patrol fighters | Reduced visibility |
| 10 | **Ace Duel** | 1v1 vs ace pilot | 1× Su-27 (ace skill) | Dogfight only |

---

## 16. Implementation Order

The system should be built incrementally, each phase delivering playable value:

### Phase 1 — Foundation
Scenario data structures, ScenarioLoader, EntityRegistry, basic entity types (GroundTarget). Load a scenario JSON → spawn static entities → render as cubes. BriefingScreen with text display.

### Phase 2 — Objectives & Mission Flow
Objective struct, ObjectiveEvaluator (DESTROY and NAVIGATE), MissionManager orchestration, MissionState transitions, ObjectiveView HUD overlay. First playable: fly to target, drop bomb, return to base.

### Phase 3 — Air Threats
AirEntity with patrol/engage/flee AI. Extend RadarView for multiple contacts. CommsView for AWACS warnings. Enemy aircraft rendered and interactive.

### Phase 4 — Ground Threats
SamSite and AAABattery with detection/engagement logic. ThreatView (RWR). Player damage model. Chaff/flare countermeasures.

### Phase 5 — Weapons
WeaponManager, ProjectileManager, weapon definitions in JSON. Lock-on mechanics. WeaponView HUD. Bomb/missile delivery.

### Phase 6 — Triggers & Events
Full trigger system. Dynamic spawns, radio messages, chained events. Complex multi-phase missions.

### Phase 7 — Polish
DebriefScreen with scoring. MapView enhancements (threat rings, objective markers, flight path). Sound effects for weapons/RWR/comms. Multiple scenario files.

### Phase 8 — Advanced
Photograph objective. Escort objective. SEAD objective. Weather effects. Night missions. Difficulty scaling (more enemies, better AI, tighter time limits).
