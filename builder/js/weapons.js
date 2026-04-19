// Weapons manager page — CRUD for weapon type definitions.

import { WEAPON_CATEGORIES, MISSILE_CATEGORIES, makeDefaultWeapon } from './entity-defs.js';

export class WeaponsPage {
  constructor(el, state) {
    this.el = el;
    this.state = state;
    this._selfUpdate = false;
    this._selectedId = null;

    state.on('weapons-changed', () => { if (!this._selfUpdate) this.render(); });
    state.on('weapon-updated', () => { if (!this._selfUpdate) this.render(); });
    state.on('reset', () => { this._selectedId = null; this.render(); });
  }

  show() {
    this.el.style.display = 'flex';
    this.render();
  }

  hide() {
    this.el.style.display = 'none';
  }

  render() {
    const weapons = this.state.weapons;
    const sel = this._selectedId ? this.state.getWeapon(this._selectedId) : null;

    this.el.innerHTML = `
      <div class="weapons-sidebar">
        <div class="weapons-sidebar-header">
          <h3>Weapons <span class="badge">${weapons.length}</span></h3>
          <button id="btn-add-weapon" class="btn btn-sm btn-primary">+ Add</button>
        </div>
        <div class="weapons-filter">
          <select id="weapons-filter-cat">
            <option value="">All categories</option>
            ${WEAPON_CATEGORIES.map(c => `<option value="${c}">${c}</option>`).join('')}
          </select>
        </div>
        <div id="weapons-list" class="item-list">
          ${this._renderList(weapons)}
        </div>
      </div>
      <div class="weapons-detail">
        ${sel ? this._renderDetail(sel) : '<div class="empty-state">Select a weapon or create a new one</div>'}
      </div>
    `;

    this._bind();
  }

  _renderList(weapons) {
    if (weapons.length === 0) {
      return '<div class="empty-text">No weapons defined</div>';
    }

    return weapons.map(w => `
      <div class="item-row ${w.id === this._selectedId ? 'selected' : ''}" data-weapon-id="${_esc(w.id)}">
        <div class="item-info">
          <span class="item-name">${_esc(w.name || w.id)}</span>
          <span class="item-meta">${_esc(w.category)}</span>
        </div>
        <button class="btn-icon btn-delete-weapon" title="Delete">✕</button>
      </div>
    `).join('');
  }

  _renderDetail(w) {
    return `
      <div class="inspector-form">
        <h3>Weapon Details</h3>

        <div class="field-group">
          <label for="wpn-id">ID</label>
          <input type="text" id="wpn-id" value="${_esc(w.id)}">
        </div>
        <div class="field-group">
          <label for="wpn-name">Name</label>
          <input type="text" id="wpn-name" value="${_esc(w.name)}">
        </div>
        <div class="field-group">
          <label for="wpn-category">Category</label>
          <select id="wpn-category">
            ${WEAPON_CATEGORIES.map(c =>
              `<option value="${c}" ${c === w.category ? 'selected' : ''}>${c}</option>`
            ).join('')}
          </select>
        </div>

        <h4>Properties</h4>
        <div class="field-group">
          <label for="wpn-range">Range (m)</label>
          <input type="number" id="wpn-range" value="${w.range}" min="0">
        </div>
        <div class="field-group">
          <label for="wpn-damage">Damage</label>
          <input type="number" id="wpn-damage" value="${w.damage}" min="0">
        </div>
        <div class="field-group">
          <label for="wpn-ammo">Ammo Capacity</label>
          <input type="number" id="wpn-ammo" value="${w.ammoCapacity}" min="1" step="1">
        </div>
        <div class="field-group">
          <label for="wpn-rof">Rate of Fire (rps)</label>
          <input type="number" id="wpn-rof" value="${w.rateOfFire}" min="0" step="0.1">
        </div>
        <div class="field-group">
          <label for="wpn-speed">Projectile Speed (m/s)</label>
          <input type="number" id="wpn-speed" value="${w.speed}" min="0">
        </div>
        <div class="field-group">
          <label for="wpn-maxvel">Max Velocity (m/s)</label>
          <input type="number" id="wpn-maxvel" value="${w.maxVelocity ?? 0}" min="0">
        </div>

        ${MISSILE_CATEGORIES.includes(w.category) ? `
        <h4>Agility (steering limits, deg/s)</h4>
        <div class="field-group">
          <label for="wpn-agil-pitch">Max Pitch</label>
          <input type="number" id="wpn-agil-pitch" value="${w.agility?.maxPitch ?? 0}" min="0" step="0.5">
        </div>
        <div class="field-group">
          <label for="wpn-agil-yaw">Max Yaw</label>
          <input type="number" id="wpn-agil-yaw" value="${w.agility?.maxYaw ?? 0}" min="0" step="0.5">
        </div>
        <div class="field-group">
          <label for="wpn-agil-roll">Max Roll</label>
          <input type="number" id="wpn-agil-roll" value="${w.agility?.maxRoll ?? 0}" min="0" step="0.5">
        </div>
        ` : ''}

        <h4>Effects</h4>
        <div class="field-group">
          <label for="wpn-aoe">Area of Effect (m)</label>
          <input type="number" id="wpn-aoe" value="${w.effects.areaOfEffect}" min="0">
        </div>
        <div class="field-group">
          <label for="wpn-ap">Armor Penetration</label>
          <input type="number" id="wpn-ap" value="${w.effects.armorPenetration}" min="0" step="0.1">
        </div>

        <h4>Player Loadout</h4>
        <div class="field-group">
          <label><input type="checkbox" id="wpn-selectable" ${w.playerSelectable ? 'checked' : ''}> Available for player selection</label>
        </div>
        <div class="field-group">
          <label><input type="checkbox" id="wpn-default" ${w.playerDefault ? 'checked' : ''}> Pre-selected by default</label>
        </div>
      </div>
    `;
  }

  _bind() {
    const state = this.state;

    // Add weapon
    this.el.querySelector('#btn-add-weapon')?.addEventListener('click', () => {
      const w = state.addWeapon();
      this._selectedId = w.id;
      this.render();
    });

    // Filter
    this.el.querySelector('#weapons-filter-cat')?.addEventListener('change', (e) => {
      const cat = e.target.value;
      const listEl = this.el.querySelector('#weapons-list');
      const filtered = cat ? state.weapons.filter(w => w.category === cat) : state.weapons;
      listEl.innerHTML = this._renderList(filtered);
      this._bindList();
    });

    this._bindList();
    this._bindDetail();
  }

  _bindList() {
    // List click — select
    this.el.querySelector('#weapons-list')?.addEventListener('click', (e) => {
      const deleteBtn = e.target.closest('.btn-delete-weapon');
      const row = e.target.closest('[data-weapon-id]');
      if (!row) return;

      const id = row.dataset.weaponId;
      if (deleteBtn) {
        if (confirm(`Delete weapon "${id}"?`)) {
          this._guarded(() => this.state.removeWeapon(id));
          if (this._selectedId === id) this._selectedId = null;
          this.render();
        }
        return;
      }

      this._selectedId = id;
      this.render();
    });
  }

  _bindDetail() {
    if (!this._selectedId) return;
    const id = this._selectedId;

    const bindField = (elId, key, transform) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const v = transform ? transform(el.value) : el.value;
        this._guarded(() => {
          if (key === 'id') {
            const weapon = this.state.getWeapon(id);
            if (!weapon) return;
            // Update references in entities
            for (const e of this.state.scenario.entities) {
              if (e.weapons) {
                const idx = e.weapons.indexOf(id);
                if (idx !== -1) e.weapons[idx] = el.value;
              }
            }
            // Update references in player loadout
            const start = this.state.scenario.start;
            if (start.availableWeapons) {
              const idx = start.availableWeapons.indexOf(id);
              if (idx !== -1) start.availableWeapons[idx] = el.value;
            }
            if (start.defaultWeapons) {
              const idx = start.defaultWeapons.indexOf(id);
              if (idx !== -1) start.defaultWeapons[idx] = el.value;
            }
            weapon.id = el.value;
            this._selectedId = el.value;
            this.state.emit('weapons-changed');
          } else {
            this.state.updateWeapon(id, { [key]: v });
          }
        });
      });
    };

    const num = (v) => parseFloat(v) || 0;
    const int = (v) => parseInt(v) || 0;

    bindField('wpn-id', 'id');
    bindField('wpn-name', 'name');

    // Category change needs re-render to show/hide agility section
    const catEl = this.el.querySelector('#wpn-category');
    if (catEl) {
      catEl.addEventListener('change', () => {
        this._guarded(() => this.state.updateWeapon(id, { category: catEl.value }));
        this.render();
      });
    }
    bindField('wpn-range', 'range', num);
    bindField('wpn-damage', 'damage', num);
    bindField('wpn-ammo', 'ammoCapacity', int);
    bindField('wpn-rof', 'rateOfFire', num);
    bindField('wpn-speed', 'speed', num);
    bindField('wpn-maxvel', 'maxVelocity', num);

    // Agility (nested)
    const bindAgility = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        this._guarded(() => this.state.updateWeapon(id, { agility: { [key]: num(el.value) } }));
      });
    };
    bindAgility('wpn-agil-pitch', 'maxPitch');
    bindAgility('wpn-agil-yaw', 'maxYaw');
    bindAgility('wpn-agil-roll', 'maxRoll');

    // Effects
    const bindEffect = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        this._guarded(() => this.state.updateWeapon(id, { effects: { [key]: num(el.value) } }));
      });
    };
    bindEffect('wpn-aoe', 'areaOfEffect');
    bindEffect('wpn-ap', 'armorPenetration');

    // Checkboxes
    const bindCheck = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('change', () => {
        this._guarded(() => this.state.updateWeapon(id, { [key]: el.checked }));
      });
    };
    bindCheck('wpn-selectable', 'playerSelectable');
    bindCheck('wpn-default', 'playerDefault');
  }

  _guarded(fn) {
    this._selfUpdate = true;
    try { fn(); } finally { this._selfUpdate = false; }
  }
}

function _esc(str) {
  return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}
