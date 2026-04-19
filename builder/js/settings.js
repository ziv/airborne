// Settings page — user preferences for grid, display, and defaults.

export class SettingsPage {
  constructor(el, state) {
    this.el = el;
    this.state = state;
  }

  show() {
    this.el.style.display = 'flex';
    this.render();
  }

  hide() {
    this.el.style.display = 'none';
  }

  render() {
    const s = this.state.settings;

    this.el.innerHTML = `
      <div class="settings-container">
        <h2>Settings</h2>

        <div class="settings-section">
          <h3>Grid</h3>
          <div class="settings-field">
            <label for="set-grid-size">Grid spacing (meters)</label>
            <input type="number" id="set-grid-size" value="${s.gridSizeMeters}" min="100" step="100">
          </div>
          <div class="settings-field">
            <label for="set-snap">
              <input type="checkbox" id="set-snap" ${s.snapToGrid ? 'checked' : ''}>
              Snap entities to grid
            </label>
          </div>
        </div>

        <div class="settings-section">
          <h3>Map Defaults</h3>
          <div class="settings-field">
            <label for="set-default-ratio">Default meters/pixel ratio</label>
            <input type="number" id="set-default-ratio" value="${s.defaultRatio}" min="0.1" step="0.5">
          </div>
        </div>

        <div class="settings-section">
          <h3>Display</h3>
          <div class="settings-field">
            <label for="set-marker-scale">Marker size scale</label>
            <input type="number" id="set-marker-scale" value="${s.markerScale}" min="0.5" max="3" step="0.1">
          </div>
          <div class="settings-field">
            <label for="set-labels">
              <input type="checkbox" id="set-labels" ${s.showLabels ? 'checked' : ''}>
              Show entity labels
            </label>
          </div>
          <div class="settings-field">
            <label for="set-ranges">
              <input type="checkbox" id="set-ranges" ${s.showRangeRings ? 'checked' : ''}>
              Show SAM/AAA range rings
            </label>
          </div>
          <div class="settings-field">
            <label for="set-wp-paths">
              <input type="checkbox" id="set-wp-paths" ${s.showWaypointPaths ? 'checked' : ''}>
              Show waypoint paths
            </label>
          </div>
        </div>

        <div class="settings-section">
          <h3>Session</h3>
          <div class="settings-field">
            <label for="set-autosave">
              <input type="checkbox" id="set-autosave" ${s.autoSave ? 'checked' : ''}>
              Auto-save to browser storage
            </label>
          </div>
          <div class="settings-field">
            <button class="btn btn-danger btn-sm" id="set-clear-session">Clear saved session</button>
          </div>
        </div>
      </div>
    `;

    this._bind();
  }

  _bind() {
    const s = this.state.settings;

    const bindNum = (id, key) => {
      this.el.querySelector(`#${id}`)?.addEventListener('input', (e) => {
        const v = parseFloat(e.target.value);
        if (!isNaN(v) && v > 0) {
          s[key] = v;
          this.state.emit('settings-changed');
          this.state.emit('canvas-redraw');
        }
      });
    };

    const bindCheck = (id, key) => {
      this.el.querySelector(`#${id}`)?.addEventListener('change', (e) => {
        s[key] = e.target.checked;
        this.state.emit('settings-changed');
        this.state.emit('canvas-redraw');
      });
    };

    bindNum('set-grid-size', 'gridSizeMeters');
    bindNum('set-default-ratio', 'defaultRatio');
    bindNum('set-marker-scale', 'markerScale');
    bindCheck('set-snap', 'snapToGrid');
    bindCheck('set-labels', 'showLabels');
    bindCheck('set-ranges', 'showRangeRings');
    bindCheck('set-wp-paths', 'showWaypointPaths');
    bindCheck('set-autosave', 'autoSave');

    this.el.querySelector('#set-clear-session')?.addEventListener('click', () => {
      if (confirm('Clear saved session from browser storage? This cannot be undone.')) {
        import('./state.js').then(({ clearSession }) => {
          clearSession();
          this.state.reset();
        });
      }
    });
  }
}
