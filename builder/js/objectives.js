// Objectives management page — dedicated CRUD for objectives.

import { OBJECTIVE_TYPES } from './entity-defs.js';

function _esc(s) { return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/"/g,'&quot;'); }

const TYPE_ICONS = { destroy: '💥', navigate: '📍', escort: '🛡️', survive: '⏱️' };

export class ObjectivesPage {
  constructor(el, state) {
    this.el = el;
    this.state = state;
    this._selectedId = null;
    this._filter = '';

    state.on('objectives-changed', () => this._renderIfVisible());
    state.on('objective-updated', () => this._renderIfVisible());
    state.on('entities-changed', () => this._renderIfVisible());
    state.on('reset', () => { this._selectedId = null; this.render(); });
  }

  show() {
    this.el.style.display = 'flex';
    this.render();
  }

  hide() {
    this.el.style.display = 'none';
  }

  _renderIfVisible() {
    if (this.el.style.display !== 'none') this.render();
  }

  render() {
    const objectives = this.state.scenario.objectives;
    const entities = this.state.scenario.entities;
    const sel = this._selectedId ? objectives.find(o => o.id === this._selectedId) : null;

    this.el.innerHTML = `
      <div class="obj-sidebar">
        <div class="obj-sidebar-header">
          <h3>Objectives <span class="badge">${objectives.length}</span></h3>
          <button id="obj-add" class="btn btn-primary btn-sm">+ New</button>
        </div>
        <div class="obj-filter">
          <select id="obj-filter-type">
            <option value="">All types</option>
            ${OBJECTIVE_TYPES.map(t => `<option value="${t}" ${t === this._filter ? 'selected' : ''}>${TYPE_ICONS[t] || ''} ${t}</option>`).join('')}
          </select>
        </div>
        <div id="obj-list" class="item-list">
          ${this._renderList(objectives)}
        </div>
      </div>
      <div class="obj-detail">
        ${sel ? this._renderDetail(sel, entities) : '<div class="empty-state">Select an objective or create a new one</div>'}
      </div>
    `;

    this._bind(objectives, entities);
  }

  _renderList(objectives) {
    let items = objectives;
    if (this._filter) items = items.filter(o => o.type === this._filter);

    if (items.length === 0) {
      return '<div class="empty-text" style="padding:12px;color:var(--text-muted)">No objectives defined</div>';
    }

    return items.map(o => `
      <div class="item-row ${o.id === this._selectedId ? 'selected' : ''}" data-oid="${_esc(o.id)}">
        <span class="obj-icon">${TYPE_ICONS[o.type] || '🎯'}</span>
        <div class="item-info">
          <span class="item-name">${_esc(o.label || o.id)}</span>
          <span class="item-meta">${o.type}${o.target ? ' → ' + o.target : ''}${o.optional ? ' · optional' : ''}</span>
        </div>
        <button class="btn-icon btn-delete-obj" data-oid="${_esc(o.id)}" title="Delete">✕</button>
      </div>
    `).join('');
  }

  _renderDetail(obj, entities) {
    const targetEntities = entities.filter(e => e.type !== 'waypoint');

    return `
      <div class="inspector-form">
        <div class="obj-detail-header">
          <h3>${TYPE_ICONS[obj.type] || '🎯'} ${_esc(obj.label || obj.id)}</h3>
          <button id="obj-delete" class="btn btn-sm btn-danger">Delete</button>
        </div>

        <h4>Identity</h4>
        <div class="field-group">
          <label for="obj-id">ID</label>
          <input type="text" id="obj-id" value="${_esc(obj.id)}">
        </div>
        <div class="field-group">
          <label for="obj-label">Label</label>
          <input type="text" id="obj-label" value="${_esc(obj.label || '')}">
        </div>
        <div class="field-group">
          <label for="obj-desc">Description</label>
          <textarea id="obj-desc" rows="3">${_esc(obj.description || '')}</textarea>
        </div>

        <h4>Configuration</h4>
        <div class="field-group">
          <label for="obj-type">Type</label>
          <select id="obj-type">
            ${OBJECTIVE_TYPES.map(t => `<option value="${t}" ${t === obj.type ? 'selected' : ''}>${TYPE_ICONS[t] || ''} ${t}</option>`).join('')}
          </select>
        </div>

        <div class="field-group">
          <label for="obj-target">Target Entity</label>
          <select id="obj-target">
            <option value="">— none —</option>
            ${targetEntities.map(e => {
              const label = `${_esc(e.id)} (${e.type}${e.subtype ? '/' + e.subtype : ''} · ${e.faction})`;
              return `<option value="${_esc(e.id)}" ${e.id === obj.target ? 'selected' : ''}>${label}</option>`;
            }).join('')}
          </select>
          ${obj.target ? `<div class="field-hint" id="obj-target-info">${this._targetInfo(obj.target, entities)}</div>` : ''}
        </div>

        <div class="field-group">
          <label for="obj-order">Priority Order</label>
          <input type="number" id="obj-order" value="${obj.order ?? 0}" min="0">
        </div>

        <h4>Flags</h4>
        <div class="obj-flags">
          <label class="checkbox-row">
            <input type="checkbox" id="obj-optional" ${obj.optional ? 'checked' : ''}>
            <span>Optional</span>
            <span class="field-hint">Not required for mission success</span>
          </label>
          <label class="checkbox-row">
            <input type="checkbox" id="obj-hidden" ${obj.hidden ? 'checked' : ''}>
            <span>Hidden</span>
            <span class="field-hint">Revealed during gameplay</span>
          </label>
        </div>
      </div>
    `;
  }

  _targetInfo(targetId, entities) {
    const e = entities.find(e => e.id === targetId);
    if (!e) return `<span style="color:var(--danger)">⚠ Entity "${_esc(targetId)}" not found on map</span>`;
    return `<span style="color:var(--text-muted)">📌 ${e.type}/${e.subtype || '—'} at X:${Math.round(e.position.x)} Z:${Math.round(e.position.z)} (${e.faction})</span>`;
  }

  _bind(objectives, entities) {
    // Add
    this.el.querySelector('#obj-add')?.addEventListener('click', () => {
      const id = 'obj-' + Date.now().toString(36);
      this.state.addObjective({
        id, label: 'New Objective', type: 'destroy',
        target: '', description: '', optional: false, hidden: false,
        order: objectives.length
      });
      this._selectedId = id;
      this.render();
    });

    // Filter
    this.el.querySelector('#obj-filter-type')?.addEventListener('change', (e) => {
      this._filter = e.target.value;
      this.render();
    });

    // Select
    this.el.querySelectorAll('.item-row[data-oid]').forEach(el => {
      el.addEventListener('click', (e) => {
        if (e.target.closest('.btn-delete-obj')) return;
        this._selectedId = el.dataset.oid;
        this.render();
      });
    });

    // Delete from list
    this.el.querySelectorAll('.btn-delete-obj').forEach(btn => {
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        const oid = btn.dataset.oid;
        if (confirm(`Delete objective "${oid}"?`)) {
          this.state.removeObjective(oid);
          if (this._selectedId === oid) this._selectedId = null;
          this.render();
        }
      });
    });

    if (!this._selectedId) return;
    const obj = objectives.find(o => o.id === this._selectedId);
    if (!obj) return;

    // Delete from detail
    this.el.querySelector('#obj-delete')?.addEventListener('click', () => {
      if (confirm(`Delete objective "${obj.id}"?`)) {
        this.state.removeObjective(obj.id);
        this._selectedId = null;
        this.render();
      }
    });

    // Field bindings
    const bindInput = (elId, key, transform) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        const val = transform ? transform(el.value) : el.value;
        if (key === 'id') {
          const oldId = obj.id;
          obj.id = val;
          this._selectedId = val;
          this.state.emit('objective-updated', oldId);
        } else {
          obj[key] = val;
          this.state.emit('objective-updated', obj.id);
        }
      });
    };

    const bindCheck = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('change', () => {
        obj[key] = el.checked;
        this.state.emit('objective-updated', obj.id);
      });
    };

    const bindSelect = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('change', () => {
        obj[key] = el.value;
        this.state.emit('objective-updated', obj.id);
      });
    };

    bindInput('obj-id', 'id');
    bindInput('obj-label', 'label');
    bindInput('obj-desc', 'description');
    bindSelect('obj-type', 'type');
    bindSelect('obj-target', 'target');
    bindInput('obj-order', 'order', v => parseInt(v) || 0);
    bindCheck('obj-optional', 'optional');
    bindCheck('obj-hidden', 'hidden');
  }
}
