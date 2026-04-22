# Scenario Builder

The Scenario Builder is a tool that allows you to create scenarios for the flight simulator application.

## General Requirements

- [ ] Vanilla JavaScript web application with no build required (ESM modules allowed)
- [ ] Every aspect should be configurable to be able to support more items, entities, objectives, etc. in the future without changing the code (JSON files for
  configuration).

### UI Requirements

- [ ] Desktop version only, no need for responsive design
- [ ] Dark/light/high contrast themes
- [ ] Main layout of the workspace should dynamically adjust to the size of the window and should be resizable by the user. The layout should consist of:
    - [ ] top toolbar with common actions
    - [ ] bottom status bar
    - [ ] a left panel for tools, entities and objectives
    - [ ] a main area for the map and scenario editing
    - [ ] a content sensitive right panel for properties and settings of the selected item on the map, current scenario or any other **current context**.
- [ ] Prefer icons to text for small buttons, but provide tooltips for all buttons and icons.
- [ ] The app should have a consistent and intuitive user interface that follows common design patterns for desktop applications. The user should be able to
  easily navigate and use the app without needing to read a manual or watch a tutorial.

## Features

### Map

The simulator is use a tiles based map. For each tile there are 2 files. A texture file 1024x1024 and a heightmap file of 512x512 pixels.

Files named are in the format `tex-x-z.png` and `hm-x-z.png` where x and z are the coordinates of the tile.

- [ ] The app should allow the user to select a folder containing the map tiles and load them into the app. Not all tiles need to be loaded at once, the app
  should load the tiles as needed when the user navigates the map.
- [ ] The app should keep permissions to the folder and load the tiles as needed when the user navigates the map.
- [ ] The user should be able to see the loaded tiles in grayscale or in color.
- [ ] The user should be able to toggle the visibility of the heightmap and texture layers.
- [ ] Clicking and dragging on the map should allow the user to pan around the map.
- [ ] The user should be able to zoom in and out of the map using the mouse wheel.
- [ ] There should be a minimap that shows the entire map and the current view area.
- [ ] Clicking on the minimap should allow the user to jump to that location on the main map.
- [ ] Allow user to input the ratio of the map (pixes per meter) to be able to calculate distances on the map.

### Left Panel

The left is the main panel and it contains 3 sections:

1. The toolbox section where the user can select the tools to use on the map.
2. The entities section where the user can see the items added to the map as a clickable list.
3. The objectives section where the user can see the objectives added to the map as a clickable list.

#### Toolbox

The toolbox contains the following tools:

- [ ] Selection tool (arrow -default tool): allows the user to select and move entities on the map.
- [ ] Entities that can be added to the map (e.g. planes, helicopters, vehicles, airbase, ships, carrier, etc.). Clicking on an entity in the list should allow
  the user to place
  it on the map by clicking on the desired location. After placing an entity the tool should switch back to the selection tool.
- [ ] Selector tool: allows the user to select multiple entities on the map by clicking and dragging a selection box around them. The selected entities should
  be highlighted.