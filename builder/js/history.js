// Undo/Redo command history using state snapshots.

const MAX_HISTORY = 50;

export class CommandHistory {
  constructor(state) {
    this.state = state;
    this._undoStack = [];
    this._redoStack = [];
    this._applying = false;
    this._lastSnapshot = null;
    this._debounceTimer = null;

    // Capture snapshots on relevant changes (debounced)
    const events = [
      'scenario-changed', 'entities-changed', 'entity-updated',
      'objectives-changed', 'start-changed', 'weapons-changed',
    ];
    for (const ev of events) {
      state.on(ev, () => this._onStateChanged());
    }
  }

  _snapshot() {
    return JSON.stringify({
      scenario: this.state.scenario,
      weapons: this.state.weapons,
    });
  }

  _onStateChanged() {
    if (this._applying) return;
    clearTimeout(this._debounceTimer);
    this._debounceTimer = setTimeout(() => this._pushSnapshot(), 200);
  }

  _pushSnapshot() {
    if (this._applying) return;
    const snap = this._snapshot();
    if (snap === this._lastSnapshot) return;
    if (this._lastSnapshot !== null) {
      this._undoStack.push(this._lastSnapshot);
      if (this._undoStack.length > MAX_HISTORY) this._undoStack.shift();
      this._redoStack.length = 0;
    }
    this._lastSnapshot = snap;
    this._emitButtons();
  }

  _apply(snap) {
    this._applying = true;
    try {
      const data = JSON.parse(snap);
      // Restore scenario
      Object.assign(this.state.scenario, data.scenario);
      // Restore weapons
      this.state.weapons.length = 0;
      this.state.weapons.push(...data.weapons);
      this.state.emit('scenario-changed');
      this.state.emit('entities-changed');
      this.state.emit('objectives-changed');
      this.state.emit('start-changed');
      this.state.emit('weapons-changed');
      this.state.emit('canvas-redraw');
    } finally {
      this._applying = false;
    }
  }

  init() {
    // Take the initial snapshot so we have a baseline
    this._lastSnapshot = this._snapshot();
    this._emitButtons();
  }

  undo() {
    if (this._undoStack.length === 0) return;
    this._redoStack.push(this._lastSnapshot);
    this._lastSnapshot = this._undoStack.pop();
    this._apply(this._lastSnapshot);
    this._emitButtons();
  }

  redo() {
    if (this._redoStack.length === 0) return;
    this._undoStack.push(this._lastSnapshot);
    this._lastSnapshot = this._redoStack.pop();
    this._apply(this._lastSnapshot);
    this._emitButtons();
  }

  get canUndo() { return this._undoStack.length > 0; }
  get canRedo() { return this._redoStack.length > 0; }

  _emitButtons() {
    this.state.emit('history-changed', { canUndo: this.canUndo, canRedo: this.canRedo });
  }
}
