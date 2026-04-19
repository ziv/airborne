// Export scenario state to game-compatible JSON.

export function exportScenario(state) {
  const s = state.scenario;

  const entities = s.entities.map(e => {
    const out = {
      id: e.id,
      type: e.type,
      subtype: e.subtype,
      faction: e.faction,
      state: e.state,
      position: { x: e.position.x, y: e.position.y, z: e.position.z },
      heading: e.heading,
      health: e.health,
      maxHealth: e.maxHealth,
      modelId: e.modelId,
      scale: e.scale,
    };

    // include params only if non-empty
    if (e.params && Object.keys(e.params).length > 0) {
      out.params = { ...e.params };
    }

    // include weapons only if assigned
    if (e.weapons && e.weapons.length > 0) {
      out.weapons = [...e.weapons];
    }

    // include waypoints for aircraft
    if (e.waypoints && e.waypoints.length > 0) {
      out.waypoints = e.waypoints.map(wp => ({ ...wp }));
    }

    return out;
  });

  const data = {
    id: s.id,
    name: s.name,
    description: s.description,
    difficulty: s.difficulty,
    weather: s.weather,
    season: s.season,
    timeOfDay: s.timeOfDay,
    theater: s.theater,
    start: {
      position: { ...s.start.position },
      heading: s.start.heading,
      speed: s.start.speed,
      altitude: s.start.altitude,
      fuel: s.start.fuel,
      carrier: s.start.carrier,
      availableWeapons: [...(s.start.availableWeapons || [])],
      defaultWeapons: [...(s.start.defaultWeapons || [])],
    },
    entities,
    skyColor: [...s.skyColor],
  };

  // Weapons definitions
  if (state.weapons && state.weapons.length > 0) {
    data.weapons = state.weapons.map(w => ({
      id: w.id,
      name: w.name,
      category: w.category,
      range: w.range,
      damage: w.damage,
      ammoCapacity: w.ammoCapacity,
      rateOfFire: w.rateOfFire,
      speed: w.speed,
      maxVelocity: w.maxVelocity,
      agility: { ...w.agility },
      effects: { ...w.effects },
      playerSelectable: w.playerSelectable,
      playerDefault: w.playerDefault,
    }));
  }

  if (s.objectives.length > 0) {
    data.objectives = s.objectives.map(o => ({ ...o }));
  }

  if (s.completion) {
    data.completion = { ...s.completion };
  }

  return { data };
}

export function downloadJSON(state) {
  const json = exportScenario(state);
  const text = JSON.stringify(json, null, 2);
  const blob = new Blob([text], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = (state.scenario.id || 'scenario') + '.json';
  a.click();
  URL.revokeObjectURL(url);
}
