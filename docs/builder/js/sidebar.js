// Left sidebar: map setup, entity palette, entity list, objectives list.

import { ENTITY_TYPES, getFactionColor } from './entity-defs.js';

export class Sidebar {
  constructor(el, state) {
    this.el = el;
    this.state = state;

    this._render();
    this._bindEvents();

    state.on('entities-changed', () => this._renderEntityList());
    state.on('objectives-changed', () => this._renderObjectiveList());
    state.on('selection-changed', () => this._highlightSelected());
    state.on('tool-changed', () => this._highlightTool());
    state.on('map-changed', () => this._updateMapInfo());
    state.on('reset', () => { this._updateMapInfo(); this._renderEntityList(); this._renderObjectiveList(); });
  }

  _render() {
    this.el.innerHTML = `
      <details class="collapsible sidebar-section" open>
        <summary>Map</summary>
        <div class="collapsible-body">
          <button id="btn-load-map" class="btn btn-full">Load Image…</button>
          <input type="file" id="input-map-file" accept="image/*" hidden>
          <div class="field-row">
            <label>Meters/pixel</label>
            <input type="number" id="input-ratio" value="${this.state.map.ratio}" min="0.01" step="1" class="input-sm">
          </div>
          <div id="map-info" class="info-text"></div>
        </div>
      </details>

      <details class="collapsible sidebar-section" open>
        <summary>Tools</summary>
        <div class="collapsible-body">
          <div class="tool-row">
            <button class="btn tool-btn active" data-tool="select" title="Select (V)">⬚ Select</button>
            <button class="btn tool-btn" data-tool="start" title="Place start position (S)">⊕ Start</button>
          </div>
        </div>
      </details>

      <details class="collapsible sidebar-section" open>
        <summary>Place Entity</summary>
        <div class="collapsible-body">
          <div id="entity-palette" class="palette"></div>
        </div>
      </details>

      <details class="collapsible sidebar-section sidebar-section-grow" open>
        <summary>Entities <span id="entity-count" class="badge">0</span></summary>
        <div class="collapsible-body collapsible-body-grow">
          <input type="text" id="entity-search" class="input-sm input-full" placeholder="Search entities…">
          <div id="entity-list" class="item-list"></div>
        </div>
      </details>

      <details class="collapsible sidebar-section" open>
        <summary>Objectives <span id="obj-count" class="badge">0</span></summary>
        <div class="collapsible-body">
          <button id="btn-add-objective" class="btn btn-full btn-sm">+ Add Objective</button>
          <div id="objective-list" class="item-list"></div>
        </div>
      </details>
    `;

    // entity palette buttons
    const palette = this.el.querySelector('#entity-palette');
    for (const [type, def] of Object.entries(ENTITY_TYPES)) {
      const btn = document.createElement('button');
      btn.className = 'btn palette-btn';
      btn.dataset.type = type;
      btn.textContent = def.label;
      btn.title = `Place ${def.label}`;
      palette.appendChild(btn);
    }

    this._updateMapInfo();
    this._renderEntityList();
    this._renderObjectiveList();
  }

  _bindEvents() {
    const state = this.state;

    // load map — prefer File System Access API to get a persistent handle
    this.el.querySelector('#btn-load-map').addEventListener('click', async () => {
      if (window.showOpenFilePicker) {
        try {
          const [handle] = await window.showOpenFilePicker({
            types: [{ description: 'Images', accept: { 'image/*': ['.png', '.jpg', '.jpeg', '.webp', '.bmp', '.gif'] } }],
            multiple: false,
          });
          const file = await handle.getFile();
          const img = new Image();
          img.onload = () => state.setMapImage(img, file.name, handle);
          img.src = URL.createObjectURL(file);
        } catch (e) {
          if (e.name !== 'AbortError') console.warn('File picker error:', e);
        }
      } else {
        this.el.querySelector('#input-map-file').click();
      }
    });

    this.el.querySelector('#input-map-file').addEventListener('change', (e) => {
      const file = e.target.files[0];
      if (!file) return;
      const img = new Image();
      img.onload = () => state.setMapImage(img, file.name);
      img.src = URL.createObjectURL(file);
    });

    // ratio
    this.el.querySelector('#input-ratio').addEventListener('input', (e) => {
      const v = parseFloat(e.target.value);
      if (v > 0) state.setMapRatio(v);
    });

    // tool buttons
    this.el.querySelectorAll('.tool-btn').forEach(btn => {
      btn.addEventListener('click', () => {
        state.setTool(btn.dataset.tool);
      });
    });

    // entity search
    this.el.querySelector('#entity-search').addEventListener('input', () => {
      this._renderEntityList();
    });

    // palette buttons
    this.el.querySelector('#entity-palette').addEventListener('click', (e) => {
      const btn = e.target.closest('[data-type]');
      if (!btn) return;
      state.setTool('place', btn.dataset.type);
    });

    // entity list clicks
    this.el.querySelector('#entity-list').addEventListener('click', (e) => {
      const item = e.target.closest('[data-entity-id]');
      if (!item) return;

      if (e.target.classList.contains('btn-delete')) {
        state.removeEntity(item.dataset.entityId);
        return;
      }
      state.setTool('select');
      state.selectEntity(item.dataset.entityId);
    });

    // objective list clicks
    this.el.querySelector('#objective-list').addEventListener('click', (e) => {
      const item = e.target.closest('[data-obj-id]');
      if (!item) return;

      if (e.target.classList.contains('btn-delete')) {
        state.removeObjective(item.dataset.objId);
        return;
      }
      state.selectObjective(item.dataset.objId);
    });

    // add objective
    this.el.querySelector('#btn-add-objective').addEventListener('click', () => {
      const obj = state.addObjective();
      state.selectObjective(obj.id);
    });
  }

  _updateMapInfo() {
    const info = this.el.querySelector('#map-info');
    if (this.state.map.image) {
      const m = this.state.map;
      const wKm = (m.width * m.ratio / 1000).toFixed(1);
      const hKm = (m.height * m.ratio / 1000).toFixed(1);
      info.textContent = `${m.fileName} — ${m.width}×${m.height}px — ${wKm}×${hKm} km`;
    } else {
      info.textContent = '';
    }
    this.el.querySelector('#input-ratio').value = this.state.map.ratio;
  }

  _renderEntityList() {
    const list = this.el.querySelector('#entity-list');
    const entities = this.state.scenario.entities;
    this.el.querySelector('#entity-count').textContent = entities.length;

    const searchInput = this.el.querySelector('#entity-search');
    const filter = (searchInput?.value || '').toLowerCase().trim();
    const filtered = filter
      ? entities.filter(e => e.id.toLowerCase().includes(filter) || e.type.toLowerCase().includes(filter) || (e.faction || '').toLowerCase().includes(filter))
      : entities;

    if (filtered.length === 0) {
      list.innerHTML = `<div class="empty-text">${entities.length === 0 ? 'No entities placed' : 'No matches'}</div>`;
      return;
    }

    list.innerHTML = filtered.map(e => {
      const def = ENTITY_TYPES[e.type];
      const color = getFactionColor(e.faction);
      const selected = e.id === this.state.ui.selectedEntityId ? ' selected' : '';
      return `<div class="list-item${selected}" data-entity-id="${e.id}">
        <span class="dot" style="background:${color}"></span>
        <span class="list-item-label">${e.id}</span>
        <span class="list-item-type">${def ? def.short : e.type}</span>
        <button class="btn-delete" title="Delete">×</button>
      </div>`;
    }).join('');
  }

  _renderObjectiveList() {
    const list = this.el.querySelector('#objective-list');
    const objectives = this.state.scenario.objectives;
    this.el.querySelector('#obj-count').textContent = objectives.length;

    if (objectives.length === 0) {
      list.innerHTML = '<div class="empty-text">No objectives</div>';
      return;
    }

    list.innerHTML = objectives.map(o => {
      const selected = o.id === this.state.ui.selectedObjectiveId ? ' selected' : '';
      return `<div class="list-item${selected}" data-obj-id="${o.id}">
        <span class="list-item-label">${o.id}</span>
        <span class="list-item-type">${o.type}</span>
        <button class="btn-delete" title="Delete">×</button>
      </div>`;
    }).join('');
  }

  _highlightSelected() {
    this._renderEntityList();
    this._renderObjectiveList();
  }

  _highlightTool() {
    const tool = this.state.ui.tool;
    const placingType = this.state.ui.placingType;

    this.el.querySelectorAll('.tool-btn').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.tool === tool && tool !== 'place');
    });

    this.el.querySelectorAll('.palette-btn').forEach(btn => {
      btn.classList.toggle('active', tool === 'place' && btn.dataset.type === placingType);
    });

    // update canvas cursor
    const canvas = document.getElementById('map-canvas');
    if (canvas) {
      canvas.style.cursor = tool === 'select' ? '' : 'crosshair';
    }
  }
}
