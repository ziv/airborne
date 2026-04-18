module;
#include <entt/entt.hpp>
#include "lib/ray.hpp"

export module Game;

import JsonConfig;
import Player;
import Components;
import Prefabs;
import WorldStreamerSystem;
import RenderSystem;
import Types;
import ResourceManager;
import WidgetsInputs;
import Accessors;


export class Game {
    entt::registry &registry;
    Scenario scenario{};
    PlayerDispatcher dispatcher;

public:
    explicit Game(const JsonConfig &config,
                  const JsonConfig &scenario_config,
                  entt::registry &reg)
        : registry(reg),
          scenario(scenario_config.get<Scenario>("/data")),
          dispatcher(config) {
        registry.ctx().emplace<Offset>(Vector3Zero());
        factories::create_player(registry, config, scenario);
        factories::create_scene(registry, config);
        factories::create_cockpit(registry, config);
        factories::create_hud(registry, config);
        factories::create_cockpit_widgets(registry);

        updates::set_minimap(0, registry, config);
        updates::set_engine_status(1, registry);
        updates::set_radar(2, registry, config);

        // spawn all items from scenario
        for (const auto &def: scenario.entities) factories::create_unit(registry, def);
    }

    void update() {
        if (is_player_crashed(registry)) return;
        dispatcher.update(registry, GetFrameTime());
        WidgetsInputs(registry);
    }

    void draw() {
        ClearBackground(scenario.skyColor);

        // 3D
        BeginMode3D(dispatcher.playerCamera.getCamera());
        WorldStreamerSystem(registry);
        RenderModels(registry);
        RenderDebugging(registry);
        EndMode3D();

        // 2D
        RenderCockpit(registry);
        RenderMinimap(registry);
        RenderEngineStatus(registry);
        RenderHud(registry);
        RenderRadar(registry);
        DrawFPS(1050, 780);
        RenderDebug(registry);
        RenderCrashLayout(registry);
    }
};
