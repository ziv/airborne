// Right inspector panel — context-sensitive property editor.

import {
  ENTITY_TYPES, FACTIONS, ENTITY_STATES, DIFFICULTIES, OBJECTIVE_TYPES,
  WEATHER, SEASONS, TIME_OF_DAY, ARMED_ENTITY_TYPES, AIRCRAFT_PRESETS,
} from './entity-defs.js';

function _rgbHex(c) {
  return [c[0], c[1], c[2]].map(v => v.toString(16).padStart(2, '0')).join('');
}

export class Inspector {
  constructor(el, state) {
    this.el = el;
    this.state = state;
    this._selfUpdate = false; // guard to prevent re-render while user is typing

    state.on('selection-changed', () => this.render());
    state.on('scenario-changed', () => { if (!this._selfUpdate && this.state.ui.inspectorTab === 'scenario') this.render(); });
    state.on('start-changed', () => { if (!this._selfUpdate && this.state.ui.inspectorTab === 'scenario') this.render(); });
    state.on('entity-updated', (id) => { if (!this._selfUpdate && id === this.state.ui.selectedEntityId) this.render(); });
    state.on('objective-updated', (id) => { if (!this._selfUpdate && id === this.state.ui.selectedObjectiveId) this.render(); });
    state.on('reset', () => this.render());
    state.on('weapons-changed', () => { if (!this._selfUpdate) this.render(); });
    state.on('models-changed', () => { if (!this._selfUpdate) this.render(); });

    this.render();
  }

  render() {
    // Save open/closed state of all collapsible sections before re-render
    const openState = {};
    this.el.querySelectorAll('details.collapsible[data-section]').forEach(d => {
      openState[d.dataset.section] = d.open;
    });
    this._openState = Object.keys(openState).length ? openState : (this._openState || {});

    const tab = this.state.ui.inspectorTab;
    switch (tab) {
      case 'entity': this._renderEntity(); break;
      case 'objective': this._renderObjective(); break;
      default: this._renderScenario(); break;
    }

    // Restore open/closed state
    this.el.querySelectorAll('details.collapsible[data-section]').forEach(d => {
      if (d.dataset.section in this._openState) {
        d.open = this._openState[d.dataset.section];
      }
    });
  }

  // --- Scenario metadata + start conditions ---

  _renderScenario() {
    const s = this.state.scenario;
    const st = s.start;

    this.el.innerHTML = `
      <h3>Scenario</h3>
      <div class="inspector-form">
        <details class="collapsible" data-section="metadata" open>
          <summary>Metadata</summary>
          <div class="collapsible-body">
            ${this._field('text', 'scenario-id', 'ID', s.id)}
            ${this._field('text', 'scenario-name', 'Name', s.name)}
            <div class="field-group">
              <label for="scenario-desc">Description</label>
              <textarea id="scenario-desc" rows="3">${this._esc(s.description)}</textarea>
            </div>
            ${this._select('scenario-difficulty', 'Difficulty', DIFFICULTIES, s.difficulty)}
            ${this._select('scenario-weather', 'Weather', WEATHER, s.weather)}
            ${this._select('scenario-season', 'Season', SEASONS, s.season)}
            ${this._select('scenario-time', 'Time of Day', TIME_OF_DAY, s.timeOfDay)}
            ${this._field('text', 'scenario-theater', 'Theater', s.theater)}
          </div>
        </details>

        <details class="collapsible" data-section="sky-color" open>
          <summary>Sky Color</summary>
          <div class="collapsible-body">
            <div class="sky-color-row">
              <input type="color" id="sky-picker" value="#${_rgbHex(s.skyColor)}" class="color-picker">
              <div class="color-preview" style="background: rgba(${s.skyColor[0]},${s.skyColor[1]},${s.skyColor[2]},${s.skyColor[3]/255})"></div>
            </div>
            <div class="field-row-multi">
              ${this._fieldInline('number', 'sky-r', 'R', s.skyColor[0], 0, 255, 1)}
              ${this._fieldInline('number', 'sky-g', 'G', s.skyColor[1], 0, 255, 1)}
              ${this._fieldInline('number', 'sky-b', 'B', s.skyColor[2], 0, 255, 1)}
              ${this._fieldInline('number', 'sky-a', 'A', s.skyColor[3], 0, 255, 1)}
            </div>
          </div>
        </details>

        <details class="collapsible" data-section="start-conditions" open>
          <summary>Start Conditions</summary>
          <div class="collapsible-body">
            ${this._renderStartBaseSelect()}
            <div class="field-row-multi">
              ${this._fieldInline('number', 'start-x', 'X', st.position.x)}
              ${this._fieldInline('number', 'start-y', 'Alt', st.position.y)}
              ${this._fieldInline('number', 'start-z', 'Z', st.position.z)}
            </div>
            ${this._field('number', 'start-heading', 'Heading (°)', st.heading, 0, 360, 1)}
            ${this._field('number', 'start-speed', 'Speed (m/s)', st.speed, 0)}
            ${this._field('number', 'start-fuel', 'Fuel (kg)', st.fuel, 0)}
            <div class="field-group">
              <label><input type="checkbox" id="start-carrier" ${st.carrier ? 'checked' : ''}> Carrier start</label>
            </div>
          </div>
        </details>

        <details class="collapsible" data-section="player-loadout" open>
          <summary>Player Loadout</summary>
          <div class="collapsible-body">
            ${this._renderPlayerLoadout()}
          </div>
        </details>
      </div>
    `;

    this._bindScenario();
  }

  _renderStartBaseSelect() {
    const bases = this.state.scenario.entities.filter(e =>
      (e.type === 'carrier' || e.type === 'airbase') &&
      (e.faction === 'friendly' || e.faction === 'neutral')
    );
    if (bases.length === 0) {
      return `<div class="info-text">No friendly/neutral airbases or carriers. Add one to pick a start location.</div>`;
    }
    const options = bases.map(e =>
      `<option value="${this._esc(e.id)}">${this._esc(e.id)} (${e.type})</option>`
    ).join('');
    return `
      <div class="field-group">
        <label>Start from base</label>
        <select id="start-base-select" class="input-full">
          <option value="">— Select a base —</option>
          ${options}
        </select>
      </div>
    `;
  }

  _renderPlayerLoadout() {
    const playerWeapons = this.state.weapons.filter(w =>
      w.category === 'air-to-air' || w.category === 'agm' || w.category === 'bomb' || w.category === 'gun'
    );
    if (playerWeapons.length === 0) {
      return `<div class="info-text">No aircraft weapons defined. Add them in the Weapons page.</div>`;
    }
    const weapons = this.state.scenario.start.weapons || [];

    // count occurrences of each weapon ID
    const wpnCount = {};
    for (const wid of weapons) wpnCount[wid] = (wpnCount[wid] || 0) + 1;
    const loadedIds = [...new Set(weapons)];

    const rows = loadedIds.map(wid => {
      const w = this.state.weapons.find(wp => wp.id === wid);
      const name = w ? (w.name || w.id) : wid;
      const qty = wpnCount[wid] || 0;
      return `
        <div class="loadout-qty-row" data-wpn="${this._esc(wid)}">
          <span class="loadout-name">${this._esc(name)}</span>
          <span class="loadout-qty-group">
            <button class="btn-sm loadout-minus">−</button>
            <span class="loadout-qty-val">${qty}</span>
            <button class="btn-sm loadout-plus">+</button>
          </span>
          <button class="btn-sm loadout-remove" title="Remove from loadout">✕</button>
        </div>`;
    }).join('');

    // weapons not yet in loadout
    const unusedWeapons = playerWeapons.filter(w => !wpnCount[w.id]);

    return `
      <div class="entity-weapons-list">${rows || '<div class="info-text">No weapons in loadout</div>'}</div>
      <select id="add-loadout-weapon" class="input-full">
        <option value="">+ Add weapon to loadout…</option>
        ${unusedWeapons.map(w => `<option value="${this._esc(w.id)}">${this._esc(w.name || w.id)}</option>`).join('')}
      </select>
    `;
  }

  _bindScenario() {
    const s = this.state;
    const bind = (id, path) => {
      const el = this.el.querySelector(`#${id}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const v = el.type === 'number' ? parseFloat(el.value) || 0 : el.value;
        this._guarded(() => {
        if (path.startsWith('start.')) {
          const key = path.slice(6);
          if (key.startsWith('position.')) {
            const axis = key.slice(9);
            s.updateStart({ position: { ...s.scenario.start.position, [axis]: v } });
          } else {
            s.updateStart({ [key]: v });
          }
        } else if (path.startsWith('sky.')) {
          const idx = parseInt(path.slice(4));
          const c = [...s.scenario.skyColor];
          c[idx] = parseInt(el.value) || 0;
          s.updateScenario({ skyColor: c });
          const preview = this.el.querySelector('.color-preview');
          if (preview) preview.style.background = `rgba(${c[0]},${c[1]},${c[2]},${c[3]/255})`;
          const picker = this.el.querySelector('#sky-picker');
          if (picker) picker.value = '#' + _rgbHex(c);
        } else {
          s.updateScenario({ [path]: v });
        }
        });
      });
    };

    bind('scenario-id', 'id');
    bind('scenario-name', 'name');
    bind('scenario-desc', 'description');
    bind('scenario-difficulty', 'difficulty');
    bind('scenario-weather', 'weather');
    bind('scenario-season', 'season');
    bind('scenario-time', 'timeOfDay');
    bind('scenario-theater', 'theater');
    bind('sky-r', 'sky.0');
    bind('sky-g', 'sky.1');
    bind('sky-b', 'sky.2');
    bind('sky-a', 'sky.3');

    // Color picker → RGBA sync
    const skyPicker = this.el.querySelector('#sky-picker');
    if (skyPicker) {
      skyPicker.addEventListener('input', () => {
        const hex = skyPicker.value;
        const r = parseInt(hex.slice(1, 3), 16);
        const g = parseInt(hex.slice(3, 5), 16);
        const b = parseInt(hex.slice(5, 7), 16);
        const a = s.scenario.skyColor[3];
        this._guarded(() => s.updateScenario({ skyColor: [r, g, b, a] }));
        // sync number inputs
        const rEl = this.el.querySelector('#sky-r');
        const gEl = this.el.querySelector('#sky-g');
        const bEl = this.el.querySelector('#sky-b');
        if (rEl) rEl.value = r;
        if (gEl) gEl.value = g;
        if (bEl) bEl.value = b;
        const preview = this.el.querySelector('.color-preview');
        if (preview) preview.style.background = `rgba(${r},${g},${b},${a/255})`;
      });
    }
    bind('start-x', 'start.position.x');
    bind('start-y', 'start.position.y');
    bind('start-z', 'start.position.z');
    bind('start-heading', 'start.heading');
    bind('start-speed', 'start.speed');
    bind('start-fuel', 'start.fuel');

    const carrierEl = this.el.querySelector('#start-carrier');
    if (carrierEl) {
      carrierEl.addEventListener('change', () => {
        this._guarded(() => this.state.updateStart({ carrier: carrierEl.checked }));
      });
    }

    // Start from base selector
    const baseSelect = this.el.querySelector('#start-base-select');
    if (baseSelect) {
      baseSelect.addEventListener('change', () => {
        const entity = s.getEntity(baseSelect.value);
        if (!entity) return;
        const isCarrier = entity.type === 'carrier';
        const startY = isCarrier ? 12 : 0;
        this._guarded(() => {
          s.updateStart({
            position: { x: entity.position.x, y: startY, z: entity.position.z },
            carrier: isCarrier,
          });
        });
        this.render();
      });
    }

    // Player loadout quantity buttons
    this.el.querySelectorAll('.loadout-qty-row').forEach(row => {
      row.addEventListener('click', (e) => {
        const btn = e.target.closest('button');
        if (!btn) return;
        const wid = row.dataset.wpn;
        const weapons = [...(s.scenario.start.weapons || [])];

        this._guarded(() => {
          if (btn.classList.contains('loadout-plus')) {
            weapons.push(wid);
            s.updateStart({ weapons });
          } else if (btn.classList.contains('loadout-minus')) {
            const idx = weapons.lastIndexOf(wid);
            if (idx !== -1) weapons.splice(idx, 1);
            s.updateStart({ weapons });
          } else if (btn.classList.contains('loadout-remove')) {
            s.updateStart({ weapons: weapons.filter(w => w !== wid) });
          }
        });
        this.render();
      });
    });

    // Add weapon to loadout
    this.el.querySelector('#add-loadout-weapon')?.addEventListener('change', (e) => {
      const wid = e.target.value;
      if (!wid) return;
      const weapons = [...(s.scenario.start.weapons || [])];
      weapons.push(wid);
      this._guarded(() => s.updateStart({ weapons }));
      e.target.value = '';
      this.render();
    });
  }

  // --- Entity properties ---

  _renderEntity() {
    const entity = this.state.getSelectedEntity();
    if (!entity) {
      this.state.ui.inspectorTab = 'scenario';
      return this._renderScenario();
    }

    const typeDef = ENTITY_TYPES[entity.type] || {};
    const subtypes = typeDef.subtypes || [];
    const hideStats = typeDef.hideStats === true;

    this.el.innerHTML = `
      <div class="inspector-header">
        <h3>${typeDef.label || entity.type}</h3>
        <div class="inspector-header-btns">
          <button id="btn-duplicate-entity" class="btn btn-sm" title="Duplicate entity">⧉ Clone</button>
          <button id="btn-delete-entity" class="btn btn-sm btn-danger" title="Delete entity">🗑 Delete</button>
          <button id="btn-deselect" class="btn btn-sm" title="Back to scenario">← Back</button>
        </div>
      </div>
      <div class="inspector-form">
        ${this._field('text', 'ent-id', 'ID', entity.id)}
        ${subtypes.length > 0
          ? this._selectWithCustom('ent-subtype', 'Subtype', subtypes, entity.subtype)
          : this._field('text', 'ent-subtype', 'Subtype', entity.subtype)}
        ${entity.type === 'aircraft' && Object.keys(AIRCRAFT_PRESETS).length > 0 ? `
          <button class="btn btn-sm btn-full" id="btn-apply-preset" title="Apply preset params for this subtype">Apply Preset</button>
        ` : ''}
        ${this._select('ent-faction', 'Faction', FACTIONS, entity.faction)}
        ${this._select('ent-state', 'State', ENTITY_STATES, entity.state)}

        <details class="collapsible" data-section="ent-position" open>
          <summary>Position</summary>
          <div class="collapsible-body">
            <div class="field-row-multi">
              ${this._fieldInline('number', 'ent-pos-x', 'X', entity.position.x)}
              ${this._fieldInline('number', 'ent-pos-y', 'Alt', entity.position.y)}
              ${this._fieldInline('number', 'ent-pos-z', 'Z', entity.position.z)}
            </div>
            ${this._field('number', 'ent-heading', 'Heading (°)', entity.heading, 0, 360, 1)}
            <button class="btn btn-sm" id="btn-copy-pos" title="Copy position to clipboard">📋 Copy Position</button>
          </div>
        </details>

        ${!hideStats ? `
          <details class="collapsible" data-section="ent-stats" open>
            <summary>Stats</summary>
            <div class="collapsible-body">
              ${this._field('number', 'ent-health', 'Health', entity.health, 0)}
              ${this._field('number', 'ent-max-health', 'Max Health', entity.maxHealth, 0)}
              ${this._field('number', 'ent-scale', 'Scale', entity.scale, 0.1, undefined, 0.1)}
              ${this._modelSelect(entity.modelId)}
            </div>
          </details>
        ` : ''}

        ${typeDef.params && typeDef.params.length > 0 ? `
          <details class="collapsible" data-section="ent-params">
            <summary>Parameters</summary>
            <div class="collapsible-body">
              ${typeDef.params.map(p => p.type === 'text'
                ? this._field('text', `ent-param-${p.key}`, p.label, entity.params[p.key] ?? p.default)
                : this._field('number', `ent-param-${p.key}`, p.label,
                  entity.params[p.key] ?? p.default, p.min, p.max, p.step)
              ).join('')}
            </div>
          </details>
        ` : ''}

        ${typeDef.properties && typeDef.properties.length > 0 ? `
          <details class="collapsible" data-section="ent-properties" open>
            <summary>Properties</summary>
            <div class="collapsible-body">
              ${typeDef.properties.map(p =>
                this._field('text', `ent-prop-${p.key}`, p.label, (entity.properties || {})[p.key] ?? p.default)
              ).join('')}
            </div>
          </details>
        ` : ''}

        ${ARMED_ENTITY_TYPES.includes(entity.type) ? `
          <details class="collapsible" data-section="ent-weapons" open>
            <summary>Weapons</summary>
            <div class="collapsible-body">${this._renderEntityWeapons(entity)}</div>
          </details>
        ` : ''}

        ${entity.type === 'aircraft' ? `
          <details class="collapsible" data-section="ent-waypoints" open>
            <summary>Waypoints</summary>
            <div class="collapsible-body">${this._renderEntityWaypoints(entity)}</div>
          </details>
        ` : ''}
      </div>
    `;

    this._bindEntity(entity, typeDef);
  }

  _renderEntityWeapons(entity) {
    const available = this.state.getWeaponsForEntityType(entity.type);
    const assigned = entity.weapons || [];

    if (available.length === 0) {
      return `<div class="info-text">No ${entity.type} weapons defined. Add them in the Weapons page.</div>`;
    }

    // Count occurrences of each weapon
    const counts = {};
    for (const wid of assigned) counts[wid] = (counts[wid] || 0) + 1;

    // Build assigned list with qty controls
    const assignedRows = Object.entries(counts).map(([wid, qty]) => {
      const w = this.state.getWeapon(wid);
      const label = w ? (w.name || w.id) : wid;
      const cat = w ? w.category : '?';
      return `
        <div class="weapon-row" data-weapon-id="${this._esc(wid)}">
          <span class="weapon-label">${this._esc(label)}</span>
          <span class="item-meta">${this._esc(cat)}</span>
          <span class="weapon-qty">
            <button class="btn-sm btn-wpn-minus" data-weapon-id="${this._esc(wid)}" title="Remove one">−</button>
            <span class="qty-value">${qty}</span>
            <button class="btn-sm btn-wpn-plus" data-weapon-id="${this._esc(wid)}" title="Add one">+</button>
          </span>
          <button class="btn-sm btn-wpn-remove-all" data-weapon-id="${this._esc(wid)}" title="Remove all">✕</button>
        </div>`;
    }).join('');

    return `
      <div class="entity-weapons-list">
        ${assignedRows || '<div class="info-text">No weapons assigned</div>'}
      </div>
      <div class="weapon-add-row">
        <select id="add-weapon-select">
          <option value="">+ Add weapon…</option>
          ${available.map(w => `<option value="${this._esc(w.id)}">${this._esc(w.name || w.id)} (${w.category})</option>`).join('')}
        </select>
      </div>
    `;
  }

  _renderEntityWaypoints(entity) {
    const wps = entity.waypoints || [];
    const rows = wps.map((wp, i) => `
      <div class="waypoint-row" data-wp-idx="${i}">
        <span class="wp-index">${i + 1}</span>
        <input class="wp-name-input" data-wp-idx="${i}" type="text" value="${wp.name ? this._esc(wp.name) : ''}" placeholder="unnamed" />
        <span class="wp-coords">X:${Math.round(wp.position.x)} Z:${Math.round(wp.position.z)} Alt:${Math.round(wp.position.y)}</span>
        <span class="wp-btns">
          <button class="btn-sm btn-wp-copy" data-wp-idx="${i}" title="Copy coordinates">📋</button>
          ${i > 0 ? `<button class="btn-sm btn-wp-up" data-wp-idx="${i}" title="Move up">↑</button>` : ''}
          ${i < wps.length - 1 ? `<button class="btn-sm btn-wp-down" data-wp-idx="${i}" title="Move down">↓</button>` : ''}
          <button class="btn-sm btn-wp-remove" data-wp-idx="${i}" title="Remove">✕</button>
        </span>
      </div>
    `).join('');

    return `
      <div class="entity-waypoints-list">
        ${rows || '<div class="info-text">No waypoints defined</div>'}
      </div>
      <div class="wp-add-bar">
        <button class="btn btn-sm" id="btn-add-wp">+ Add Waypoint</button>
        <button class="btn btn-sm" id="btn-place-wp-map" title="Click map to place waypoints">📍 Place on Map</button>
      </div>
      <div class="wp-edit-fields" id="wp-edit-container" style="display:none">
        ${this._field('text', 'wp-name', 'Name', '')}
        <div class="field-row-multi">
          ${this._fieldInline('number', 'wp-x', 'X', 0)}
          ${this._fieldInline('number', 'wp-y', 'Alt', 0)}
          ${this._fieldInline('number', 'wp-z', 'Z', 0)}
        </div>
        <div class="wp-form-btns">
          <button class="btn btn-primary btn-sm" id="btn-confirm-wp">Add</button>
          <button class="btn btn-sm" id="btn-paste-wp" title="Paste copied coordinates">📋 Paste</button>
          <button class="btn btn-sm" id="btn-cancel-wp">Cancel</button>
        </div>
      </div>
    `;
  }

  _bindEntity(entity, typeDef) {
    const state = this.state;
    let eid = entity.id;

    this.el.querySelector('#btn-deselect').addEventListener('click', () => {
      state.selectEntity(null);
    });

    const bindSimple = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const v = el.type === 'number' ? parseFloat(el.value) || 0 : el.value;
        const current = state.getEntity(eid);
        if (!current) return;

        this._guarded(() => {
        if (key === 'id') {
          current.id = el.value;
          if (state.ui.selectedEntityId === eid) state.ui.selectedEntityId = el.value;
          eid = el.value;
          state.emit('entities-changed');
          state.emit('canvas-redraw');
        } else {
          state.updateEntity(eid, { [key]: v });
        }
        });
      });
    };

    const bindPos = (elId, axis) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const current = state.getEntity(eid);
        if (!current) return;
        this._guarded(() => state.updateEntity(eid, { position: { [axis]: parseFloat(el.value) || 0 } }));
      });
    };

    const bindParam = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const current = state.getEntity(eid);
        if (!current) return;
        this._guarded(() => state.updateEntity(eid, { params: { [key]: parseFloat(el.value) || 0 } }));
      });
    };

    bindSimple('ent-id', 'id');
    bindSimple('ent-subtype', 'subtype');
    bindSimple('ent-faction', 'faction');
    bindSimple('ent-state', 'state');
    bindPos('ent-pos-x', 'x');
    bindPos('ent-pos-y', 'y');
    bindPos('ent-pos-z', 'z');
    bindSimple('ent-heading', 'heading');
    bindSimple('ent-health', 'health');
    bindSimple('ent-max-health', 'maxHealth');
    bindSimple('ent-scale', 'scale');

    // Model select uses 'change' event
    const modelEl = this.el.querySelector('#ent-model');
    if (modelEl) {
      modelEl.addEventListener('change', () => {
        const current = state.getEntity(eid);
        if (!current) return;
        this._guarded(() => state.updateEntity(eid, { modelId: modelEl.value }));
      });
    }

    if (typeDef.params) {
      for (const p of typeDef.params) {
        bindParam(`ent-param-${p.key}`, p.key);
      }
    }

    // Property field bindings (text values stored in entity.properties)
    if (typeDef.properties) {
      for (const p of typeDef.properties) {
        const el = this.el.querySelector(`#ent-prop-${p.key}`);
        if (!el) continue;
        el.addEventListener('input', () => {
          const current = state.getEntity(eid);
          if (!current) return;
          this._guarded(() => state.updateEntity(eid, { properties: { [p.key]: el.value } }));
        });
      }
    }

    // Weapon list controls
    this.el.querySelector('.entity-weapons-list')?.addEventListener('click', (e) => {
      const btn = e.target.closest('button');
      if (!btn) return;
      const wid = btn.dataset.weaponId;
      const current = state.getEntity(eid);
      if (!current || !wid) return;
      if (!current.weapons) current.weapons = [];

      this._guarded(() => {
        if (btn.classList.contains('btn-wpn-plus')) {
          current.weapons.push(wid);
        } else if (btn.classList.contains('btn-wpn-minus')) {
          const idx = current.weapons.lastIndexOf(wid);
          if (idx !== -1) current.weapons.splice(idx, 1);
        } else if (btn.classList.contains('btn-wpn-remove-all')) {
          current.weapons = current.weapons.filter(id => id !== wid);
        }
        state.emit('entity-updated', eid);
      });
      this.render();
    });

    // Add weapon dropdown
    this.el.querySelector('#add-weapon-select')?.addEventListener('change', (e) => {
      const wid = e.target.value;
      if (!wid) return;
      const current = state.getEntity(eid);
      if (!current) return;
      if (!current.weapons) current.weapons = [];
      this._guarded(() => {
        current.weapons.push(wid);
        state.emit('entity-updated', eid);
      });
      e.target.value = '';
      this.render();
    });

    // Delete entity
    this.el.querySelector('#btn-delete-entity')?.addEventListener('click', () => {
      if (confirm(`Delete entity "${eid}"?`)) {
        state.removeEntity(eid);
      }
    });

    // Duplicate entity
    this.el.querySelector('#btn-duplicate-entity')?.addEventListener('click', () => {
      const current = state.getEntity(eid);
      if (!current) return;
      const clone = state.addEntity(current.type, current.position.x + 500, current.position.z + 500, {
        subtype: current.subtype,
        faction: current.faction,
        state: current.state,
        heading: current.heading,
        health: current.health,
        maxHealth: current.maxHealth,
        scale: current.scale,
        modelId: current.modelId,
        params: { ...current.params },
        weapons: [...(current.weapons || [])],
        ...(current.waypoints ? { waypoints: current.waypoints.map(wp => ({ ...wp })) } : {}),
      });
      if (clone) state.selectEntity(clone.id);
    });

    // Copy position
    this.el.querySelector('#btn-copy-pos')?.addEventListener('click', () => {
      const current = state.getEntity(eid);
      if (!current) return;
      this._copiedPosition = { x: current.position.x, y: current.position.y, z: current.position.z };
    });

    // Apply aircraft preset
    this.el.querySelector('#btn-apply-preset')?.addEventListener('click', () => {
      const current = state.getEntity(eid);
      if (!current) return;
      const preset = AIRCRAFT_PRESETS[current.subtype];
      if (!preset) return;
      this._guarded(() => {
        for (const [key, value] of Object.entries(preset)) {
          state.updateEntity(eid, { params: { [key]: value } });
        }
      });
      this.render();
    });

    // Waypoint controls (aircraft only)
    if (entity.type === 'aircraft') {
      // Waypoint name editing
      this.el.querySelectorAll('.wp-name-input').forEach(input => {
        input.addEventListener('input', () => {
          const idx = parseInt(input.dataset.wpIdx);
          const current = state.getEntity(eid);
          if (!current || !current.waypoints || !current.waypoints[idx]) return;
          this._guarded(() => {
            current.waypoints[idx].name = input.value;
            state.emit('entity-updated', eid);
          });
        });
      });

      this.el.querySelector('.entity-waypoints-list')?.addEventListener('click', (e) => {
        const btn = e.target.closest('button');
        if (!btn) return;
        const idx = parseInt(btn.dataset.wpIdx);
        const current = state.getEntity(eid);
        if (!current || !current.waypoints) return;

        if (btn.classList.contains('btn-wp-copy')) {
          const wp = current.waypoints[idx];
          if (wp) this._copiedPosition = { x: wp.position.x, y: wp.position.y, z: wp.position.z };
          return;
        }
        this._guarded(() => {
          if (btn.classList.contains('btn-wp-up') && idx > 0) {
            [current.waypoints[idx - 1], current.waypoints[idx]] = [current.waypoints[idx], current.waypoints[idx - 1]];
          } else if (btn.classList.contains('btn-wp-down') && idx < current.waypoints.length - 1) {
            [current.waypoints[idx], current.waypoints[idx + 1]] = [current.waypoints[idx + 1], current.waypoints[idx]];
          } else if (btn.classList.contains('btn-wp-remove')) {
            current.waypoints.splice(idx, 1);
          }
          state.emit('entity-updated', eid);
        });
        this.render();
      });

      this.el.querySelector('#btn-add-wp')?.addEventListener('click', () => {
        const form = this.el.querySelector('#wp-edit-container');
        if (form) form.style.display = 'block';
      });

      this.el.querySelector('#btn-place-wp-map')?.addEventListener('click', () => {
        if (state.ui.tool === 'place-waypoint') {
          state.setTool('select');
        } else {
          state.setTool('place-waypoint');
        }
      });

      this.el.querySelector('#btn-cancel-wp')?.addEventListener('click', () => {
        const form = this.el.querySelector('#wp-edit-container');
        if (form) form.style.display = 'none';
      });

      this.el.querySelector('#btn-paste-wp')?.addEventListener('click', () => {
        if (!this._copiedPosition) return;
        const xEl = this.el.querySelector('#wp-x');
        const yEl = this.el.querySelector('#wp-y');
        const zEl = this.el.querySelector('#wp-z');
        if (xEl) xEl.value = this._copiedPosition.x;
        if (yEl) yEl.value = this._copiedPosition.y;
        if (zEl) zEl.value = this._copiedPosition.z;
      });

      this.el.querySelector('#btn-confirm-wp')?.addEventListener('click', () => {
        const current = state.getEntity(eid);
        if (!current) return;
        if (!current.waypoints) current.waypoints = [];
        const name = this.el.querySelector('#wp-name')?.value || '';
        const x = parseFloat(this.el.querySelector('#wp-x')?.value) || 0;
        const y = parseFloat(this.el.querySelector('#wp-y')?.value) || 0;
        const z = parseFloat(this.el.querySelector('#wp-z')?.value) || 0;
        this._guarded(() => {
          current.waypoints.push({ name, position: { x, y, z } });
          state.emit('entity-updated', eid);
        });
        this.render();
      });
    }
  }

  // --- Objective properties ---

  _renderObjective() {
    const obj = this.state.getSelectedObjective();
    if (!obj) {
      this.state.ui.inspectorTab = 'scenario';
      return this._renderScenario();
    }

    const entityIds = this.state.scenario.entities.map(e => e.id);

    this.el.innerHTML = `
      <div class="inspector-header">
        <h3>Objective</h3>
        <button id="btn-deselect-obj" class="btn btn-sm" title="Back to scenario">← Back</button>
      </div>
      <div class="inspector-form">
        ${this._field('text', 'obj-id', 'ID', obj.id)}
        ${this._select('obj-type', 'Type', OBJECTIVE_TYPES, obj.type)}
        ${this._selectWithCustom('obj-target', 'Target Entity', entityIds, obj.target)}
        ${this._field('text', 'obj-label', 'Label', obj.label)}
        <div class="field-group">
          <label for="obj-desc">Description</label>
          <textarea id="obj-desc" rows="2">${this._esc(obj.description || '')}</textarea>
        </div>
        ${this._field('number', 'obj-order', 'Order', obj.order, 1)}
        <div class="field-group">
          <label><input type="checkbox" id="obj-optional" ${obj.optional ? 'checked' : ''}> Optional</label>
        </div>
        <div class="field-group">
          <label><input type="checkbox" id="obj-hidden" ${obj.hidden ? 'checked' : ''}> Hidden</label>
        </div>
      </div>
    `;

    this._bindObjective(obj);
  }

  _bindObjective(obj) {
    const state = this.state;
    const oid = obj.id;

    this.el.querySelector('#btn-deselect-obj').addEventListener('click', () => {
      state.selectObjective(null);
    });

    const bind = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const v = el.type === 'number' ? parseFloat(el.value) || 0 : el.value;
        this._guarded(() => state.updateObjective(oid, { [key]: v }));
      });
    };

    bind('obj-id', 'id');
    bind('obj-type', 'type');
    bind('obj-target', 'target');
    bind('obj-label', 'label');
    bind('obj-desc', 'description');
    bind('obj-order', 'order');

    const optEl = this.el.querySelector('#obj-optional');
    if (optEl) {
      optEl.addEventListener('change', () => {
        this._guarded(() => state.updateObjective(oid, { optional: optEl.checked }));
      });
    }

    const hidEl = this.el.querySelector('#obj-hidden');
    if (hidEl) {
      hidEl.addEventListener('change', () => {
        this._guarded(() => state.updateObjective(oid, { hidden: hidEl.checked }));
      });
    }
  }

  // --- Field helpers ---

  _field(type, id, label, value, min, max, step) {
    const attrs = [];
    if (min !== undefined) attrs.push(`min="${min}"`);
    if (max !== undefined) attrs.push(`max="${max}"`);
    if (step !== undefined) attrs.push(`step="${step}"`);
    return `<div class="field-group">
      <label for="${id}">${label}</label>
      <input type="${type}" id="${id}" value="${this._esc(value ?? '')}" ${attrs.join(' ')}>
    </div>`;
  }

  _fieldInline(type, id, label, value, min, max, step) {
    const attrs = [];
    if (min !== undefined) attrs.push(`min="${min}"`);
    if (max !== undefined) attrs.push(`max="${max}"`);
    if (step !== undefined) attrs.push(`step="${step}"`);
    return `<div class="field-inline">
      <label for="${id}">${label}</label>
      <input type="${type}" id="${id}" value="${this._esc(value ?? '')}" ${attrs.join(' ')}>
    </div>`;
  }

  _select(id, label, options, value) {
    const opts = options.map(o =>
      `<option value="${o}" ${o === value ? 'selected' : ''}>${o}</option>`
    ).join('');
    return `<div class="field-group">
      <label for="${id}">${label}</label>
      <select id="${id}">${opts}</select>
    </div>`;
  }

  _modelSelect(currentId) {
    const models = this.state.getModels ? this.state.getModels() : [];
    const opts = models.map(m => {
      const display = m.label ? `${this._esc(m.label)} (${this._esc(m.file)})` : this._esc(m.file || m.id);
      return `<option value="${this._esc(m.id)}" ${m.id === currentId ? 'selected' : ''}>${display}</option>`;
    }).join('');
    return `<div class="field-group">
      <label for="ent-model">Model</label>
      <select id="ent-model">
        <option value="" ${!currentId ? 'selected' : ''}>— none —</option>
        ${opts}
      </select>
    </div>`;
  }

  _selectWithCustom(id, label, options, value) {
    const opts = options.map(o =>
      `<option value="${o}" ${o === value ? 'selected' : ''}>${o}</option>`
    ).join('');
    return `<div class="field-group">
      <label for="${id}">${label}</label>
      <input type="text" id="${id}" value="${this._esc(value ?? '')}" list="${id}-list">
      <datalist id="${id}-list">${opts}</datalist>
    </div>`;
  }

  _esc(str) {
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  /** Run a state-mutating callback without triggering inspector re-render */
  _guarded(fn) {
    this._selfUpdate = true;
    try { fn(); } finally { this._selfUpdate = false; }
  }
}
