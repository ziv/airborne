/**
 * @file CockpitView.cpp
 * @brief Chroma-keyed cockpit rendering and indicator light sprites.
 */
#include "CockpitView.h"

#include "cockpit/CockpitViews.h"


CockpitView::CockpitView(GameData &gameData, const AppConfig &config) : cockpitTexture(LoadTexture(
                                                                            config.get<std::string_view>("/game/cockpitTexture").data())),
                                                                        cockpitShader(LoadShader(
                                                                            nullptr, config.get<std::string_view>("/game/cockpitChroma").data())),
                                                                        sprites(LoadTexture(config.get<std::string_view>("/game/cockpitSprites").data())),
                                                                        minihud(config),
                                                                        mapView(config) {
    // chroma shader to make cockpit transparent
    constexpr float thresholdValue = 0.7f;
    SetShaderValue(cockpitShader, GetShaderLocation(cockpitShader, "threshold"), &thresholdValue, SHADER_UNIFORM_FLOAT);
}

void CockpitView::update(const AircraftState &state, const float dt) {
    // rotate screens (there are A, B and C)
    if (IsKeyPressed(KEY_F2)) paneA = (paneA + 1) % 4;
    if (IsKeyPressed(KEY_F3)) paneB = (paneB + 1) % 4;
    if (IsKeyPressed(KEY_F4)) paneC = (paneC + 1) % 4;

    minihud.update(state, dt);

    // only update views if they are rendered
    mapView.update(state, dt);
    radarView.update();
}

void CockpitView::draw(const AircraftState &state) {
    BeginShaderMode(cockpitShader);
    DrawTexture(cockpitTexture, 0.0f, 0.0f, WHITE);
    EndShaderMode();
    // if (state.controls.brakes) {
    //     DrawTextureRec(sprites, (Rectangle){0, 0, 19, 19}, (Vector2){340.0f, 494.0f}, WHITE);
    // }
    // if (state.controls.gear) {
    //     DrawTextureRec(sprites, (Rectangle){19, 0, 39, 38}, (Vector2){320.0f, 515.0f}, WHITE);
    // }
    // mapView.draw(Views::PlanBPos);
    minihud.draw(state);

    // mapView.draw(Views::PlanBPos);
    // radarView.draw(state, Views::PlanCPos);
    // radarView.draw()

    drawPane(state, paneA, Views::PlanAPos);
    drawPane(state, paneB, Views::PlanBPos);
    drawPane(state, paneC, Views::PlanCPos);
}

void CockpitView::drawPane(const AircraftState &state, const int pane, const Vector2 &position) {
    switch (pane) {
        case 0:
            mapView.draw(position);
            break;
        case 1:
            radarView.draw(state, position);
            break;
        case 2:
            offView.draw(state, position);
            break;
        case 3:
            engineView.draw(state, position);
            break;
        default:
            break;
    }
}
