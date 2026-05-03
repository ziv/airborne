module;
#include <string>

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