// Main entry — initialize modules and wire them together.

import * as defs from './entity-defs.js';
import { ScenarioState, setDefs, saveSession, restoreSession, reloadMapFromHandle, clearSession } from './state.js';
import { MapCanvas } from './canvas.js';
import { Sidebar } from './sidebar.js';
import { Inspector } from './inspector.js';
import { WeaponsPage } from './weapons.js';
import { ObjectivesPage } from './objectives.js';
import { SettingsPage } from './settings.js';
import { validate } from './validation.js';
import { downloadJSON } from './exporter.js';
import { CommandHistory } from './history.js';

// wire entity definitions into state module
setDefs(defs);

const state = new ScenarioState();

// --- Initialize UI modules ---

const canvas = new MapCanvas(document.getElementById('map-canvas'), state);
const sidebar = new Sidebar(document.getElementById('sidebar'), state);
const inspector = new Inspector(document.getElementById('inspector'), state);
const weaponsPage = new WeaponsPage(document.getElementById('weapons-page'), state);
const objectivesPage = new ObjectivesPage(document.getElementById('objectives-page'), state);
const settingsPage = new SettingsPage(document.getElementById('settings-page'), state);

// --- Page navigation (Map ↔ Weapons) ---
const appEl = document.getElementById('app');
const btnPageMap = document.getElementById('btn-page-map');
const btnPageWeapons = document.getElementById('btn-page-weapons');
const btnPageObjectives = document.getElementById('btn-page-objectives');
const btnPageSettings = document.getElementById('btn-page-settings');

function showPage(page) {
  const btns = [btnPageMap, btnPageWeapons, btnPageObjectives, btnPageSettings];
  btns.forEach(b => b.classList.remove('tab-active'));

  weaponsPage.hide();
  objectivesPage.hide();
  settingsPage.hide();

  if (page === 'weapons') {
    appEl.style.display = 'none';
    weaponsPage.show();
    btnPageWeapons.classList.add('tab-active');
  } else if (page === 'objectives') {
    appEl.style.display = 'none';
    objectivesPage.show();
    btnPageObjectives.classList.add('tab-active');
  } else if (page === 'settings') {
    appEl.style.display = 'none';
    settingsPage.show();
    btnPageSettings.classList.add('tab-active');
  } else {
    appEl.style.display = 'flex';
    btnPageMap.classList.add('tab-active');
    window.dispatchEvent(new Event('resize'));
  }
}

btnPageMap.addEventListener('click', () => showPage('map'));
btnPageWeapons.addEventListener('click', () => showPage('weapons'));
btnPageObjectives.addEventListener('click', () => showPage('objectives'));
btnPageSettings.addEventListener('click', () => showPage('settings'));

// Grayscale toggle
const btnGrayscale = document.getElementById('btn-toggle-grayscale');
btnGrayscale.addEventListener('click', () => {
  state.ui.grayscale = !state.ui.grayscale;
  btnGrayscale.classList.toggle('tab-active', state.ui.grayscale);
  canvas.draw();
});

// Grid toggle
const btnGrid = document.getElementById('btn-grid-toggle');
btnGrid.addEventListener('click', () => {
  state.ui.showGrid = !state.ui.showGrid;
  btnGrid.classList.toggle('tab-active', state.ui.showGrid);
  canvas.draw();
});

// Zoom buttons
document.getElementById('btn-zoom-in').addEventListener('click', () => {
  state.camera.zoom = Math.min(state.camera.zoom * 1.3, 64);
  state.emit('camera-changed');
  state.emit('canvas-redraw');
});
document.getElementById('btn-zoom-out').addEventListener('click', () => {
  state.camera.zoom = Math.max(state.camera.zoom / 1.3, 0.05);
  state.emit('camera-changed');
  state.emit('canvas-redraw');
});

// --- Undo / Redo ---
const history = new CommandHistory(state);
const btnUndo = document.getElementById('btn-undo');
const btnRedo = document.getElementById('btn-redo');

state.on('history-changed', ({ canUndo, canRedo }) => {
  btnUndo.disabled = !canUndo;
  btnRedo.disabled = !canRedo;
});
btnUndo.addEventListener('click', () => history.undo());
btnRedo.addEventListener('click', () => history.redo());

// --- Restore previous session ---
restoreSession(state).then(async (restored) => {
  // Seed default weapons if none exist (fresh or pre-weapons session)
  if (state.weapons.length === 0 && defs.DEFAULT_WEAPONS) {
    for (const w of defs.DEFAULT_WEAPONS) {
      state.weapons.push({ ...w });
    }
    state.emit('weapons-changed');
  }

  if (!restored) return;

  state.emit('scenario-changed');
  state.emit('start-changed');
  state.emit('entities-changed');
  state.emit('objectives-changed');
  state.emit('weapons-changed');
  state.emit('selection-changed');

  // Try to reload the map image from the stored file handle
  if (state.map.fileHandle) {
    const ok = await reloadMapFromHandle(state);
    if (!ok) {
      _showReloadBanner();
    }
  }

  // Initialize undo history after session is fully restored
  history.init();
});

function _showReloadBanner() {
  const banner = document.createElement('div');
  banner.className = 'reload-banner';
  banner.innerHTML = `
    <span>Map image needs file access permission to reload.</span>
    <button id="btn-reload-map" class="btn btn-sm">Grant access</button>
    <button id="btn-dismiss-banner" class="btn btn-sm">✕</button>
  `;
  document.body.prepend(banner);

  banner.querySelector('#btn-reload-map').addEventListener('click', async () => {
    const ok = await reloadMapFromHandle(state);
    if (ok) {
      banner.remove();
    } else {
      showToast('Could not reload map — try loading the image again', 'error');
      banner.remove();
    }
  });
  banner.querySelector('#btn-dismiss-banner').addEventListener('click', () => banner.remove());
}

// --- Auto-save on changes (debounced) ---
let _saveTimer = null;
function scheduleSave() {
  if (!state.settings.autoSave) return;
  clearTimeout(_saveTimer);
  _saveTimer = setTimeout(() => saveSession(state), 300);
}

for (const evt of [
  'scenario-changed', 'start-changed', 'entities-changed',
  'entity-updated', 'objectives-changed', 'objective-updated',
  'weapons-changed', 'weapon-updated',
  'map-changed', 'camera-changed', 'settings-changed',
]) {
  state.on(evt, scheduleSave);
}

// --- Toolbar buttons ---

document.getElementById('btn-new').addEventListener('click', () => {
  if (state.scenario.entities.length > 0) {
    if (!confirm('Start a new scenario? Unsaved changes will be lost.')) return;
  }
  clearSession();
  state.reset();
});

document.getElementById('btn-import').addEventListener('click', () => {
  document.getElementById('input-import-file').click();
});

document.getElementById('input-import-file').addEventListener('change', (e) => {
  const file = e.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = () => {
    try {
      const json = JSON.parse(reader.result);
      state.importJSON(json);
      showToast('Scenario imported');
    } catch (err) {
      showToast('Import failed: ' + err.message, 'error');
    }
  };
  reader.readAsText(file);
  e.target.value = '';
});

document.getElementById('btn-validate').addEventListener('click', () => {
  const result = validate(state);
  showValidationResult(result);
});

document.getElementById('btn-export').addEventListener('click', () => {
  const result = validate(state);
  if (!result.valid) {
    showValidationResult(result);
    return;
  }
  downloadJSON(state);
  showToast('Scenario exported');
});

// Save to filesystem (uses File System Access API if available)
import { exportScenario } from './exporter.js';

document.getElementById('btn-save').addEventListener('click', async () => {
  const json = exportScenario(state);
  const text = JSON.stringify(json, null, 2);
  try {
    if (window.showSaveFilePicker) {
      const handle = await window.showSaveFilePicker({
        suggestedName: (state.scenario.id || 'scenario') + '.json',
        types: [{ description: 'JSON', accept: { 'application/json': ['.json'] } }],
      });
      const writable = await handle.createWritable();
      await writable.write(text);
      await writable.close();
      showToast('Scenario saved');
    } else {
      // fallback: download
      downloadJSON(state);
    }
  } catch (err) {
    if (err.name !== 'AbortError') showToast('Save failed: ' + err.message, 'error');
  }
});

// Load from filesystem
document.getElementById('btn-load').addEventListener('click', () => {
  document.getElementById('input-load-file').click();
});
document.getElementById('input-load-file').addEventListener('change', (e) => {
  const file = e.target.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = () => {
    try {
      const json = JSON.parse(reader.result);
      state.importJSON(json);
      showToast('Scenario loaded');
    } catch (err) {
      showToast('Load failed: ' + err.message, 'error');
    }
  };
  reader.readAsText(file);
  e.target.value = '';
});

// --- Validation dialog ---

function showValidationResult(result) {
  const dialog = document.getElementById('validation-dialog');
  const content = dialog.querySelector('.dialog-content');

  let html = '';
  if (result.valid && result.warnings.length === 0) {
    html = '<p class="success-text">✓ Scenario is valid. No issues found.</p>';
  } else {
    if (result.errors.length > 0) {
      html += '<h4 class="error-text">Errors</h4><ul class="error-list">';
      html += result.errors.map(e => `<li>${e}</li>`).join('');
      html += '</ul>';
    }
    if (result.warnings.length > 0) {
      html += '<h4 class="warning-text">Warnings</h4><ul class="warning-list">';
      html += result.warnings.map(w => `<li>${w}</li>`).join('');
      html += '</ul>';
    }
  }

  content.innerHTML = html;
  dialog.classList.add('visible');

  dialog.querySelector('.dialog-close').onclick = () => dialog.classList.remove('visible');
  dialog.addEventListener('click', (e) => {
    if (e.target === dialog) dialog.classList.remove('visible');
  });
}

// --- Toast notifications ---

function showToast(message, type = 'info') {
  const toast = document.createElement('div');
  toast.className = `toast toast-${type}`;
  toast.textContent = message;
  document.body.appendChild(toast);
  requestAnimationFrame(() => toast.classList.add('visible'));
  setTimeout(() => {
    toast.classList.remove('visible');
    setTimeout(() => toast.remove(), 300);
  }, 2500);
}

// --- Keyboard shortcuts ---

document.addEventListener('keydown', (e) => {
  // Undo/Redo work even in inputs
  if ((e.ctrlKey || e.metaKey) && e.key === 'z') {
    e.preventDefault();
    history.undo();
    return;
  }
  if ((e.ctrlKey || e.metaKey) && e.key === 'y') {
    e.preventDefault();
    history.redo();
    return;
  }
  // don't intercept when typing in inputs
  if (e.target.tagName === 'INPUT' || e.target.tagName === 'TEXTAREA' || e.target.tagName === 'SELECT') return;

  switch (e.key) {
    case 'v':
    case 'V':
      state.setTool('select');
      break;
    case 's':
    case 'S':
      state.setTool('start');
      break;
    case 'Escape':
      state.setTool('select');
      state.selectEntity(null);
      break;
    case 'Delete':
    case 'Backspace':
      if (state.ui.selectedEntityId) {
        state.removeEntity(state.ui.selectedEntityId);
      } else if (state.ui.selectedObjectiveId) {
        state.removeObjective(state.ui.selectedObjectiveId);
      }
      break;
  }
});
