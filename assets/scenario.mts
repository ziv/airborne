/**
 * Scenario JSON File Definition
 * -----------------------------
 */

export enum WeaponCategory {
    AIR_TO_AIR,  ///< Air-to-air missiles (e.g. AIM-9, AIM-120).
    AGM,         ///< Air-to-ground missiles (e.g. AGM-65 Maverick).
    BOMB,        ///< Unguided/guided bombs (e.g. Mk-82, GBU-12).
    GUN,         ///< Aircraft cannon (e.g. M61 Vulcan).
    SAM,         ///< Surface-to-air missile system.
    AAA          ///< Anti-aircraft artillery system.
}


export type MissileDescriptor = {
    // for identification
    name: string;
    nato_code: string;
    type: WeaponCategory.AAA | WeaponCategory.AGM | WeaponCategory.SAM;

    // for physics
    thrust: number;
    weight: number;
    turn_rate: number;
    max_speed: number;
    burn_time: number; // the time the engine is cut off

    // for gameplay
    damage: number;
    area_of_effect: number;
    accuracy: number; // 0 to 1, higher is better
}


export type SameSiteDescriptor = {
    id: string;
    name: string;
    armament: MissileDescriptor[];

    health: number;
    max_health: number;
    rate_of_fire: number;
    position: Vector3;
    heading: AngleDeg;
};

// raylib type
export type Vector3 = { x: number; y: number; z: number };
export type AngleDeg = number;
export type entt_hashed_string = number;


export enum Faction {FRIENDLY, ENEMY, NEUTRAL}

export enum EntityState {
    INACTIVE,   ///< Not yet activated.
    ACTIVE,     ///< Alive and operational.
    DAMAGED,    ///< Alive but below 50 % health.
    DESTROYED,  ///< Killed — remains in the registry for scoring/triggers.
    DESPAWNED   ///< Removed from the world entirely.
}

export enum EntityType {
    None,
    AIRCRAFT,
    SAM,        ///< Surface-to-air missile site.
    AAA,        ///< Anti-aircraft artillery.
    STRUCTURE,  ///< Static buildings: bridges, depots, radars, bunkers.
    NAVAL,
    AIRBASE,  ///< Friendly or enemy airstrip / carrier.
    SHIP,
    CARRIER,  ///< Friendly or enemy airstrip / carrier.
    WAYPOINT  ///<
}

export enum Difficulty { EASY, MEDIUM, HARD }

export enum Weather { SUNNY, CLOUDY, OVERCAST, RAINY }

export enum Season { SUMMER, WINTER }

export enum TimeOfDay { DAY, NIGHT, DAWN, DUSK }

export enum ObjectiveType {
    DESTROY,   ///< Eliminate a specific target entity.
    NAVIGATE,  ///< Reach a waypoint or area.
    ESCORT,    ///< Protect a friendly entity.
    SURVIVE    ///< Stay alive for a duration or until event.
}


export type TilesDef = {
    texture_path: string;
    heightmap_path: string;
    meter_to_pixel: number;
    texture_size: number;
    heightmap_size: number;
    lowset: number;
    highest: number;
    x_tiles_count: number;
    z_tiles_count: number;
};

export type StartDef = {
    position: Vector3;
    heading: number;
    fuel: number;
    carrier: boolean;
    weapons: { slot: number; weapon: string; }[];
};

export interface Component {
    id: string;
}

export interface TypeComponent extends Component {
    type: EntityType;
}

export interface SubtypeComponent extends Component {
    subtype: string;
}

export interface FactionComponent extends Component {
    faction: Faction;
}

export interface StateComponent extends Component {
    state: EntityState;
}

export interface PositionComponent extends Component {
    position: Vector3;
}

export interface HeadingComponent extends Component {
    heading: AngleDeg;
}

export interface HealthComponent extends Component {
    health: number;
    max_health: number;
}

export interface ModelComponent extends Component {
    model_path: string;
}

export interface SpeedComponent extends Component {
    speed: number;
    max_speed: number;
}

export interface TurnRateComponent extends Component {
    turn_rate: number;  // degrees per second
}

export interface ClimbRateComponent extends Component {
    climb_rate: number;  // meters per second
}

export interface RadarRangeComponent extends Component {
    detection_range: number;
    engagement_range: number;
}

export interface RangeComponent extends Component {
    range: number;
}

export interface FireRateComponent extends Component {
    fire_rate: number;  // rounds per minute
}

export interface DamageComponent extends Component {
    damage: number;
}

export interface RateOfFire extends Component {
    rate_of_fire: number;
}

export interface AreaOfEffect extends Component {
    radius: number;
}

export interface AccuracyComponent extends Component {
    accuracy: number;
}

export interface WeaponCategoryComponent extends Component {
    category: WeaponCategory;
}


export interface WeaponsComponent extends Component {
    weapons: EntityDef[];
}

export type Components =
    | TypeComponent
    | SubtypeComponent
    | FactionComponent
    | StateComponent
    | PositionComponent
    | HeadingComponent
    | HealthComponent
    | ModelComponent
    | SpeedComponent
    | TurnRateComponent
    | ClimbRateComponent
    | RadarRangeComponent
    | RangeComponent
    | FireRateComponent
    | DamageComponent
    | RateOfFire
    | AreaOfEffect
    | AccuracyComponent
    | WeaponCategoryComponent
    | WeaponsComponent;

export type EntityDef = {
    id: string;
    name: string;
    properties: Components[];
};

// examples

// example structure

const example_structure: EntityDef = {
    id: "some-id",
    name: "international airport",
    properties: [
        {id: "position", position: {x: 100, y: 0, z: 200}},
        {id: "health", health: 100, max_health: 100},
        {id: "faction", faction: Faction.NEUTRAL},
    ]
};

const example_sam_weapon: EntityDef = {
    id: "missile-0",
    name: "patriot-missile",
    properties: [
        {id: "type", type: EntityType.SAM},
        {id: "subtype", subtype: "patriot"},
        {id: "category", category: WeaponCategory.SAM},
        {id: "damage", damage: 50},
        {id: "range", range: 800},
        {id: "fire_rate", fire_rate: 10},
        {id: "accuracy", accuracy: 0.8}
    ]
};

const example_sam_structure: EntityDef = {
    id: "sam-0",
    name: "patriot-site",
    properties: [
        {id: "position", position: {x: 200, y: 0, z: 200}},
        {id: "health", health: 100, max_health: 100},
        {id: "faction", faction: Faction.ENEMY},
        {id: "radar", detection_range: 1000, engagement_range: 800},
        {id: "weapon", weapons: []}
    ]
};

export interface Armament {
    name: number;
    range: number;
    damage: number;
    area_of_effect: number;
    rate_of_fire: number;
    accuracy: number;
    ballistic: boolean;
}

export interface Missile extends Armament {
    ballistic: false;
}

export interface Bullet extends Armament {
    ballistic: true;
}