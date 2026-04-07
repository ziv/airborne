#include "CockpitView.h"

CockpitView::CockpitView(const AppConfig &config) : cockpitTexture(LoadTexture(config.get<std::string_view>("/game/cockpitTexture").data())),
                                                    cockpitShader(LoadShader(nullptr, config.get<std::string_view>("/game/cockpitChroma").data())),
                                                    sprites(LoadTexture(config.get<std::string_view>("/game/cockpitSprites").data())) {
    constexpr float thresholdValue = 0.7f;
    SetShaderValue(cockpitShader, GetShaderLocation(cockpitShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);
}

void CockpitView::draw(const AircraftState &state) const {
    BeginShaderMode(cockpitShader);
    DrawTexture(cockpitTexture, 0.0f, 0.0f, WHITE);
    EndShaderMode();
    if (state.controls.brakes) {
        DrawTextureRec(sprites, (Rectangle){0, 0, 19, 19}, (Vector2){340.0f, 494.0f}, WHITE);
    }
    if (state.controls.gear) {
        DrawTextureRec(sprites, (Rectangle){19, 0, 39, 38}, (Vector2){320.0f, 515.0f}, WHITE);
    }
}
