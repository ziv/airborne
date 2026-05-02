module;
#include <entt/entt.hpp>

#include "../lib/ray.hpp"

export module Components:Global;

import Types;

export struct Configuration {
  AppConfig conf{};
};

// global game state

export enum class GameStatus { INITIALIZE, LOADING, PLAYING, PAUSED, GAME_OVER };

export struct GameState {
  GameStatus status = GameStatus::INITIALIZE;
};

// player/game options
export struct GameOptions {
  bool changed = false;
  float tilt = 0.25f;
  float fov = 85.0f;
  std::string tiles_token;
  std::string maps_token;
};