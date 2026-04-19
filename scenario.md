# Scenario

A scenario file defines a single mission: where the player starts, what entities exist in the world, and what objectives to complete.

## Scenario metadata

* **id** — unique identifier (e.g. `"first-strike"`)
* **name** — display name
* **description** — mission briefing text
* **difficulty** — `"easy"` | `"medium"` | `"hard"`
* **theater** — geographic region / map name (maps to scene config in `config.jsonc`)
* **skyColor** — RGBA array `[r, g, b, a]` used as the clear color each frame
* **weather** — 
* **day/night** — 

## Start conditions

Where and how the player begins the mission.

* **position** — `{x, y, z}` world-space spawn point
* **heading** — compass heading in degrees (0 = north/+Z)
* **speed** — initial speed in m/s (0 for ground/carrier start)
* **altitude** — initial altitude in meters
* **fuel** — starting fuel in kilograms
* **carrier** — `true` if starting on a carrier deck (affects ground height)

## Player loadout (planned)

* **slots** — number of weapon hardpoints
* **available** — list of weapon IDs the player can choose from
* **defaults** — pre-selected weapon ID per slot

## Objectives (planned)

Each objective has:

* **id** — unique identifier
* **type** — `"destroy"` | `"navigate"` | `"escort"` | `"survive"`
* **target** — entity ID or area reference
* **label** — display text
* **required** — `true` for primary objectives, `false` for bonus
* **order** — display/evaluation order
* **params** — type-specific parameters (e.g. `arrivalRadius` for navigate)

## Completion (planned)

* **success** — condition for mission success (e.g. `"all_required_objectives"`)
* **failure** — list of failure conditions (e.g. `"player_destroyed"`, `"player_ejected"`)

---

## Entities

All entities in the scenario share a common base definition, then each type has additional data either inlined in `params`/`properties` or in a separate lookup map.

### Common fields (all entity types)

* **id** — unique string identifier, used for objective references and debugging
* **type** — `"aircraft"` | `"sam"` | `"aaa"` | `"structure"` | `"naval"` | `"airbase"` | `"carrier"` | `"waypoint"`
* **subtype** — finer classification within the type (e.g. `"mig29"`, `"zsu23"`, `"radar"`, `"carrier"`, `"international-airport"`)
* **faction** — `"friendly"` | `"enemy"` | `"neutral"`
* **state** — `"inactive"` | `"active"` | `"damaged"` | `"destroyed"` | `"despawned"`
* **position** — `{x, y, z}` world-space coordinates
* **heading** — compass heading in degrees (0 = north/+Z)
* **health** — current health points
* **maxHealth** — maximum health points
* **scale** — model render scale (default `1.0`)
* **modelId** — path to `.glb` model file (e.g. `"assets/models/mig-29.glb"`). If empty, the entity exists in the world but has no 3D model (systems still operate on it)
* **params** — `map<string, float>` for type-specific numeric data
* **properties** — `map<string, string>` for type-specific string data

### Components created for all entities

Every entity spawned from the scenario gets these ECS components:

| Component      | Source field  | Purpose                             |
|----------------|---------------|--------------------------------------|
| `Identify`     | `id`          | Name/label for debugging and UI      |
| `IdentifyType` | `type`        | Entity type for system queries       |
| `Position3D`   | `position`    | World-space position + offset        |
| `Heading`      | `heading`     | Compass orientation                  |
| `FriendFoe`    | `faction`     | Friend/foe identification for radar  |
| `WithModel`    | `modelId`     | 3D model handle (only if model is preloaded in resources) |

### Rendering requirements

For an entity to be visible in the 3D world, it needs: `Position3D` + `Heading` + `WithModel`. The `RenderModels` system draws all entities with these three components (excluding the `World` entity). Models must be listed in `resources.jsonc` and preloaded at startup.

On the radar, entities are drawn based on `Identify` + `Position3D` + `FriendFoe` + `IdentifyType`. Each type has a distinct blip shape (triangle for aircraft, X for SAM/AAA, circle for naval, rectangle for structures/airbases).

---

## Entity type details

### Aircraft (`type: "aircraft"`)

AI-controlled aircraft. Currently spawned as static entities (no AI movement yet).

Additional data per aircraft (in a separate `aircraft` map, keyed by entity ID):

* **fuel** — starting fuel in kg
* **fuel_consumption_rate** — kg per second at full throttle
* **max_speed** — maximum speed in m/s
* **agility** — AI autopilot maneuver limits
    * **roll** — max roll rate
    * **pitch** — max pitch rate
    * **yaw** — max yaw rate
* **waypoints** — ordered list of navigation points `{x, y, z}`
* **weapons** — list of mounted weapons
    * **type** — weapon category (e.g. `"missile"`, `"bomb"`)
    * **subtype** — specific weapon ID (e.g. `"aim9"`, `"mk82"`)
    * **quantity** — count available

Planned components (not yet implemented):
* `LinearVelocity` — for AI movement
* `Aircraft` — aerodynamic coefficients (cd, cl, weight, agility ratios)
* `Engine` — thrust and throttle state
* `AircraftControls` — AI input state

### AAA (`type: "aaa"`)

Anti-aircraft artillery. Ground-based, fires at aircraft within range.

Behavior params (in `params` map on the entity):

* **detectionRange** — meters, distance at which the unit detects aircraft
* **engagementRange** — meters, distance at which it opens fire
* **maxAltitude** — meters, maximum altitude it can engage
* **fireRate** — rounds per second
* **accuracy** — hit probability per burst (0.0–1.0)
* **burstDuration** — seconds per firing burst

### SAM (`type: "sam"`)

Surface-to-air missile site. Similar to AAA but fires guided missiles.

Behavior params (in `params` map):

* **detectionRange** — meters
* **engagementRange** — meters
* **maxAltitude** — meters
* **fireRate** — missiles per salvo
* **accuracy** — guidance accuracy factor (0.0–1.0)
* **reloadTime** — seconds between salvos

Each SAM site references weapons from the weapon definitions map.

### Structure (`type: "structure"`)

Static buildings and installations (radar sites, bridges, depots, bunkers). Can be destroyed but don't move or shoot.

Subtypes: `"radar"`, `"bridge"`, `"depot"`, `"bunker"`

No additional params required beyond the common fields. They exist as mission targets and are rendered with their model.

### Airbase (`type: "airbase"`)

Friendly or enemy airstrip. Provides a landing zone for the player.

Additional components on creation:
* `Landable { carrier: false }` — marks this entity as a landing zone

Landing zone params (in `params` map):

* **minx**, **maxx**, **minz**, **maxz** — bounding box of the airfield area (world coordinates)

Landing zone detection uses a heading-aligned box:
* Half-width: 200m across the runway
* Half-length: 2000m along the runway
* Surface height: 0m (ground level)
* Landing box height: 150m above surface

### Carrier (`type: "carrier"`)

Friendly aircraft carrier. Provides a landing zone with an elevated deck.

Additional components on creation:
* `Landable { carrier: true }` — marks as carrier landing zone
* `Carrier` — tag component for carrier-specific queries

Landing zone detection uses a heading-aligned box:
* Half-width: 100m across the deck
* Half-length: 250m along the deck
* Surface height: 8m (deck elevation above sea level)
* Landing box height: 150m above surface

### Naval (`type: "naval"`)

Surface vessels (non-carrier). Displayed on radar as a circle blip. No special components beyond the common set.

### Waypoint (`type: "waypoint"`)

Invisible navigation point. No model, no health. Used for AI aircraft routes and mission objectives. Only needs `id`, `type`, `position`, and optionally `faction`.

---

## Weapon definitions (planned)

A global map of weapon types referenced by aircraft and SAM loadouts:

* **weapon_id** (key)
    * **range** — max effective range in meters
    * **reload_time** — seconds between launches
    * **damage** — damage per hit
    * **velocity** — projectile/missile speed in m/s
    * **agility** — guidance maneuver limits (for guided weapons)
        * **roll**
        * **pitch**
        * **yaw**
    * **type** — `"missile"` | `"bomb"` | `"gun"`
    * **guidance** — `"ir"` | `"radar"` | `"unguided"` (for missiles)
    * **blast_radius** — meters (for bombs and missiles with area effect)

---

## Triggers (planned)

Event-driven scripting to make scenarios dynamic:

* **id** — unique trigger identifier
* **event** — what activates it (e.g. `"entity_destroyed"`, `"player_enters_area"`, `"timer"`)
* **condition** — optional additional condition to check
* **action** — what happens (e.g. `"spawn_entity"`, `"update_objective"`, `"play_radio"`)
* **params** — event/action-specific parameters
* **once** — `true` if trigger fires only once (default), `false` for repeating