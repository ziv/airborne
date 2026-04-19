// Scenario validation — checks consistency before export.

export function validate(state) {
  const errors = [];
  const warnings = [];
  const { scenario, map } = state;

  // Map
  if (!map.image) {
    warnings.push('No map image loaded. Export will work but positions cannot be verified visually.');
  }

  // Metadata
  if (!scenario.id.trim()) errors.push('Scenario ID is required.');
  if (!scenario.name.trim()) errors.push('Scenario name is required.');

  // Start
  const sp = scenario.start.position;
  if (sp.x === 0 && sp.z === 0) {
    warnings.push('Start position is at origin (0, 0). Did you forget to place it?');
  }

  // Entities
  if (scenario.entities.length === 0) {
    warnings.push('No entities in the scenario.');
  }

  // unique IDs
  const idCounts = {};
  for (const e of scenario.entities) {
    if (!e.id.trim()) {
      errors.push(`Entity of type "${e.type}" has an empty ID.`);
      continue;
    }
    idCounts[e.id] = (idCounts[e.id] || 0) + 1;
  }
  for (const [id, count] of Object.entries(idCounts)) {
    if (count > 1) errors.push(`Duplicate entity ID: "${id}" (appears ${count} times).`);
  }

  // entity-specific checks
  for (const e of scenario.entities) {
    if (e.health > e.maxHealth) {
      warnings.push(`Entity "${e.id}": health (${e.health}) exceeds maxHealth (${e.maxHealth}).`);
    }
    if ((e.type === 'aaa' || e.type === 'sam') && (!e.params || Object.keys(e.params).length === 0)) {
      warnings.push(`Entity "${e.id}" (${e.type}): no behavior params defined.`);
    }
  }

  // Objectives
  for (const obj of scenario.objectives) {
    if (!obj.id.trim()) errors.push('Objective has an empty ID.');
    if (!obj.label.trim()) warnings.push(`Objective "${obj.id}": label is empty.`);
    if (obj.target) {
      const targetExists = scenario.entities.some(e => e.id === obj.target);
      if (!targetExists) {
        errors.push(`Objective "${obj.id}" references target "${obj.target}" which does not exist.`);
      }
    } else if (obj.type === 'destroy') {
      errors.push(`Objective "${obj.id}" (destroy) has no target.`);
    }
  }

  // Weapons
  const weaponIds = new Set(state.weapons.map(w => w.id));
  const wpnIdCounts = {};
  for (const w of state.weapons) {
    if (!w.id.trim()) errors.push('A weapon type has an empty ID.');
    if (!w.name.trim()) warnings.push(`Weapon "${w.id}": name is empty.`);
    wpnIdCounts[w.id] = (wpnIdCounts[w.id] || 0) + 1;
  }
  for (const [id, count] of Object.entries(wpnIdCounts)) {
    if (count > 1) errors.push(`Duplicate weapon ID: "${id}" (appears ${count} times).`);
  }

  // Entity weapon references
  for (const e of scenario.entities) {
    if (e.weapons) {
      for (const wid of e.weapons) {
        if (!weaponIds.has(wid)) {
          errors.push(`Entity "${e.id}" references weapon "${wid}" which does not exist.`);
        }
      }
    }
  }

  // Player loadout references
  for (const wid of (scenario.start.availableWeapons || [])) {
    if (!weaponIds.has(wid)) {
      errors.push(`Player loadout references weapon "${wid}" which does not exist.`);
    }
  }
  for (const wid of (scenario.start.defaultWeapons || [])) {
    if (!weaponIds.has(wid)) {
      errors.push(`Player default loadout references weapon "${wid}" which does not exist.`);
    }
    if (!(scenario.start.availableWeapons || []).includes(wid)) {
      warnings.push(`Default weapon "${wid}" is not in the available weapons list.`);
    }
  }

  // Aircraft-specific checks
  for (const e of scenario.entities) {
    if (e.type === 'aircraft') {
      if (!e.params.maxSpeed || e.params.maxSpeed <= 0) {
        warnings.push(`Aircraft "${e.id}": maxSpeed is not set.`);
      }
      if ((e.params.speed || 0) > (e.params.maxSpeed || 0)) {
        warnings.push(`Aircraft "${e.id}": speed exceeds maxSpeed.`);
      }
      if (!e.params.fuelCapacity || e.params.fuelCapacity <= 0) {
        warnings.push(`Aircraft "${e.id}": fuelCapacity is not set.`);
      }
      if (e.waypoints && e.waypoints.length > 0) {
        for (let i = 0; i < e.waypoints.length; i++) {
          const wp = e.waypoints[i];
          if (wp.x === 0 && wp.z === 0) {
            warnings.push(`Aircraft "${e.id}" waypoint ${i + 1}: at origin (0,0).`);
          }
        }
      }
    }
  }

  // No friendly entities
  const hasFriendly = scenario.entities.some(e => e.faction === 'friendly');
  if (!hasFriendly && scenario.entities.length > 0) {
    warnings.push('No friendly entities in scenario.');
  }

  // No enemy entities
  const hasEnemy = scenario.entities.some(e => e.faction === 'enemy');
  if (!hasEnemy && scenario.entities.length > 0) {
    warnings.push('No enemy entities in scenario.');
  }

  // No destroy objectives
  const hasDestroyObj = scenario.objectives.some(o => o.type === 'destroy');
  if (!hasDestroyObj && scenario.objectives.length > 0) {
    warnings.push('No destroy objectives — player may have no combat goals.');
  }

  return { errors, warnings, valid: errors.length === 0 };
}
