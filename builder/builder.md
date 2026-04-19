# Scenario Builder

A web application for creating and editing scenario files. It provides a visual interface for placing entities, defining objectives, and configuring mission
parameters. The builder generates JSON scenario files compatible with the game engine.

## Creating a scenario

1. Should allow user to choose an image (map) and display it as our working canvas.
2. Collect details about the map size ratio to the real world dimensions (e.g. 1 pixel = 10 meters).
3. The user should be able to mark areas on the map (e.g. airbases, carriers, nav points) and place entities (e.g. SAM sites, AAA, structures) with specific properties (e.g. SAM range, AAA caliber).
4. The user should be able to configure the aircraft/SAM/AAA weapons loadout.
5. The user should be able to define mission objectives (e.g. destroy specific targets, navigate to waypoints) and success/failure conditions.
6. The builder should validate the scenario for consistency (e.g. all referenced entities exist, objectives are achievable) and provide feedback on errors.
7. Finally, the builder should export the scenario as a JSON file that can be loaded by the game engine.

## Constraints

1. No external libraries or frameworks should be used. The builder should be implemented using vanilla JavaScript, HTML, and CSS.
2. The builder should be a single-page application (SPA) without page reloads.
3. The builder should be user-friendly and intuitive, with a clean ~~and responsive~~ design.
4. The builder should run on desktop browsers only (no mobile support required).
