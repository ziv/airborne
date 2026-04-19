// Models resource page — manage the list of available 3D model paths.

const STORAGE_KEY = 'airborne-builder-models';

const MODEL_BASE_PATH = 'assets/models/';

const DEFAULT_MODELS = [
  { id: 'aa-radar', file: 'aa-radar.glb', label: 'AA Radar' },
  { id: 'carrier-s', file: 'carrier-s.glb', label: 'Carrier (Small)' },
  { id: 'mig-29', file: 'mig-29.glb', label: 'MiG-29' },
];

function _esc(s) { return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/"/g,'&quot;'); }

function loadModels() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (raw) {
      const models = JSON.parse(raw);
      // migrate old 'path' key to 'file'
      for (const m of models) {
        if (m.path && !m.file) { m.file = m.path; delete m.path; }
      }
      return models;
    }
  } catch { /* ignore */ }
  return null;
}

function saveModels(models) {
  localStorage.setItem(STORAGE_KEY, JSON.stringify(models));
}

export class ModelsPage {
  constructor(el, state) {
    this.el = el;
    this.state = state;
    this._selectedId = null;

    this.models = loadModels() || DEFAULT_MODELS.map(m => ({ ...m }));
    state.getModels = () => this.models;
    state.getModelPath = (id) => {
      const m = this.models.find(m => m.id === id);
      return m ? MODEL_BASE_PATH + m.file : '';
    };

    state.on('reset', () => { this._selectedId = null; this._renderIfVisible(); });
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

  _persist() {
    saveModels(this.models);
    this.state.emit('models-changed');
  }

  render() {
    const sel = this._selectedId ? this.models.find(m => m.id === this._selectedId) : null;

    this.el.innerHTML = `
      <div class="models-sidebar">
        <div class="models-sidebar-header">
          <h3>Models <span class="badge">${this.models.length}</span></h3>
          <button id="model-add" class="btn btn-sm btn-primary">+ Add</button>
        </div>
        <div id="models-list" class="item-list">
          ${this._renderList()}
        </div>
      </div>
      <div class="models-detail">
        ${sel ? this._renderDetail(sel) : '<div class="empty-state">Select a model or add a new one</div>'}
      </div>
    `;

    this._bind();
  }

  _renderList() {
    if (this.models.length === 0) {
      return '<div class="empty-text" style="padding:12px;color:var(--text-muted)">No models defined</div>';
    }
    return this.models.map(m => `
      <div class="item-row ${m.id === this._selectedId ? 'selected' : ''}" data-mid="${_esc(m.id)}">
        <span class="model-icon">🧊</span>
        <div class="item-info">
          <span class="item-name">${_esc(m.label || m.id)}</span>
          <span class="item-meta">${_esc(MODEL_BASE_PATH + m.file)}</span>
        </div>
        <button class="btn-icon btn-delete-model" data-mid="${_esc(m.id)}" title="Delete">✕</button>
      </div>
    `).join('');
  }

  _renderDetail(model) {
    return `
      <div class="inspector-form">
        <h3>🧊 ${_esc(model.label || model.id)}</h3>

        <h4>Identity</h4>
        <div class="field-group">
          <label for="model-id">ID</label>
          <input type="text" id="model-id" value="${_esc(model.id)}">
        </div>
        <div class="field-group">
          <label for="model-label">Display Name</label>
          <input type="text" id="model-label" value="${_esc(model.label || '')}">
        </div>

        <h4>File</h4>
        <div class="field-group">
          <label for="model-file">Filename</label>
          <input type="text" id="model-file" value="${_esc(model.file || '')}">
        </div>
        <div class="field-group">
          <label>Full Path</label>
          <input type="text" value="${_esc(MODEL_BASE_PATH + (model.file || ''))}" disabled>
        </div>

        <div style="margin-top: 16px">
          <button id="model-delete" class="btn btn-sm btn-danger">Delete Model</button>
        </div>
      </div>
    `;
  }

  _bind() {
    // Add
    this.el.querySelector('#model-add')?.addEventListener('click', () => {
      const id = 'model-' + Date.now().toString(36);
      this.models.push({ id, path: '', label: 'New Model' });
      this._selectedId = id;
      this._persist();
      this.render();
    });

    // Select
    this.el.querySelectorAll('.item-row[data-mid]').forEach(el => {
      el.addEventListener('click', (e) => {
        if (e.target.closest('.btn-delete-model')) return;
        this._selectedId = el.dataset.mid;
        this.render();
      });
    });

    // Delete from list
    this.el.querySelectorAll('.btn-delete-model').forEach(btn => {
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        const mid = btn.dataset.mid;
        if (confirm(`Delete model "${mid}"?`)) {
          this.models = this.models.filter(m => m.id !== mid);
          if (this._selectedId === mid) this._selectedId = null;
          this._persist();
          this.render();
        }
      });
    });

    if (!this._selectedId) return;
    const model = this.models.find(m => m.id === this._selectedId);
    if (!model) return;

    // Delete from detail
    this.el.querySelector('#model-delete')?.addEventListener('click', () => {
      if (confirm(`Delete model "${model.id}"?`)) {
        this.models = this.models.filter(m => m.id !== model.id);
        this._selectedId = null;
        this._persist();
        this.render();
      }
    });

    // Field bindings
    const bindInput = (elId, key) => {
      const el = this.el.querySelector(`#${elId}`);
      if (!el) return;
      el.addEventListener('input', () => {
        if (key === 'id') {
          const oldId = model.id;
          model.id = el.value;
          this._selectedId = el.value;
          // Update entity modelId references
          for (const e of this.state.scenario.entities) {
            if (e.modelId === oldId) e.modelId = el.value;
          }
          this.state.emit('entities-changed');
        } else {
          model[key] = el.value;
        }
        this._persist();
      });
    };

    bindInput('model-id', 'id');
    bindInput('model-label', 'label');
    bindInput('model-file', 'file');
  }
}
