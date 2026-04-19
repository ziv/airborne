// Map canvas — renders map image, entity markers, handles pan/zoom/click.

import { drawEntityMarker, drawStartMarker, drawRangeRings } from './entity-defs.js';

const MARKER_SIZE = 10;
const HIT_RADIUS = 14;

export class MapCanvas {
  constructor(canvasEl, state) {
    this.canvas = canvasEl;
    this.ctx = canvasEl.getContext('2d');
    this.state = state;

    this._panning = false;
    this._panStart = null;
    this._lastMouse = { x: 0, y: 0 };
    this._rafPending = false;

    // entity dragging
    this._dragEntity = null;   // entity being dragged
    this._dragStart = null;    // { mx, my } screen coords at drag start
    this._dragging = false;    // true once past threshold

    this._bindEvents();

    state.on('camera-changed', () => this.draw());
    state.on('canvas-redraw', () => this.draw());
    state.on('map-changed', () => this.draw());

    this._resizeObserver = new ResizeObserver(() => this._resize());
    this._resizeObserver.observe(canvasEl.parentElement);
    this._resize();
  }

  // --- Rendering ---

  draw() {
    const { ctx, canvas, state } = this;
    const w = canvas.width;
    const h = canvas.height;

    ctx.clearRect(0, 0, w, h);
    ctx.fillStyle = '#0d1117';
    ctx.fillRect(0, 0, w, h);

    ctx.save();

    // camera transform: translate to center, then apply pan and zoom
    const cx = w / 2 + state.camera.x * state.camera.zoom;
    const cy = h / 2 + state.camera.y * state.camera.zoom;
    ctx.translate(cx, cy);
    ctx.scale(state.camera.zoom, state.camera.zoom);

    // draw map image centered on origin
    if (state.map.image) {
      const imgW = state.map.width;
      const imgH = state.map.height;
      if (state.ui.grayscale) {
        ctx.filter = 'grayscale(1) brightness(0.85)';
      }
      ctx.drawImage(state.map.image, -imgW / 2, -imgH / 2);
      ctx.filter = 'none';

      // grid overlay
      if (state.ui.showGrid) this._drawGrid();
    } else {
      ctx.fillStyle = '#161b22';
      ctx.font = '16px system-ui';
      ctx.textAlign = 'center';
      ctx.fillText('Load a map image to begin', 0, 0);
    }

    // draw range rings for all SAM/AAA entities
    if (state.settings.showRangeRings) {
      for (const entity of state.scenario.entities) {
        if ((entity.type === 'sam' || entity.type === 'aaa') && entity.params) {
          const { px, py } = state.worldToPixel(entity.position.x, entity.position.z);
          const isSelected = entity.id === state.ui.selectedEntityId;
          drawRangeRings(ctx, px, py, entity.params, 1 / state.map.ratio, isSelected);
        }
      }
    }

    // draw waypoint paths for aircraft
    if (state.settings.showWaypointPaths) {
      for (const entity of state.scenario.entities) {
        if (entity.type === 'aircraft' && entity.waypoints && entity.waypoints.length > 0) {
          const selected = entity.id === state.ui.selectedEntityId;
          ctx.save();
          ctx.strokeStyle = selected ? '#00E5FF' : 'rgba(0,200,255,0.4)';
          ctx.lineWidth = (selected ? 2 : 1) / state.camera.zoom;
          ctx.setLineDash([6 / state.camera.zoom, 4 / state.camera.zoom]);
          ctx.beginPath();
          const startPx = state.worldToPixel(entity.position.x, entity.position.z);
          ctx.moveTo(startPx.px, startPx.py);
          for (const wp of entity.waypoints) {
            const wpPx = state.worldToPixel(wp.x, wp.z);
            ctx.lineTo(wpPx.px, wpPx.py);
          }
          ctx.stroke();
          ctx.setLineDash([]);
          const dotR = Math.max(3, 5 / state.camera.zoom);
          for (let i = 0; i < entity.waypoints.length; i++) {
            const wp = entity.waypoints[i];
            const wpPx = state.worldToPixel(wp.x, wp.z);
            ctx.beginPath();
            ctx.arc(wpPx.px, wpPx.py, dotR, 0, Math.PI * 2);
            ctx.fillStyle = selected ? '#00E5FF' : 'rgba(0,200,255,0.5)';
            ctx.fill();
            if (selected) {
              ctx.fillStyle = '#fff';
              ctx.font = `bold ${Math.max(8, 9 / state.camera.zoom)}px system-ui`;
              ctx.textAlign = 'center';
              ctx.fillText(`${i + 1}`, wpPx.px, wpPx.py - dotR - 2 / state.camera.zoom);
            }
          }
          ctx.restore();
        }
      }
    }

    // draw entities
    const mScale = state.settings.markerScale || 1;
    const markerSize = (MARKER_SIZE * mScale) / state.camera.zoom;
    for (const entity of state.scenario.entities) {
      const { px, py } = state.worldToPixel(entity.position.x, entity.position.z);
      const selected = entity.id === state.ui.selectedEntityId;
      drawEntityMarker(ctx, entity.type, entity.faction, px, py, markerSize, selected, entity.heading);

      if (state.settings.showLabels) {
        ctx.fillStyle = '#fff';
        ctx.font = `bold ${Math.max(9, 11 / state.camera.zoom)}px system-ui`;
        ctx.textAlign = 'center';
        ctx.fillText(entity.id, px, py + markerSize + 10 / state.camera.zoom);
      }
    }

    // draw start position
    const sp = state.scenario.start.position;
    if (sp.x !== 0 || sp.z !== 0) {
      const { px, py } = state.worldToPixel(sp.x, sp.z);
      drawStartMarker(ctx, px, py, markerSize * 1.2, state.scenario.start.heading);

      ctx.fillStyle = '#00E5FF';
      ctx.font = `${Math.max(9, 11 / state.camera.zoom)}px system-ui`;
      ctx.textAlign = 'center';
      ctx.fillText('START', px, py + markerSize + 12 / state.camera.zoom);
    }

    ctx.restore();

    // HUD overlay (zoom level, cursor coords)
    this._drawHud();

    // Minimap
    this._drawMinimap();
  }

  _drawGrid() {
    const { ctx, state } = this;
    if (!state.map.image || state.camera.zoom < 0.3) return;

    const ratio = state.map.ratio;
    let gridMeters = state.settings.gridSizeMeters || 1000;
    const pixelSpacing = gridMeters / ratio * state.camera.zoom;
    if (pixelSpacing < 30) gridMeters *= 5;
    if (pixelSpacing < 6) gridMeters *= 10;
    const gridPixels = gridMeters / ratio;

    ctx.strokeStyle = 'rgba(255, 255, 255, 0.07)';
    ctx.lineWidth = 0.5 / state.camera.zoom;
    ctx.beginPath();

    const halfW = state.map.width / 2;
    const halfH = state.map.height / 2;

    for (let x = -halfW; x <= halfW; x += gridPixels) {
      ctx.moveTo(x, -halfH);
      ctx.lineTo(x, halfH);
    }
    for (let y = -halfH; y <= halfH; y += gridPixels) {
      ctx.moveTo(-halfW, y);
      ctx.lineTo(halfW, y);
    }
    ctx.stroke();
  }

  _drawHud() {
    const { ctx, canvas, state } = this;
    const h = canvas.height;
    const w = canvas.width;

    // bottom bar
    ctx.fillStyle = 'rgba(13, 17, 23, 0.85)';
    ctx.fillRect(0, h - 28, w, 28);

    ctx.fillStyle = '#8b949e';
    ctx.font = '12px monospace';
    ctx.textAlign = 'left';

    // mouse world coordinates
    const rect = { width: w, height: h };
    const { px, py } = state.screenToPixel(this._lastMouse.x, this._lastMouse.y, rect);
    const world = state.pixelToWorld(px, py);
    ctx.fillText(`X: ${Math.round(world.x)}  Z: ${Math.round(world.z)}`, 10, h - 9);

    ctx.textAlign = 'center';
    ctx.fillText(`Zoom: ${(state.camera.zoom * 100).toFixed(0)}%`, w / 2, h - 9);

    ctx.textAlign = 'right';
    ctx.fillText(`Entities: ${state.scenario.entities.length}`, w - 10, h - 9);

    // Tool mode indicator
    if (state.ui.tool === 'place-waypoint') {
      ctx.fillStyle = '#d29922';
      ctx.textAlign = 'center';
      ctx.fillText('📍 Placing Waypoints — click map to add, Esc to stop', w / 2, h - 38);
    } else if (state.ui.tool === 'place') {
      ctx.fillStyle = '#58a6ff';
      ctx.textAlign = 'center';
      ctx.fillText('Click to place entity — Esc to cancel', w / 2, h - 38);
    }

    // Entity tooltip on hover
    this._drawTooltip(px, py);
  }

  _drawTooltip(cursorPx, cursorPy) {
    if (this._panning || this._dragging) return;
    const { ctx, state } = this;
    const hit = this._hitTest(cursorPx, cursorPy);
    if (!hit) { this.canvas.title = ''; return; }

    this.canvas.title = '';

    const lines = [
      hit.id,
      `${hit.type}${hit.subtype ? ' / ' + hit.subtype : ''}`,
      `Faction: ${hit.faction}`,
    ];
    if (hit.type !== 'waypoint') {
      lines.push(`HP: ${hit.health}/${hit.maxHealth}`);
    }
    if (hit.weapons && hit.weapons.length > 0) {
      lines.push(`Weapons: ${hit.weapons.length}`);
    }
    if (hit.waypoints && hit.waypoints.length > 0) {
      lines.push(`Waypoints: ${hit.waypoints.length}`);
    }

    const fontSize = 12;
    const padding = 6;
    const lineHeight = fontSize + 3;
    ctx.font = `${fontSize}px system-ui`;

    const maxW = Math.max(...lines.map(l => ctx.measureText(l).width));
    const boxW = maxW + padding * 2;
    const boxH = lines.length * lineHeight + padding * 2;

    // position near cursor, offset right and down
    let tx = this._lastMouse.x + 14;
    let ty = this._lastMouse.y + 14;
    // clamp to canvas
    if (tx + boxW > this.canvas.width - 4) tx = this._lastMouse.x - boxW - 4;
    if (ty + boxH > this.canvas.height - 32) ty = this._lastMouse.y - boxH - 4;

    ctx.fillStyle = 'rgba(22, 27, 34, 0.92)';
    ctx.strokeStyle = 'rgba(48, 54, 61, 0.8)';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.roundRect(tx, ty, boxW, boxH, 4);
    ctx.fill();
    ctx.stroke();

    ctx.fillStyle = '#e6edf3';
    ctx.textAlign = 'left';
    for (let i = 0; i < lines.length; i++) {
      if (i === 0) ctx.font = `bold ${fontSize}px system-ui`;
      else ctx.font = `${fontSize}px system-ui`;
      ctx.fillText(lines[i], tx + padding, ty + padding + fontSize + i * lineHeight);
    }
  }

  _drawMinimap() {
    const { ctx, canvas, state } = this;
    if (!state.map.image) return;

    const mmW = 160;
    const mmH = 120;
    const mmPad = 8;
    const mmX = canvas.width - mmW - mmPad;
    const mmY = canvas.height - mmH - mmPad - 28; // above HUD bar

    // background
    ctx.save();
    ctx.fillStyle = 'rgba(13, 17, 23, 0.8)';
    ctx.strokeStyle = 'rgba(48, 54, 61, 0.8)';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.roundRect(mmX - 2, mmY - 2, mmW + 4, mmH + 4, 4);
    ctx.fill();
    ctx.stroke();

    // clip to minimap area
    ctx.beginPath();
    ctx.rect(mmX, mmY, mmW, mmH);
    ctx.clip();

    // draw scaled map image
    const imgW = state.map.width;
    const imgH = state.map.height;
    const scale = Math.min(mmW / imgW, mmH / imgH);
    const dw = imgW * scale;
    const dh = imgH * scale;
    const dx = mmX + (mmW - dw) / 2;
    const dy = mmY + (mmH - dh) / 2;

    if (state.ui.grayscale) ctx.filter = 'grayscale(1) brightness(0.6)';
    else ctx.filter = 'brightness(0.6)';
    ctx.drawImage(state.map.image, dx, dy, dw, dh);
    ctx.filter = 'none';

    // draw entity dots
    for (const entity of state.scenario.entities) {
      const epx = entity.position.x / state.map.ratio - imgW / 2;
      const epy = entity.position.z / state.map.ratio - imgH / 2;
      const sx = dx + (epx + imgW / 2) * scale;
      const sy = dy + (epy + imgH / 2) * scale;
      ctx.fillStyle = entity.faction === 'friendly' ? '#4CAF50'
                    : entity.faction === 'enemy' ? '#FF5252' : '#FFD740';
      ctx.fillRect(sx - 1.5, sy - 1.5, 3, 3);
    }

    // draw viewport rectangle
    const vw = canvas.width / state.camera.zoom;
    const vh = canvas.height / state.camera.zoom;
    const vcx = -state.camera.x;
    const vcy = -state.camera.y;
    const vx = dx + (vcx - vw / 2 + imgW / 2) * scale;
    const vy = dy + (vcy - vh / 2 + imgH / 2) * scale;
    const vrw = vw * scale;
    const vrh = vh * scale;

    ctx.strokeStyle = 'rgba(0, 229, 255, 0.7)';
    ctx.lineWidth = 1.5;
    ctx.strokeRect(vx, vy, vrw, vrh);

    ctx.restore();
  }

  // --- Event handling ---

  _bindEvents() {
    const c = this.canvas;

    c.addEventListener('contextmenu', (e) => e.preventDefault());

    c.addEventListener('wheel', (e) => {
      e.preventDefault();
      const factor = e.deltaY < 0 ? 1.04 : 1 / 1.04;
      const rect = c.getBoundingClientRect();
      const mx = e.clientX - rect.left;
      const my = e.clientY - rect.top;
      // pivot zoom around mouse
      const pivotX = (mx - c.width / 2) / this.state.camera.zoom - this.state.camera.x;
      const pivotY = (my - c.height / 2) / this.state.camera.zoom - this.state.camera.y;
      this.state.zoomCamera(factor, pivotX, pivotY);
    }, { passive: false });

    c.addEventListener('mousedown', (e) => {
      const rect = c.getBoundingClientRect();
      const mx = e.clientX - rect.left;
      const my = e.clientY - rect.top;

      // middle mouse, right-click, or alt+click → pan
      if (e.button === 1 || e.button === 2 || (e.button === 0 && e.altKey)) {
        this._panning = true;
        this._panStart = { x: mx, y: my, camX: this.state.camera.x, camY: this.state.camera.y };
        c.style.cursor = 'grabbing';
        e.preventDefault();
        return;
      }

      if (e.button === 0) {
        // check if we hit an entity — if so, select and prepare for drag
        const rect2 = { width: c.width, height: c.height };
        const { px, py } = this.state.screenToPixel(mx, my, rect2);
        const hitEntity = this._hitTest(px, py);

        if (hitEntity && this.state.ui.tool === 'select') {
          this.state.selectEntity(hitEntity.id);
          this._dragEntity = hitEntity;
          this._dragStart = { mx, my };
          this._dragging = false;
          c.style.cursor = 'grab';
          e.preventDefault();
          return;
        }

        this._handleClick(mx, my);
      }
    });

    c.addEventListener('mousemove', (e) => {
      const rect = c.getBoundingClientRect();
      const mx = e.clientX - rect.left;
      const my = e.clientY - rect.top;
      this._lastMouse = { x: mx, y: my };

      if (this._panning && this._panStart) {
        const dx = (mx - this._panStart.x) / this.state.camera.zoom;
        const dy = (my - this._panStart.y) / this.state.camera.zoom;
        this.state.camera.x = this._panStart.camX + dx;
        this.state.camera.y = this._panStart.camY + dy;
        this.state.emit('camera-changed');
        return;
      }

      // entity dragging
      if (this._dragEntity && this._dragStart) {
        const dx = mx - this._dragStart.mx;
        const dy = my - this._dragStart.my;
        if (!this._dragging && (dx * dx + dy * dy) > 9) {
          this._dragging = true;
          c.style.cursor = 'grabbing';
        }
        if (this._dragging) {
          const rect2 = { width: c.width, height: c.height };
          const { px, py } = this.state.screenToPixel(mx, my, rect2);
          const world = this.state.pixelToWorld(px, py);
          this.state.updateEntity(this._dragEntity.id, {
            position: { x: world.x, z: world.z },
          });
          return;
        }
      }

      // schedule redraw for HUD cursor coords
      if (!this._rafPending) {
        this._rafPending = true;
        requestAnimationFrame(() => {
          this._rafPending = false;
          // Update cursor based on tool
          const tool = this.state.ui.tool;
          if (tool === 'place' || tool === 'place-waypoint' || tool === 'start') {
            c.style.cursor = 'crosshair';
          } else if (!this._dragEntity) {
            c.style.cursor = '';
          }
          this.draw();
        });
      }
    });

    c.addEventListener('mouseup', () => {
      if (this._panning) {
        this._panning = false;
        this._panStart = null;
        c.style.cursor = '';
      }
      if (this._dragEntity) {
        this._dragEntity = null;
        this._dragStart = null;
        this._dragging = false;
        c.style.cursor = '';
      }
    });

    c.addEventListener('mouseleave', () => {
      this._panning = false;
      this._panStart = null;
      this._dragEntity = null;
      this._dragStart = null;
      this._dragging = false;
      c.style.cursor = '';
    });
  }

  _hitTest(px, py) {
    const hitRadius = HIT_RADIUS / this.state.camera.zoom;
    let hit = null;
    let bestDist = Infinity;

    for (const entity of this.state.scenario.entities) {
      const ep = this.state.worldToPixel(entity.position.x, entity.position.z);
      const dx = px - ep.px;
      const dy = py - ep.py;
      const dist = Math.sqrt(dx * dx + dy * dy);
      if (dist < hitRadius && dist < bestDist) {
        hit = entity;
        bestDist = dist;
      }
    }
    return hit;
  }

  _handleClick(mx, my) {
    const { state } = this;
    const rect = { width: this.canvas.width, height: this.canvas.height };
    const { px, py } = state.screenToPixel(mx, my, rect);
    const world = state.pixelToWorld(px, py);

    if (state.ui.tool === 'place' && state.ui.placingType) {
      let wx = world.x, wz = world.z;
      if (state.settings.snapToGrid) {
        const g = state.settings.gridSizeMeters || 1000;
        wx = Math.round(wx / g) * g;
        wz = Math.round(wz / g) * g;
      }
      const entity = state.addEntity(state.ui.placingType, wx, wz);
      if (entity) {
        state.selectEntity(entity.id);
        // stay in place mode for rapid placement; Escape to exit
      }
      return;
    }

    if (state.ui.tool === 'place-waypoint') {
      const eid = state.ui.selectedEntityId;
      const entity = eid ? state.getEntity(eid) : null;
      if (entity && entity.type === 'aircraft') {
        if (!entity.waypoints) entity.waypoints = [];
        entity.waypoints.push({ x: world.x, y: entity.params.altitude || 0, z: world.z, speed: 0 });
        state.emit('entity-updated', eid);
        state.emit('canvas-redraw');
      }
      return;
    }

    if (state.ui.tool === 'start') {
      state.updateStartPosition(world.x, world.z);
      state.setTool('select');
      return;
    }

    // select mode — hit test entities
    const hit = this._hitTest(px, py);
    state.selectEntity(hit ? hit.id : null);
  }

  _resize() {
    const parent = this.canvas.parentElement;
    this.canvas.width = parent.clientWidth;
    this.canvas.height = parent.clientHeight;
    this.draw();
  }
}
