#include "CockpitView.h"

CockpitView::CockpitView(const AppConfig &config) : cockpitTexture(LoadTexture(config.gameCockpitTexture.c_str())),
                                                         cockpitShader(LoadShader(nullptr, config.gameCockpitChroma.c_str())) {
    constexpr float thresholdValue = 0.5f;
    SetShaderValue(cockpitShader, GetShaderLocation(cockpitShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);
}

void CockpitView::draw() const {
    BeginShaderMode(cockpitShader);
    DrawTextureEx(cockpitTexture, {0.0f, -270.0f}, 0, 1.0f, WHITE);
    EndShaderMode();
}
