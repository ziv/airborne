module;

export module Screens:Base;

import JsonConfig;

export enum class ScreenState {
  SPLASH,
  LOADING,
  MAIN_MENU,
  BRIEFING,
  GAMEPLAY,
  DEBRIEF,
  HELP,
  EXIT
};

export class BaseScreen {
public:
  virtual ~BaseScreen() = default;

  virtual ScreenState update() = 0;

  virtual void draw() = 0;
};
