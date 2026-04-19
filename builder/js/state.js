// Central scenario state store with event bus.
// All UI modules listen for events and update reactively.

class EventBus {
  constructor() { this._listeners = {}; }
  on(event, fn) { (this._listeners[event] ||= []).push(fn); }
  off(event, fn) {
    const arr = this._listeners[event];
    if (arr) this._listeners[event] = arr.filter(f => f !== fn);
  }
  emit(event, data) {
    (this._listeners[event] || []).forEach(fn => fn(data));
  }
}

let _nextEntityNum = 1;

function makeEntityId(type) {
  return `${type}-${String(_nextEntityNum++).padStart(2, '0')}`;
}

export class ScenarioState extends EventBus {
  constructor() {
    super();

    this.scenario = {
      id: '',
      name: '',
      description: '',
      difficulty: 'easy',
      weather: 'sunny',
      season: 'summer',
      timeOfDay: 'day',
      theater: '',
      skyColor: [0, 121, 241, 255],
      start: {
        position: { x: 0, y: 0, z: 0 },
        heading: 0,
        speed: 0,
        altitude: 0,
        fuel: 3500,
        carrier: false,
        availableWeapons: [],
        defaultWeapons: [],
      },
      entities: [],
      objectives: [],
      completion: {
        success: 'all_required_objectives',
        failure: ['player_destroyed', 'player_ejected'],
      },
    };

    this.weapons = [];  // weapon type definitions (shared across scenario)

    this.map = {
      image: null,       // HTMLImageElement
      fileHandle: null,  // FileSystemFileHandle (persisted in IndexedDB)
      fileName: '',
      ratio: 10,         // meters per pixel
      width: 0,          // image pixel width
      height: 0,         // image pixel height
    };

    this.ui = {
      selectedEntityId: null,
      selectedObjectiveId: null,
      tool: 'select',         // 'select' | 'place' | 'start'
      placingType: null,      // entity type string when tool='place'
      inspectorTab: 'scenario', // 'scenario' | 'entity' | 'objective'
      grayscale: true,
      showGrid: false,
    };

    this.settings = {
      gridSizeMeters: 1000,       // base grid spacing in meters
      defaultRatio: 10,           // default meters/pixel for new maps
      snapToGrid: false,          // snap entity placement to grid
      markerScale: 1.0,           // entity marker size multiplier
      showLabels: true,           // show entity ID labels on map
      showRangeRings: true,       // show SAM/AAA range rings
      showWaypointPaths: true,    // show aircraft waypoint paths
      autoSave: true,             // auto-save session changes
    };

    this.camera = { x: 0, y: 0, zoom: 1 };
  }

  // --- Map ---

  setMapImage(image, fileName, fileHandle = null) {
    this.map.image = image;
    this.map.fileName = fileName;
    this.map.fileHandle = fileHandle;
    this.map.width = image.width;
    this.map.height = image.height;
    // center camera on map
    this.camera.x = 0;
    this.camera.y = 0;
    this.camera.zoom = 1;
    this.emit('map-changed');
    this.emit('camera-changed');
  }

  setMapRatio(ratio) {
    this.map.ratio = Math.max(0.01, ratio);
    this.emit('map-changed');
    this.emit('canvas-redraw');
  }

  // --- Camera ---

  panCamera(dx, dy) {
    this.camera.x += dx;
    this.camera.y += dy;
    this.emit('camera-changed');
  }

  zoomCamera(factor, pivotX, pivotY) {
    const oldZoom = this.camera.zoom;
    this.camera.zoom = Math.max(0.05, Math.min(20, this.camera.zoom * factor));
    const ratio = 1 - this.camera.zoom / oldZoom;
    this.camera.x += (pivotX - this.camera.x) * ratio;
    this.camera.y += (pivotY - this.camera.y) * ratio;
    this.emit('camera-changed');
  }

  setZoom(zoom) {
    this.camera.zoom = Math.max(0.05, Math.min(20, zoom));
    this.emit('camera-changed');
  }

  // --- Coordinate conversions ---
  // Map-space origin is the image center.
  // Image pixel (0,0) = map-space (-imgW/2, -imgH/2).

  // World position → map-space coordinate (centered)
  worldToPixel(worldX, worldZ) {
    return {
      px: worldX / this.map.ratio - this.map.width / 2,
      py: worldZ / this.map.ratio - this.map.height / 2,
    };
  }

  // Map-space coordinate → world position
  pixelToWorld(px, py) {
    return {
      x: (px + this.map.width / 2) * this.map.ratio,
      z: (py + this.map.height / 2) * this.map.ratio,
    };
  }

  // Screen position → map pixel position (accounting for camera)
  screenToPixel(screenX, screenY, canvasRect) {
    const cx = canvasRect.width / 2 + this.camera.x * this.camera.zoom;
    const cy = canvasRect.height / 2 + this.camera.y * this.camera.zoom;
    return {
      px: (screenX - cx) / this.camera.zoom,
      py: (screenY - cy) / this.camera.zoom,
    };
  }

  // Map pixel position → screen position
  pixelToScreen(px, py, canvasRect) {
    const cx = canvasRect.width / 2 + this.camera.x * this.camera.zoom;
    const cy = canvasRect.height / 2 + this.camera.y * this.camera.zoom;
    return {
      sx: cx + px * this.camera.zoom,
      sy: cy + py * this.camera.zoom,
    };
  }

  // --- UI state ---

  setTool(tool, placingType = null) {
    this.ui.tool = tool;
    this.ui.placingType = placingType;
    this.emit('tool-changed');
  }

  selectEntity(id) {
    this.ui.selectedEntityId = id;
    this.ui.selectedObjectiveId = null;
    this.ui.inspectorTab = id ? 'entity' : 'scenario';
    this.emit('selection-changed');
    this.emit('canvas-redraw');
  }

  selectObjective(id) {
    this.ui.selectedObjectiveId = id;
    this.ui.selectedEntityId = null;
    this.ui.inspectorTab = id ? 'objective' : 'scenario';
    this.emit('selection-changed');
  }

  // --- Scenario metadata ---

  updateScenario(changes) {
    Object.assign(this.scenario, changes);
    this.emit('scenario-changed');
  }

  updateStart(changes) {
    Object.assign(this.scenario.start, changes);
    this.emit('start-changed');
    this.emit('canvas-redraw');
  }

  updateStartPosition(x, z) {
    this.scenario.start.position.x = x;
    this.scenario.start.position.z = z;
    this.emit('start-changed');
    this.emit('canvas-redraw');
  }

  // --- Entities ---

  addEntity(type, worldX, worldZ, overrides = {}) {
    const { ENTITY_TYPES } = _defs;
    const def = ENTITY_TYPES[type];
    if (!def) return null;

    const id = overrides.id || makeEntityId(type);
    const params = {};
    for (const p of def.params) params[p.key] = p.default;

    const entity = {
      id,
      type,
      subtype: def.defaults.subtype,
      faction: def.defaults.faction,
      state: 'active',
      position: { x: worldX, y: 0, z: worldZ },
      heading: 0,
      health: def.defaults.health,
      maxHealth: def.defaults.maxHealth,
      scale: def.defaults.scale,
      modelId: def.defaults.modelId,
      params,
      weapons: [],
      ...(type === 'aircraft' ? { waypoints: [] } : {}),
      ...overrides,
    };

    this.scenario.entities.push(entity);
    this.emit('entities-changed');
    this.emit('canvas-redraw');
    return entity;
  }

  removeEntity(id) {
    const idx = this.scenario.entities.findIndex(e => e.id === id);
    if (idx === -1) return;
    this.scenario.entities.splice(idx, 1);
    if (this.ui.selectedEntityId === id) {
      this.ui.selectedEntityId = null;
      this.ui.inspectorTab = 'scenario';
      this.emit('selection-changed');
    }
    this.emit('entities-changed');
    this.emit('canvas-redraw');
  }

  updateEntity(id, changes) {
    const entity = this.scenario.entities.find(e => e.id === id);
    if (!entity) return;

    for (const [key, value] of Object.entries(changes)) {
      if (key === 'position') {
        Object.assign(entity.position, value);
      } else if (key === 'params') {
        Object.assign(entity.params, value);
      } else {
        entity[key] = value;
      }
    }
    this.emit('entity-updated', id);
    this.emit('canvas-redraw');
  }

  getEntity(id) {
    return this.scenario.entities.find(e => e.id === id);
  }

  getSelectedEntity() {
    if (!this.ui.selectedEntityId) return null;
    return this.getEntity(this.ui.selectedEntityId);
  }

  // --- Objectives ---

  addObjective() {
    const id = `obj-${String(this.scenario.objectives.length + 1).padStart(2, '0')}`;
    const obj = {
      id,
      type: 'destroy',
      target: '',
      label: '',
      required: true,
      order: this.scenario.objectives.length + 1,
      params: {},
    };
    this.scenario.objectives.push(obj);
    this.emit('objectives-changed');
    return obj;
  }

  removeObjective(id) {
    const idx = this.scenario.objectives.findIndex(o => o.id === id);
    if (idx === -1) return;
    this.scenario.objectives.splice(idx, 1);
    if (this.ui.selectedObjectiveId === id) {
      this.ui.selectedObjectiveId = null;
      this.ui.inspectorTab = 'scenario';
      this.emit('selection-changed');
    }
    this.emit('objectives-changed');
  }

  updateObjective(id, changes) {
    const obj = this.scenario.objectives.find(o => o.id === id);
    if (!obj) return;
    Object.assign(obj, changes);
    this.emit('objective-updated', id);
  }

  getObjective(id) {
    return this.scenario.objectives.find(o => o.id === id);
  }

  getSelectedObjective() {
    if (!this.ui.selectedObjectiveId) return null;
    return this.getObjective(this.ui.selectedObjectiveId);
  }

  // --- Weapons ---

  addWeapon(data = {}) {
    const { makeDefaultWeapon } = _defs;
    const weapon = { ...makeDefaultWeapon(), ...data };
    if (!weapon.id) weapon.id = `wpn-${String(this.weapons.length + 1).padStart(2, '0')}`;
    this.weapons.push(weapon);
    this.emit('weapons-changed');
    return weapon;
  }

  removeWeapon(id) {
    const idx = this.weapons.findIndex(w => w.id === id);
    if (idx === -1) return;
    this.weapons.splice(idx, 1);
    // Remove from entity weapon lists
    for (const e of this.scenario.entities) {
      if (e.weapons) {
        e.weapons = e.weapons.filter(wid => wid !== id);
      }
    }
    // Remove from player loadout
    if (this.scenario.start.availableWeapons) {
      this.scenario.start.availableWeapons = this.scenario.start.availableWeapons.filter(wid => wid !== id);
    }
    if (this.scenario.start.defaultWeapons) {
      this.scenario.start.defaultWeapons = this.scenario.start.defaultWeapons.filter(wid => wid !== id);
    }
    this.emit('weapons-changed');
    this.emit('entities-changed');
    this.emit('start-changed');
  }

  updateWeapon(id, changes) {
    const weapon = this.weapons.find(w => w.id === id);
    if (!weapon) return;
    for (const [key, value] of Object.entries(changes)) {
      if (key === 'effects') {
        Object.assign(weapon.effects, value);
      } else if (key === 'agility') {
        if (!weapon.agility) weapon.agility = { maxPitch: 0, maxYaw: 0, maxRoll: 0 };
        Object.assign(weapon.agility, value);
      } else {
        weapon[key] = value;
      }
    }
    this.emit('weapon-updated', id);
  }

  getWeapon(id) {
    return this.weapons.find(w => w.id === id);
  }

  getWeaponsByCategory(category) {
    return this.weapons.filter(w => w.category === category);
  }

  getWeaponsForEntityType(entityType) {
    const { WEAPON_CATEGORY_USERS } = _defs;
    return this.weapons.filter(w => {
      const users = WEAPON_CATEGORY_USERS[w.category];
      return users && users.includes(entityType);
    });
  }

  // --- Reset ---

  reset() {
    _nextEntityNum = 1;
    this.scenario.id = '';
    this.scenario.name = '';
    this.scenario.description = '';
    this.scenario.difficulty = 'easy';
    this.scenario.weather = 'sunny';
    this.scenario.season = 'summer';
    this.scenario.timeOfDay = 'day';
    this.scenario.theater = '';
    this.scenario.skyColor = [0, 121, 241, 255];
    this.scenario.start = { position: { x: 0, y: 0, z: 0 }, heading: 0, speed: 0, altitude: 0, fuel: 3500, carrier: false, availableWeapons: [], defaultWeapons: [] };
    this.scenario.entities = [];
    this.scenario.objectives = [];
    this.scenario.completion = { success: 'all_required_objectives', failure: ['player_destroyed', 'player_ejected'] };
    this.weapons = [];
    this.map.image = null;
    this.map.fileHandle = null;
    this.map.fileName = '';
    this.map.width = 0;
    this.map.height = 0;
    this.ui.selectedEntityId = null;
    this.ui.selectedObjectiveId = null;
    this.ui.tool = 'select';
    this.ui.placingType = null;
    this.ui.inspectorTab = 'scenario';
    this.camera = { x: 0, y: 0, zoom: 1 };
    this.emit('reset');
    this.emit('map-changed');
    this.emit('entities-changed');
    this.emit('objectives-changed');
    this.emit('weapons-changed');
    this.emit('selection-changed');
    this.emit('camera-changed');
    this.emit('scenario-changed');
    this.emit('start-changed');
  }

  // --- Import ---

  importJSON(json) {
    this.reset();
    const data = json.data || json;
    this.scenario.id = data.id || '';
    this.scenario.name = data.name || '';
    this.scenario.description = data.description || '';
    this.scenario.difficulty = data.difficulty || 'easy';
    this.scenario.weather = data.weather || 'sunny';
    this.scenario.season = data.season || 'summer';
    this.scenario.timeOfDay = data.timeOfDay || 'day';
    this.scenario.theater = data.theater || '';
    if (data.skyColor) this.scenario.skyColor = data.skyColor;
    if (data.start) Object.assign(this.scenario.start, data.start);
    if (data.entities) {
      this.scenario.entities = data.entities.map(e => ({
        id: e.id || '',
        type: e.type || 'structure',
        subtype: e.subtype || '',
        faction: e.faction || 'enemy',
        state: e.state || 'active',
        position: e.position || { x: 0, y: 0, z: 0 },
        heading: e.heading || 0,
        health: e.health ?? 100,
        maxHealth: e.maxHealth ?? 100,
        scale: e.scale ?? 1,
        modelId: e.modelId || e.model || '',
        params: e.params || {},
        weapons: e.weapons || [],
        ...(e.type === 'aircraft' ? { waypoints: e.waypoints || [] } : {}),
      }));
      _nextEntityNum = this.scenario.entities.length + 1;
    }
    if (data.objectives) this.scenario.objectives = data.objectives;
    if (data.completion) this.scenario.completion = data.completion;

    // Weapons
    if (data.weapons) {
      this.weapons = data.weapons.map(w => ({
        id: w.id || '',
        name: w.name || '',
        category: w.category || 'agm',
        range: w.range ?? 0,
        damage: w.damage ?? 0,
        ammoCapacity: w.ammoCapacity ?? 1,
        rateOfFire: w.rateOfFire ?? 1,
        speed: w.speed ?? 0,
        maxVelocity: w.maxVelocity ?? 0,
        agility: {
          maxPitch: w.agility?.maxPitch ?? 0,
          maxYaw: w.agility?.maxYaw ?? 0,
          maxRoll: w.agility?.maxRoll ?? 0,
        },
        effects: {
          areaOfEffect: w.effects?.areaOfEffect ?? 0,
          armorPenetration: w.effects?.armorPenetration ?? 0,
        },
        playerSelectable: w.playerSelectable ?? false,
        playerDefault: w.playerDefault ?? false,
      }));
    }

    // Start loadout
    if (data.start) {
      this.scenario.start.availableWeapons = data.start.availableWeapons || [];
      this.scenario.start.defaultWeapons = data.start.defaultWeapons || [];
    }

    this.emit('scenario-changed');
    this.emit('start-changed');
    this.emit('entities-changed');
    this.emit('objectives-changed');
    this.emit('weapons-changed');
    this.emit('canvas-redraw');
  }
}

// deferred reference to entity-defs (set in main.js to avoid circular import)
let _defs = null;
export function setDefs(defs) { _defs = defs; }

// --- IndexedDB persistence ---
// Stores scenario data + a FileSystemFileHandle for the map image.
// On restore the handle is used to re-read the file from disk (no duplication).

const DB_NAME = 'airborne-builder';
const DB_VERSION = 1;
const STORE_NAME = 'session';
const SESSION_KEY = 'current';

function _openDB() {
  return new Promise((resolve, reject) => {
    const req = indexedDB.open(DB_NAME, DB_VERSION);
    req.onupgradeneeded = () => {
      req.result.createObjectStore(STORE_NAME);
    };
    req.onsuccess = () => resolve(req.result);
    req.onerror = () => reject(req.error);
  });
}

function _dbPut(db, value) {
  return new Promise((resolve, reject) => {
    const tx = db.transaction(STORE_NAME, 'readwrite');
    tx.objectStore(STORE_NAME).put(value, SESSION_KEY);
    tx.oncomplete = () => resolve();
    tx.onerror = () => reject(tx.error);
  });
}

function _dbGet(db) {
  return new Promise((resolve, reject) => {
    const tx = db.transaction(STORE_NAME, 'readonly');
    const req = tx.objectStore(STORE_NAME).get(SESSION_KEY);
    req.onsuccess = () => resolve(req.result);
    req.onerror = () => reject(req.error);
  });
}

function _dbClear(db) {
  return new Promise((resolve, reject) => {
    const tx = db.transaction(STORE_NAME, 'readwrite');
    tx.objectStore(STORE_NAME).delete(SESSION_KEY);
    tx.oncomplete = () => resolve();
    tx.onerror = () => reject(tx.error);
  });
}

export async function saveSession(state) {
  try {
    const db = await _openDB();
    const data = {
      scenario: state.scenario,
      weapons: state.weapons,
      map: {
        fileHandle: state.map.fileHandle,  // structured-clonable
        fileName: state.map.fileName,
        ratio: state.map.ratio,
        width: state.map.width,
        height: state.map.height,
      },
      camera: { ...state.camera },
      settings: { ...state.settings },
    };
    await _dbPut(db, data);
  } catch (e) {
    console.warn('Session save failed:', e.message);
  }
}

/** Load an Image element from a FileSystemFileHandle, re-requesting permission if needed. */
export async function loadImageFromHandle(handle) {
  // Check / request read permission (requestPermission requires user gesture
  // but queryPermission doesn't — try query first, it's often still 'granted')
  let perm = await handle.queryPermission({ mode: 'read' });
  if (perm === 'prompt') {
    perm = await handle.requestPermission({ mode: 'read' });
  }
  if (perm !== 'granted') return null;

  const file = await handle.getFile();
  return new Promise((resolve) => {
    const url = URL.createObjectURL(file);
    const img = new Image();
    img.onload = () => { URL.revokeObjectURL(url); resolve(img); };
    img.onerror = () => { URL.revokeObjectURL(url); resolve(null); };
    img.src = url;
  });
}

export async function restoreSession(state) {
  try {
    const db = await _openDB();
    const data = await _dbGet(db);
    if (!data) return false;

    // Restore scenario
    Object.assign(state.scenario, data.scenario);
    if (data.scenario?.entities) {
      _nextEntityNum = data.scenario.entities.length + 1;
    }
    // Ensure loadout arrays exist
    if (!state.scenario.start.availableWeapons) state.scenario.start.availableWeapons = [];
    if (!state.scenario.start.defaultWeapons) state.scenario.start.defaultWeapons = [];

    // Restore weapons
    if (data.weapons) state.weapons = data.weapons;

    // Restore camera
    if (data.camera) Object.assign(state.camera, data.camera);

    // Restore settings
    if (data.settings) Object.assign(state.settings, data.settings);

    // Restore map metadata
    if (data.map) {
      state.map.fileName = data.map.fileName || '';
      state.map.ratio = data.map.ratio || 10;
      state.map.width = data.map.width || 0;
      state.map.height = data.map.height || 0;
      state.map.fileHandle = data.map.fileHandle || null;

      // Image is loaded separately via reloadMapFromHandle()
    }

    return true;
  } catch (e) {
    console.warn('Session restore failed:', e.message);
    return false;
  }
}

/** Re-read the map image from the stored file handle. Returns true on success. */
export async function reloadMapFromHandle(state) {
  const handle = state.map.fileHandle;
  if (!handle) return false;
  try {
    const img = await loadImageFromHandle(handle);
    if (!img) return false;
    state.map.image = img;
    state.map.width = img.width;
    state.map.height = img.height;
    state.emit('map-changed');
    state.emit('camera-changed');
    state.emit('canvas-redraw');
    return true;
  } catch (e) {
    console.warn('Failed to reload map from handle:', e.message);
    return false;
  }
}

export async function clearSession() {
  try {
    const db = await _openDB();
    await _dbClear(db);
  } catch (e) {
    console.warn('Session clear failed:', e.message);
  }
  try { localStorage.removeItem('airborne-builder-session'); } catch (_) {}
}
