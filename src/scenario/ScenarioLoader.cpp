/**
 * @file ScenarioLoader.cpp
 * @brief JSON deserialization for scenarios, entities, objectives, and triggers.
 */
#include "ScenarioLoader.h"
#include "../primitives/Utils.h"
#include <filesystem>

using json = nlohmann::json;

Vector3 ScenarioLoader::parseVector3(const json &j) {
    return {j[0].get<float>(), j[1].get<float>(), j[2].get<float>()};
}

Faction ScenarioLoader::parseFaction(const std::string &s) {
    if (s == "friendly") return Faction::FRIENDLY;
    if (s == "neutral") return Faction::NEUTRAL;
    return Faction::ENEMY;
}

EntityType ScenarioLoader::parseEntityType(const std::string &s) {
    if (s == "aircraft") return EntityType::AIRCRAFT;
    if (s == "sam") return EntityType::SAM;
    if (s == "aaa") return EntityType::AAA;
    if (s == "structure") return EntityType::STRUCTURE;
    if (s == "naval") return EntityType::NAVAL;
    if (s == "airbase") return EntityType::AIRBASE;
    if (s == "waypoint") return EntityType::WAYPOINT;
    return EntityType::STRUCTURE;
}

ObjectiveType ScenarioLoader::parseObjectiveType(const std::string &s) {
    if (s == "destroy") return ObjectiveType::DESTROY;
    if (s == "photograph") return ObjectiveType::PHOTOGRAPH;
    if (s == "navigate") return ObjectiveType::NAVIGATE;
    if (s == "intercept") return ObjectiveType::INTERCEPT;
    if (s == "escort") return ObjectiveType::ESCORT;
    if (s == "sead") return ObjectiveType::SEAD;
    if (s == "survive") return ObjectiveType::SURVIVE;
    return ObjectiveType::DESTROY;
}

StartConditions ScenarioLoader::parseStart(const json &j) {
    StartConditions sc;
    if (j.contains("position")) sc.position = parseVector3(j["position"]);
    if (j.contains("heading")) sc.heading = j["heading"].get<float>();
    if (j.contains("speed")) sc.speed = j["speed"].get<float>();
    if (j.contains("altitude")) sc.altitude = j["altitude"].get<float>();
    if (j.contains("fuel")) sc.fuel = j["fuel"].get<float>();
    if (j.contains("carrier")) sc.carrier = j["carrier"].get<bool>();
    return sc;
}

WeaponLoadout ScenarioLoader::parseLoadout(const json &j) {
    WeaponLoadout wl;
    if (j.contains("slots")) wl.slots = j["slots"].get<int>();
    if (j.contains("available")) {
        for (const auto &w: j["available"]) wl.available.push_back(w.get<std::string>());
    }
    // if (j.contains("default")) {
    //     for (const auto &w: j["default"]) wl.defaultLoadout.push_back(w.get<std::string>());
    // }
    // wl.selectedLoadout = wl.defaultLoadout;
    return wl;
}

ScenarioScoring ScenarioLoader::parseScoring(const json &j) {
    ScenarioScoring ss;
    if (j.contains("objectiveComplete")) ss.objectiveComplete = j["objectiveComplete"].get<int>();
    if (j.contains("bonusObjective")) ss.bonusObjective = j["bonusObjective"].get<int>();
    if (j.contains("enemyAircraftKill")) ss.enemyAircraftKill = j["enemyAircraftKill"].get<int>();
    if (j.contains("samDestroyed")) ss.samDestroyed = j["samDestroyed"].get<int>();
    if (j.contains("aaaDestroyed")) ss.aaaDestroyed = j["aaaDestroyed"].get<int>();
    if (j.contains("timeBonus")) ss.timeBonus = j["timeBonus"].get<bool>();
    if (j.contains("fuelBonus")) ss.fuelBonus = j["fuelBonus"].get<bool>();
    if (j.contains("noDamageBonus")) ss.noDamageBonus = j["noDamageBonus"].get<int>();
    return ss;
}

Objective ScenarioLoader::parseObjective(const json &j) {
    Objective obj;
    obj.id = j["id"].get<std::string>();
    obj.type = parseObjectiveType(j["type"].get<std::string>());
    obj.label = j["label"].get<std::string>();
    if (j.contains("target")) obj.targetEntityId = j["target"].get<std::string>();
    if (j.contains("required")) obj.required = j["required"].get<bool>();
    if (j.contains("order")) obj.displayOrder = j["order"].get<int>();

    if (j.contains("params")) {
        const auto &p = j["params"];
        if (p.contains("minAltitude")) obj.params.minAltitude = p["minAltitude"].get<float>();
        if (p.contains("maxAltitude")) obj.params.maxAltitude = p["maxAltitude"].get<float>();
        if (p.contains("maxDistance")) obj.params.maxDistance = p["maxDistance"].get<float>();
        if (p.contains("requiredAngle")) obj.params.requiredAngle = p["requiredAngle"].get<float>();
        if (p.contains("arrivalRadius")) obj.params.arrivalRadius = p["arrivalRadius"].get<float>();
        if (p.contains("durationSeconds")) obj.params.durationSeconds = p["durationSeconds"].get<float>();
    }
    return obj;
}

EntityBase ScenarioLoader::parseEntity(const json &j) {
    EntityBase e;
    e.id = j["id"].get<std::string>();
    e.type = parseEntityType(j["type"].get<std::string>());
    if (j.contains("subtype")) e.subtype = j["subtype"].get<std::string>();
    if (j.contains("faction")) e.faction = parseFaction(j["faction"].get<std::string>());
    if (j.contains("position")) e.position = parseVector3(j["position"]);
    if (j.contains("heading")) e.heading = j["heading"].get<float>();
    if (j.contains("health")) {
        e.health = j["health"].get<float>();
        e.maxHealth = e.health;
    }
    if (j.contains("model")) e.modelId = j["model"].get<std::string>();
    return e;
}

Trigger ScenarioLoader::parseTrigger(const json &j) {
    Trigger t;
    t.id = j["id"].get<std::string>();
    if (j.contains("once")) t.once = j["once"].get<bool>();

    // parse condition
    t.condition.type = [&]() -> TriggerType {
        const auto &type = j["type"].get<std::string>();
        if (type == "zone_enter") return TriggerType::ZONE_ENTER;
        if (type == "zone_exit") return TriggerType::ZONE_EXIT;
        if (type == "entity_destroyed") return TriggerType::ENTITY_DESTROYED;
        if (type == "entity_detects_player") return TriggerType::ENTITY_DETECTS_PLAYER;
        if (type == "time_elapsed") return TriggerType::TIME_ELAPSED;
        if (type == "objective_complete") return TriggerType::OBJECTIVE_COMPLETE;
        if (type == "altitude_below") return TriggerType::ALTITUDE_BELOW;
        if (type == "altitude_above") return TriggerType::ALTITUDE_ABOVE;
        if (type == "speed_below") return TriggerType::SPEED_BELOW;
        return TriggerType::DAMAGE_TAKEN;
    }();

    if (j.contains("params")) {
        const auto &p = j["params"];
        if (p.contains("entityId")) t.condition.entityId = p["entityId"].get<std::string>();
        if (p.contains("zone")) t.condition.zoneCenter = parseVector3(p["zone"]);
        if (p.contains("radius")) t.condition.radius = p["radius"].get<float>();
        if (p.contains("seconds")) t.condition.value = p["seconds"].get<float>();
        if (p.contains("objectiveId")) t.condition.objectiveId = p["objectiveId"].get<std::string>();
    }

    // parse action
    if (j.contains("action")) {
        const auto &a = j["action"];
        t.action.type = [&]() -> TriggerActionType {
            const auto &type = a["type"].get<std::string>();
            if (type == "spawn_entity") return TriggerActionType::SPAWN_ENTITY;
            if (type == "destroy_entity") return TriggerActionType::DESTROY_ENTITY;
            if (type == "radio_message") return TriggerActionType::RADIO_MESSAGE;
            if (type == "update_objective") return TriggerActionType::UPDATE_OBJECTIVE;
            if (type == "play_sound") return TriggerActionType::PLAY_SOUND;
            if (type == "set_weather") return TriggerActionType::SET_WEATHER;
            return TriggerActionType::ACTIVATE_TRIGGER;
        }();
        if (a.contains("message")) t.action.message = a["message"].get<std::string>();
        if (a.contains("entityId")) t.action.entityId = a["entityId"].get<std::string>();
        if (a.contains("objectiveId")) t.action.objectiveId = a["objectiveId"].get<std::string>();
        if (a.contains("soundFile")) t.action.soundFile = a["soundFile"].get<std::string>();
    }
    return t;
}

Scenario ScenarioLoader::parseScenario(const json &data) {
    Scenario s;
    s.id = data["id"].get<std::string>();
    s.name = data["name"].get<std::string>();
    s.description = data["description"].get<std::string>();
    if (data.contains("difficulty")) s.difficulty = data["difficulty"].get<std::string>();
    if (data.contains("theater")) s.theater = data["theater"].get<std::string>();

    if (data.contains("start")) s.start = parseStart(data["start"]);
    if (data.contains("loadout")) s.loadout = parseLoadout(data["loadout"]);
    if (data.contains("scoring")) s.scoring = parseScoring(data["scoring"]);

    if (data.contains("objectives")) {
        for (const auto &obj: data["objectives"]) {
            s.objectives.push_back(parseObjective(obj));
        }
    }

    if (data.contains("entities")) {
        for (const auto &ent: data["entities"]) {
            s.entityDefinitions.push_back(parseEntity(ent));
        }
    }

    if (data.contains("triggers")) {
        for (const auto &trg: data["triggers"]) {
            s.triggers.push_back(parseTrigger(trg));
        }
    }

    TraceLog(LOG_INFO, "Scenario parsed: %s (%zu entities, %zu objectives, %zu triggers)",
             s.name.c_str(), s.entityDefinitions.size(), s.objectives.size(), s.triggers.size());
    return s;
}

Scenario ScenarioLoader::load(const std::string &path) {
    TraceLog(LOG_INFO, "Loading scenario: %s", path.c_str());
    const json data = UtilsLoaders::LoadJson(path);
    return data.get<Scenario>();
}

std::vector<Scenario> ScenarioLoader::loadAll(const std::string &configPath) {
    TraceLog(LOG_INFO, "Loading all scenarios from: %s", configPath.c_str());
    const json data = UtilsLoaders::LoadJson(configPath);

    std::vector<Scenario> scenarios;
    if (data.contains("scenarios")) {
        for (const auto &entry: data["scenarios"]) {
            try {
                scenarios.push_back(entry.get<Scenario>());
            } catch (const std::exception &e) {
                TraceLog(LOG_WARNING, "Failed to parse scenario: %s", e.what());
            }
        }
    }
    TraceLog(LOG_INFO, "Loaded %zu scenarios", scenarios.size());
    return scenarios;
}

std::vector<std::string> ScenarioLoader::listAvailable(const std::string &scenariosDir) {
    std::vector<std::string> files;
    try {
        for (const auto &entry: std::filesystem::directory_iterator(scenariosDir)) {
            if (entry.path().extension() == ".jsonc") {
                files.push_back(entry.path().string());
            }
        }
        std::sort(files.begin(), files.end());
    } catch (const std::filesystem::filesystem_error &e) {
        TraceLog(LOG_WARNING, "Could not list scenarios in %s: %s", scenariosDir.c_str(), e.what());
    }
    return files;
}
