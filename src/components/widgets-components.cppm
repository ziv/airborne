module;

export module Components:Widgets;

import Types;

export struct DashboardSlot {
  int slot_index;
};

export struct CockpitWidget {};

export struct HudWidget {
  HudConfig cfg{};
  Pixel center_x{};
  Pixel center_y{};
  Pixel pixels_per_degree{};
  float ppd{};
  float tilt{};
  int color_index{};
};

export struct RadarWidget {
  RadarConfig cfg{};
  int range_count = 0;
  int range_index = 0;
};

export struct MinimapWidget {
  MinimapConfig cfg{};
  int map_zoom = 14;  // slippy-map tile zoom level (1–20); X = zoom in, Z = zoom out
};

export struct EngineWidget {};

// todo remove
export struct CameraWidget {};

export struct TargetCameraWidget {
  int render_tex_id = 0;
  int size = 150;
};
