// Entity type definitions, visual styles, default values, and param schemas.

export const FACTIONS = ['friendly', 'enemy', 'neutral'];
export const ENTITY_STATES = ['active', 'inactive', 'damaged', 'destroyed', 'despawned'];
export const DIFFICULTIES = ['easy', 'medium', 'hard'];
export const WEATHER = ['sunny', 'cloudy', 'overcast', 'rainy'];
export const SEASONS = ['summer', 'winter'];
export const TIME_OF_DAY = ['day', 'night', 'dawn', 'dusk'];
export const OBJECTIVE_TYPES = ['destroy', 'navigate', 'escort', 'survive'];

export const WEAPON_CATEGORIES = ['air-to-air', 'agm', 'bomb', 'gun', 'sam', 'aaa'];

// Categories that represent guided missiles (have steering/agility)
export const MISSILE_CATEGORIES = ['air-to-air', 'agm', 'sam'];

// Entity types that can have weapons assigned
export const ARMED_ENTITY_TYPES = ['aircraft', 'sam', 'aaa', 'naval'];

// Weapon category → which entity types can use them
export const WEAPON_CATEGORY_USERS = {
  'air-to-air': ['aircraft'],
  'agm': ['aircraft'],
  'bomb': ['aircraft'],
  'gun': ['aircraft'],
  'sam': ['sam'],
  'aaa': ['aaa'],
};

export function makeDefaultWeapon() {
  return {
    id: '',
    name: '',
    category: 'agm',
    range: 0,
    damage: 0,
    ammoCapacity: 1,
    rateOfFire: 1,
    speed: 0,
    maxVelocity: 0,
    agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 },
    effects: {
      areaOfEffect: 0,
      armorPenetration: 0,
    },
    playerSelectable: false,
    playerDefault: false,
  };
}

// --- Default weapon presets ---

export const DEFAULT_WEAPONS = [
  // === Air-to-Air Missiles === (agility in deg/s: maxPitch, maxYaw, maxRoll)
  { id: 'aim-9m', name: 'AIM-9M Sidewinder', category: 'air-to-air', range: 18000, damage: 85, ammoCapacity: 2, rateOfFire: 0.5, speed: 900, maxVelocity: 920, agility: { maxPitch: 40, maxYaw: 40, maxRoll: 120 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: true, playerDefault: true },
  { id: 'aim-9x', name: 'AIM-9X Sidewinder II', category: 'air-to-air', range: 26000, damage: 90, ammoCapacity: 2, rateOfFire: 0.5, speed: 1000, maxVelocity: 1020, agility: { maxPitch: 60, maxYaw: 60, maxRoll: 180 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: true, playerDefault: false },
  { id: 'aim-120b', name: 'AIM-120B AMRAAM', category: 'air-to-air', range: 75000, damage: 95, ammoCapacity: 4, rateOfFire: 0.3, speed: 1400, maxVelocity: 1470, agility: { maxPitch: 35, maxYaw: 35, maxRoll: 100 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: true, playerDefault: true },
  { id: 'aim-120c', name: 'AIM-120C-7 AMRAAM', category: 'air-to-air', range: 105000, damage: 95, ammoCapacity: 4, rateOfFire: 0.3, speed: 1400, maxVelocity: 1470, agility: { maxPitch: 38, maxYaw: 38, maxRoll: 110 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: true, playerDefault: false },
  { id: 'aim-7m', name: 'AIM-7M Sparrow', category: 'air-to-air', range: 45000, damage: 80, ammoCapacity: 4, rateOfFire: 0.4, speed: 1200, maxVelocity: 1250, agility: { maxPitch: 25, maxYaw: 25, maxRoll: 80 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: true, playerDefault: false },
  { id: 'r-73', name: 'R-73 (AA-11 Archer)', category: 'air-to-air', range: 20000, damage: 82, ammoCapacity: 2, rateOfFire: 0.5, speed: 900, maxVelocity: 920, agility: { maxPitch: 50, maxYaw: 50, maxRoll: 150 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'r-27r', name: 'R-27R (AA-10 Alamo-A)', category: 'air-to-air', range: 60000, damage: 88, ammoCapacity: 2, rateOfFire: 0.3, speed: 1300, maxVelocity: 1350, agility: { maxPitch: 28, maxYaw: 28, maxRoll: 85 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'r-77', name: 'R-77 (AA-12 Adder)', category: 'air-to-air', range: 80000, damage: 90, ammoCapacity: 4, rateOfFire: 0.3, speed: 1400, maxVelocity: 1450, agility: { maxPitch: 35, maxYaw: 35, maxRoll: 100 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },

  // === Air-to-Ground Missiles (AGM) ===
  { id: 'agm-65d', name: 'AGM-65D Maverick (IR)', category: 'agm', range: 22000, damage: 120, ammoCapacity: 2, rateOfFire: 0.2, speed: 320, maxVelocity: 340, agility: { maxPitch: 15, maxYaw: 15, maxRoll: 40 }, effects: { areaOfEffect: 5, armorPenetration: 80 }, playerSelectable: true, playerDefault: true },
  { id: 'agm-65g', name: 'AGM-65G Maverick (IR Heavy)', category: 'agm', range: 22000, damage: 150, ammoCapacity: 2, rateOfFire: 0.2, speed: 320, maxVelocity: 340, agility: { maxPitch: 14, maxYaw: 14, maxRoll: 35 }, effects: { areaOfEffect: 8, armorPenetration: 95 }, playerSelectable: true, playerDefault: false },
  { id: 'agm-88', name: 'AGM-88 HARM', category: 'agm', range: 80000, damage: 100, ammoCapacity: 2, rateOfFire: 0.2, speed: 660, maxVelocity: 700, agility: { maxPitch: 20, maxYaw: 20, maxRoll: 60 }, effects: { areaOfEffect: 10, armorPenetration: 40 }, playerSelectable: true, playerDefault: false },
  { id: 'kh-29t', name: 'Kh-29T (AS-14 Kedge)', category: 'agm', range: 12000, damage: 140, ammoCapacity: 2, rateOfFire: 0.2, speed: 300, maxVelocity: 320, agility: { maxPitch: 12, maxYaw: 12, maxRoll: 30 }, effects: { areaOfEffect: 8, armorPenetration: 75 }, playerSelectable: false, playerDefault: false },
  { id: 'kh-25ml', name: 'Kh-25ML (AS-10 Karen)', category: 'agm', range: 10000, damage: 90, ammoCapacity: 4, rateOfFire: 0.3, speed: 280, maxVelocity: 300, agility: { maxPitch: 12, maxYaw: 12, maxRoll: 30 }, effects: { areaOfEffect: 6, armorPenetration: 50 }, playerSelectable: false, playerDefault: false },

  // === Bombs ===
  { id: 'mk-82', name: 'Mk 82 (500 lb)', category: 'bomb', range: 0, damage: 130, ammoCapacity: 6, rateOfFire: 2, speed: 0, maxVelocity: 0, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 25, armorPenetration: 50 }, playerSelectable: true, playerDefault: true },
  { id: 'mk-84', name: 'Mk 84 (2000 lb)', category: 'bomb', range: 0, damage: 250, ammoCapacity: 2, rateOfFire: 1, speed: 0, maxVelocity: 0, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 50, armorPenetration: 85 }, playerSelectable: true, playerDefault: false },
  { id: 'gbu-12', name: 'GBU-12 Paveway II', category: 'bomb', range: 15000, damage: 140, ammoCapacity: 4, rateOfFire: 1, speed: 0, maxVelocity: 0, agility: { maxPitch: 5, maxYaw: 5, maxRoll: 0 }, effects: { areaOfEffect: 20, armorPenetration: 60 }, playerSelectable: true, playerDefault: false },
  { id: 'gbu-24', name: 'GBU-24 Paveway III', category: 'bomb', range: 20000, damage: 260, ammoCapacity: 2, rateOfFire: 0.5, speed: 0, maxVelocity: 0, agility: { maxPitch: 6, maxYaw: 6, maxRoll: 0 }, effects: { areaOfEffect: 45, armorPenetration: 90 }, playerSelectable: true, playerDefault: false },
  { id: 'cbu-87', name: 'CBU-87 CEM (Cluster)', category: 'bomb', range: 0, damage: 80, ammoCapacity: 4, rateOfFire: 1.5, speed: 0, maxVelocity: 0, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 120, armorPenetration: 20 }, playerSelectable: true, playerDefault: false },
  { id: 'fab-500', name: 'FAB-500 (500 kg)', category: 'bomb', range: 0, damage: 160, ammoCapacity: 4, rateOfFire: 2, speed: 0, maxVelocity: 0, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 35, armorPenetration: 55 }, playerSelectable: false, playerDefault: false },

  // === Guns ===
  { id: 'm61a1', name: 'M61A1 Vulcan (20mm)', category: 'gun', range: 2000, damage: 20, ammoCapacity: 500, rateOfFire: 100, speed: 1050, maxVelocity: 1050, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 0, armorPenetration: 15 }, playerSelectable: true, playerDefault: true },
  { id: 'gsh-30-1', name: 'GSh-30-1 (30mm)', category: 'gun', range: 1800, damage: 35, ammoCapacity: 150, rateOfFire: 25, speed: 870, maxVelocity: 870, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 0, armorPenetration: 25 }, playerSelectable: false, playerDefault: false },

  // === SAM Systems ===
  { id: 'sa-2', name: 'S-75 Dvina (SA-2 Guideline)', category: 'sam', range: 45000, damage: 200, ammoCapacity: 1, rateOfFire: 0.05, speed: 1100, maxVelocity: 1150, agility: { maxPitch: 12, maxYaw: 12, maxRoll: 30 }, effects: { areaOfEffect: 65, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'sa-3', name: 'S-125 Neva (SA-3 Goa)', category: 'sam', range: 25000, damage: 150, ammoCapacity: 2, rateOfFire: 0.1, speed: 900, maxVelocity: 950, agility: { maxPitch: 18, maxYaw: 18, maxRoll: 50 }, effects: { areaOfEffect: 40, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'sa-6', name: '2K12 Kub (SA-6 Gainful)', category: 'sam', range: 24000, damage: 160, ammoCapacity: 3, rateOfFire: 0.15, speed: 1000, maxVelocity: 1050, agility: { maxPitch: 20, maxYaw: 20, maxRoll: 55 }, effects: { areaOfEffect: 50, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'sa-8', name: '9K33 Osa (SA-8 Gecko)', category: 'sam', range: 12000, damage: 100, ammoCapacity: 6, rateOfFire: 0.25, speed: 700, maxVelocity: 730, agility: { maxPitch: 22, maxYaw: 22, maxRoll: 60 }, effects: { areaOfEffect: 20, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'sa-10', name: 'S-300PM (SA-10 Grumble)', category: 'sam', range: 150000, damage: 250, ammoCapacity: 4, rateOfFire: 0.1, speed: 2000, maxVelocity: 2100, agility: { maxPitch: 30, maxYaw: 30, maxRoll: 90 }, effects: { areaOfEffect: 70, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'sa-11', name: '9K37 Buk (SA-11 Gadfly)', category: 'sam', range: 42000, damage: 180, ammoCapacity: 4, rateOfFire: 0.15, speed: 1200, maxVelocity: 1280, agility: { maxPitch: 25, maxYaw: 25, maxRoll: 70 }, effects: { areaOfEffect: 55, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'sa-15', name: '9K331 Tor (SA-15 Gauntlet)', category: 'sam', range: 12000, damage: 110, ammoCapacity: 8, rateOfFire: 0.3, speed: 850, maxVelocity: 900, agility: { maxPitch: 28, maxYaw: 28, maxRoll: 80 }, effects: { areaOfEffect: 15, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'patriot', name: 'MIM-104 Patriot', category: 'sam', range: 160000, damage: 230, ammoCapacity: 4, rateOfFire: 0.08, speed: 1700, maxVelocity: 1800, agility: { maxPitch: 28, maxYaw: 28, maxRoll: 85 }, effects: { areaOfEffect: 60, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'hawk', name: 'MIM-23 Hawk', category: 'sam', range: 45000, damage: 140, ammoCapacity: 3, rateOfFire: 0.12, speed: 900, maxVelocity: 950, agility: { maxPitch: 20, maxYaw: 20, maxRoll: 55 }, effects: { areaOfEffect: 35, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },
  { id: 'stinger', name: 'FIM-92 Stinger (MANPADS)', category: 'sam', range: 4800, damage: 60, ammoCapacity: 1, rateOfFire: 0.1, speed: 700, maxVelocity: 740, agility: { maxPitch: 22, maxYaw: 22, maxRoll: 60 }, effects: { areaOfEffect: 0, armorPenetration: 0 }, playerSelectable: false, playerDefault: false },

  // === AAA Systems ===
  { id: 'zsu-23-4', name: 'ZSU-23-4 Shilka', category: 'aaa', range: 2500, damage: 15, ammoCapacity: 2000, rateOfFire: 58, speed: 970, maxVelocity: 970, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 0, armorPenetration: 20 }, playerSelectable: false, playerDefault: false },
  { id: 'zu-23-2', name: 'ZU-23-2 (towed)', category: 'aaa', range: 2000, damage: 12, ammoCapacity: 100, rateOfFire: 16, speed: 970, maxVelocity: 970, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 0, armorPenetration: 15 }, playerSelectable: false, playerDefault: false },
  { id: 's-60', name: 'S-60 (57mm)', category: 'aaa', range: 6000, damage: 40, ammoCapacity: 60, rateOfFire: 2, speed: 1000, maxVelocity: 1000, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 3, armorPenetration: 35 }, playerSelectable: false, playerDefault: false },
  { id: 'bofors-l70', name: 'Bofors 40mm L/70', category: 'aaa', range: 4000, damage: 25, ammoCapacity: 120, rateOfFire: 4, speed: 1005, maxVelocity: 1005, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 2, armorPenetration: 25 }, playerSelectable: false, playerDefault: false },
  { id: 'gepard', name: 'Flakpanzer Gepard (35mm)', category: 'aaa', range: 3500, damage: 20, ammoCapacity: 640, rateOfFire: 18, speed: 1175, maxVelocity: 1175, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 0, armorPenetration: 22 }, playerSelectable: false, playerDefault: false },
  { id: 'vulcan-m163', name: 'M163 VADS (20mm Vulcan)', category: 'aaa', range: 1800, damage: 10, ammoCapacity: 2100, rateOfFire: 50, speed: 1050, maxVelocity: 1050, agility: { maxPitch: 0, maxYaw: 0, maxRoll: 0 }, effects: { areaOfEffect: 0, armorPenetration: 12 }, playerSelectable: false, playerDefault: false },
];

// Aircraft subtype presets — apply via AIRCRAFT_PRESETS[subtype] to override default params
export const AIRCRAFT_PRESETS = {
  f15:     { speed: 270, maxSpeed: 830, altitude: 5000, fuelCapacity: 6100, fuelConsumption: 1.8, turnRate: 14, climbRate: 254, radarRange: 160000 },
  f16:     { speed: 250, maxSpeed: 700, altitude: 4000, fuelCapacity: 3160, fuelConsumption: 1.3, turnRate: 18, climbRate: 254, radarRange: 105000 },
  mig29:   { speed: 260, maxSpeed: 720, altitude: 4500, fuelCapacity: 3500, fuelConsumption: 1.5, turnRate: 16, climbRate: 330, radarRange: 80000 },
  su27:    { speed: 280, maxSpeed: 750, altitude: 5000, fuelCapacity: 9400, fuelConsumption: 2.0, turnRate: 14, climbRate: 300, radarRange: 120000 },
  mig21:   { speed: 250, maxSpeed: 650, altitude: 3000, fuelCapacity: 2600, fuelConsumption: 1.2, turnRate: 12, climbRate: 225, radarRange: 30000 },
  f4:      { speed: 260, maxSpeed: 720, altitude: 4000, fuelCapacity: 7550, fuelConsumption: 2.2, turnRate: 10, climbRate: 210, radarRange: 70000 },
  tornado: { speed: 250, maxSpeed: 730, altitude: 4000, fuelCapacity: 5500, fuelConsumption: 1.7, turnRate: 11, climbRate: 200, radarRange: 90000 },
};

export const ENTITY_TYPES = {
  aircraft: {
    label: 'Aircraft',
    short: 'AC',
    subtypes: ['f15', 'f16', 'mig29', 'su27', 'mig21', 'f4', 'tornado'],
    defaults: { subtype: 'mig29', faction: 'enemy', health: 100, maxHealth: 100, scale: 1, modelId: '' },
    params: [
      { key: 'speed', label: 'Speed (m/s)', type: 'number', default: 250, min: 0 },
      { key: 'maxSpeed', label: 'Max Speed (m/s)', type: 'number', default: 700, min: 0 },
      { key: 'altitude', label: 'Altitude (m)', type: 'number', default: 3000, min: 0 },
      { key: 'fuelCapacity', label: 'Fuel Capacity (kg)', type: 'number', default: 6100, min: 0 },
      { key: 'fuelConsumption', label: 'Fuel Consumption (kg/s)', type: 'number', default: 1.5, min: 0, step: 0.1 },
      { key: 'turnRate', label: 'Turn Rate (°/s)', type: 'number', default: 12, min: 0, step: 0.5 },
      { key: 'climbRate', label: 'Climb Rate (m/s)', type: 'number', default: 250, min: 0 },
      { key: 'radarRange', label: 'Radar Range (m)', type: 'number', default: 100000, min: 0 },
    ],
  },
  sam: {
    label: 'SAM',
    short: 'SAM',
    subtypes: ['sa2', 'sa6', 'sa8', 'hawk'],
    defaults: { subtype: 'sa6', faction: 'enemy', health: 80, maxHealth: 80, scale: 1, modelId: '' },
    params: [
      { key: 'detectionRange', label: 'Detection Range (m)', type: 'number', default: 8000, min: 0 },
      { key: 'engagementRange', label: 'Engagement Range (m)', type: 'number', default: 6000, min: 0 },
      { key: 'maxAltitude', label: 'Max Altitude (m)', type: 'number', default: 5000, min: 0 },
      { key: 'fireRate', label: 'Fire Rate', type: 'number', default: 2, min: 0, step: 0.1 },
      { key: 'accuracy', label: 'Accuracy (0–1)', type: 'number', default: 0.4, min: 0, max: 1, step: 0.01 },
      { key: 'reloadTime', label: 'Reload Time (s)', type: 'number', default: 10, min: 0, step: 0.5 },
    ],
  },
  aaa: {
    label: 'AAA',
    short: 'AAA',
    subtypes: ['zsu23', 'zu23', 'bofors'],
    defaults: { subtype: 'zsu23', faction: 'enemy', health: 50, maxHealth: 50, scale: 1, modelId: '' },
    params: [
      { key: 'detectionRange', label: 'Detection Range (m)', type: 'number', default: 5000, min: 0 },
      { key: 'engagementRange', label: 'Engagement Range (m)', type: 'number', default: 3000, min: 0 },
      { key: 'maxAltitude', label: 'Max Altitude (m)', type: 'number', default: 3000, min: 0 },
      { key: 'fireRate', label: 'Fire Rate (rps)', type: 'number', default: 10, min: 0, step: 0.1 },
      { key: 'accuracy', label: 'Accuracy (0–1)', type: 'number', default: 0.2, min: 0, max: 1, step: 0.01 },
      { key: 'burstDuration', label: 'Burst Duration (s)', type: 'number', default: 2, min: 0, step: 0.1 },
    ],
  },
  structure: {
    label: 'Structure',
    short: 'STR',
    subtypes: ['radar', 'bridge', 'depot', 'bunker'],
    defaults: { subtype: 'radar', faction: 'enemy', health: 100, maxHealth: 100, scale: 1, modelId: '' },
    params: [],
  },
  airbase: {
    label: 'Airbase',
    short: 'AB',
    subtypes: ['international-airport', 'military-airfield', 'forward-strip'],
    defaults: { subtype: 'international-airport', faction: 'friendly', health: 100, maxHealth: 100, scale: 1, modelId: '' },
    params: [
      { key: 'minx', label: 'Area Min X (m)', type: 'number', default: 0 },
      { key: 'maxx', label: 'Area Max X (m)', type: 'number', default: 0 },
      { key: 'minz', label: 'Area Min Z (m)', type: 'number', default: 0 },
      { key: 'maxz', label: 'Area Max Z (m)', type: 'number', default: 0 },
    ],
  },
  carrier: {
    label: 'Carrier',
    short: 'CV',
    subtypes: ['carrier'],
    defaults: { subtype: 'carrier', faction: 'friendly', health: 100, maxHealth: 100, scale: 1, modelId: 'assets/models/carrier-s.glb' },
    params: [
      { key: 'minx', label: 'Area Min X (m)', type: 'number', default: 0 },
      { key: 'maxx', label: 'Area Max X (m)', type: 'number', default: 0 },
      { key: 'minz', label: 'Area Min Z (m)', type: 'number', default: 0 },
      { key: 'maxz', label: 'Area Max Z (m)', type: 'number', default: 0 },
    ],
  },
  naval: {
    label: 'Naval',
    short: 'NV',
    subtypes: ['destroyer', 'cruiser', 'frigate'],
    defaults: { subtype: 'destroyer', faction: 'enemy', health: 100, maxHealth: 100, scale: 1, modelId: '' },
    params: [],
  },
  waypoint: {
    label: 'Waypoint',
    short: 'WP',
    subtypes: [],
    defaults: { subtype: '', faction: 'neutral', health: 0, maxHealth: 0, scale: 1, modelId: '' },
    params: [
      { key: 'waypointLabel', label: 'Label', type: 'text', default: '' },
    ],
    hideStats: true,
  },
};

// --- Canvas marker drawing ---

const FACTION_COLORS = { friendly: '#00FF55', enemy: '#FF2020', neutral: '#FFD600' };

export function getFactionColor(faction) {
  return FACTION_COLORS[faction] || FACTION_COLORS.neutral;
}

export function drawEntityMarker(ctx, type, faction, x, y, size, selected, heading) {
  const color = getFactionColor(faction);
  ctx.save();
  ctx.translate(x, y);

  if (selected) {
    ctx.strokeStyle = '#ffffff';
    ctx.lineWidth = 2.5;
    ctx.setLineDash([4, 3]);
    ctx.beginPath();
    ctx.arc(0, 0, size + 5, 0, Math.PI * 2);
    ctx.stroke();
    ctx.setLineDash([]);
  }

  // dark halo for contrast against any background
  ctx.shadowColor = 'rgba(0,0,0,0.7)';
  ctx.shadowBlur = 4;

  ctx.fillStyle = color;
  ctx.strokeStyle = color;
  ctx.lineWidth = 2.5;

  // rotate by heading if provided
  if (heading) ctx.rotate(-heading * Math.PI / 180);

  switch (type) {
    case 'aircraft': _triangle(ctx, size); break;
    case 'sam': _samMarker(ctx, size); break;
    case 'aaa': _aaaBurst(ctx, size); break;
    case 'structure': _square(ctx, size); break;
    case 'airbase': _runway(ctx, size); break;
    case 'carrier': _diamond(ctx, size); break;
    case 'naval': _circle(ctx, size); break;
    case 'waypoint': _crosshair(ctx, size); break;
  }

  ctx.restore();
}

function _triangle(ctx, s) {
  ctx.beginPath();
  ctx.moveTo(0, -s);
  ctx.lineTo(-s * 0.7, s * 0.7);
  ctx.lineTo(s * 0.7, s * 0.7);
  ctx.closePath();
  ctx.fill();
}

// SAM: upward-pointing missile/arrow shape
function _samMarker(ctx, s) {
  ctx.beginPath();
  ctx.moveTo(0, -s * 1.2);
  ctx.lineTo(-s * 0.4, s * 0.2);
  ctx.lineTo(-s * 0.8, s * 1.0);
  ctx.lineTo(0, s * 0.5);
  ctx.lineTo(s * 0.8, s * 1.0);
  ctx.lineTo(s * 0.4, s * 0.2);
  ctx.closePath();
  ctx.fill();
}

// AAA: starburst / flak burst
function _aaaBurst(ctx, s) {
  const spikes = 6;
  ctx.beginPath();
  for (let i = 0; i < spikes * 2; i++) {
    const r = i % 2 === 0 ? s : s * 0.45;
    const a = (i * Math.PI) / spikes - Math.PI / 2;
    const x = Math.cos(a) * r;
    const y = Math.sin(a) * r;
    if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  }
  ctx.closePath();
  ctx.fill();
}

function _square(ctx, s) {
  ctx.fillRect(-s, -s, s * 2, s * 2);
}

function _runway(ctx, s) {
  ctx.fillRect(-s * 0.25, -s * 1.2, s * 0.5, s * 2.4);
  ctx.fillRect(-s, -s * 0.15, s * 2, s * 0.3);
}

function _diamond(ctx, s) {
  ctx.beginPath();
  ctx.moveTo(0, -s); ctx.lineTo(s * 0.7, 0);
  ctx.lineTo(0, s); ctx.lineTo(-s * 0.7, 0);
  ctx.closePath();
  ctx.fill();
}

function _circle(ctx, s) {
  ctx.beginPath();
  ctx.arc(0, 0, s * 0.8, 0, Math.PI * 2);
  ctx.fill();
}

function _crosshair(ctx, s) {
  ctx.beginPath();
  ctx.moveTo(0, -s); ctx.lineTo(0, s);
  ctx.moveTo(-s, 0); ctx.lineTo(s, 0);
  ctx.stroke();
  ctx.beginPath();
  ctx.arc(0, 0, s * 0.35, 0, Math.PI * 2);
  ctx.stroke();
}

// draw start position marker
export function drawStartMarker(ctx, x, y, size, heading) {
  ctx.save();
  ctx.translate(x, y);
  if (heading) ctx.rotate(-heading * Math.PI / 180);

  // outer ring
  ctx.strokeStyle = '#00E5FF';
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.arc(0, 0, size + 2, 0, Math.PI * 2);
  ctx.stroke();

  // arrow
  ctx.fillStyle = '#00E5FF';
  ctx.beginPath();
  ctx.moveTo(0, -size * 1.2);
  ctx.lineTo(-size * 0.5, size * 0.4);
  ctx.lineTo(0, size * 0.1);
  ctx.lineTo(size * 0.5, size * 0.4);
  ctx.closePath();
  ctx.fill();

  ctx.restore();
}

// draw detection/engagement range rings for AAA/SAM
export function drawRangeRings(ctx, x, y, params, metersToPixels, isSelected = false) {
  ctx.save();
  ctx.translate(x, y);

  // brighter for selected, visible for others
  const detAlpha = isSelected ? 0.7 : 0.45;
  const engAlpha = isSelected ? 0.8 : 0.5;
  const lw = isSelected ? 2.2 : 1.5;

  if (params.detectionRange) {
    const r = params.detectionRange * metersToPixels;
    ctx.strokeStyle = `rgba(255, 210, 0, ${detAlpha})`;
    ctx.lineWidth = lw;
    ctx.setLineDash([6, 4]);
    ctx.beginPath();
    ctx.arc(0, 0, r, 0, Math.PI * 2);
    ctx.stroke();
  }

  if (params.engagementRange) {
    const r = params.engagementRange * metersToPixels;
    ctx.strokeStyle = `rgba(255, 50, 50, ${engAlpha})`;
    ctx.lineWidth = lw * 1.2;
    ctx.setLineDash([4, 3]);
    ctx.beginPath();
    ctx.arc(0, 0, r, 0, Math.PI * 2);
    ctx.stroke();

    // filled engagement zone for selected
    if (isSelected) {
      ctx.fillStyle = `rgba(255, 50, 50, 0.12)`;
      ctx.beginPath();
      ctx.arc(0, 0, r, 0, Math.PI * 2);
      ctx.fill();
    }
  }

  ctx.setLineDash([]);
  ctx.restore();
}
