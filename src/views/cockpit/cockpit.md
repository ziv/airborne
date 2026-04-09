# Cockpit View

- [x] Main texture with opacity.
- [x] HUD
- [ ] Instruments (altimeter, speedometer, gear, etc.)
- [ ] Indicators (afterburner, brakes, etc.)
- [x] Sub-Views (radar, map, weapon status, etc.) [partially implemented]

## Sub-Views

Sub-Views are changeable, and can be switched by the player from a list of available views.
Each view has its own texture and rendering logic.

Instead of dynamic sub-views, we support 3 fixed sub-views, and the player can switch their content by pressing `F2`,
`F3`, and `F4` keys. Means, all available sub-views are initiated at the start of the game, but their update and rending
logic is only executed when they are active. We don't want to create and destroy objects in runtime, it is better to
just hide them when they are not active.

Sub-Views are configured in the `cockpit.jsoc` JSON file. If we don't want to use a sub-view, we can just set it to
`null` in the configuration file. They mapped as `A`, `B`, and `C` in the configuration file, and they are mapped to
`F2`, `F3`, and `F4` keys respectively.

```json5
{
  // Map of sub-views
  // and their configuration
  "panels": {
    "A": {
      "position": [
        100,
        200
      ],
      "size": [
        150,
        150
      ]
    },
  }
}
```

### Implemented Sub-Views:

- [x] Map View
- [x] Radar View
- [ ] Weapon Status View
- [ ] Targeting View
  I
