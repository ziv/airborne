#include "CockpitView.h"

CockpitView::CockpitView(const AppConfig &config) : cockpitTexture(LoadTexture(config.get<std::string_view>("/game/cockpitTexture").data())),
                                                    cockpitShader(LoadShader(nullptr, config.get<std::string_view>("/game/cockpitChroma").data())) {
    constexpr float thresholdValue = 0.5f;
    SetShaderValue(cockpitShader, GetShaderLocation(cockpitShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);
}

void CockpitView::draw() const {
    BeginShaderMode(cockpitShader);
    DrawTexture(cockpitTexture, 0.0f, 0.0f, WHITE);
    // DrawTextureEx(cockpitTexture, {0.0f, 0.0f}, 0, 1.0f, WHITE);
    EndShaderMode();
}
