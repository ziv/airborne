/**
 * @file MinihudView.cpp
 * @brief Artificial horizon, speed/altitude readouts, RoC, and warning indicators.
 */
#include "MinihudView.h"
#include "raymath.h"
#include "rlgl.h"
#include "../../primitives/Utils.h"


MinihudView::MinihudView(const AppConfig &config) : ladderX(config.get<int>("/views/hudLadderX")),
                                                    ladderY(config.get<int>("/views/hudLadderY")),
                                                    ladderWidth(config.get<int>("/views/hudLadderWidth")),
                                                    ladderHeight(config.get<int>("/views/hudLadderHeight")),
                                                    fov(config.get<float>("/pilot/fov")),
                                                    width(config.get<int>("/config/screenWidth")),
                                                    height(config.get<int>("/config/screenHeight")),
                                                    ladderOffset(config.get<float>("/views/hudLadderOffset")) {
}

void MinihudView::update(const AircraftState &state, float dt) {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_H)) {
        color++;
        if (color > 2) color = 0;
    }
}

/**
 * HUD structure
 *
 * +---------------------------------------------------+
 * | A                     B                         C |
 * |                                                   |
 * | D                     E                         F |
 * |                                                   |
 * | G                     H                         I |
 * +---------------------------------------------------+
 *
 * D - Speed (knots)
 * E - Artificial horizon ladder
 * F - Height (feet), Height above ground in border (feet), RoC indicator
 * G - After burner alert
 */

///
void MinihudView::draw(const AircraftState &state) const {
    drawLadder(state);
    drawRateOfClimb(state);

    // speed indicator
    const auto speed = state.forces.speed * GamePhysics::MS_TO_KNOTS;
    DrawText(TextFormat("%s", FormatNumberSuffix(speed)), 430, 310, 15, colors[color]);

    // height indicator
    const auto heightAbsolute = state.position.y * GamePhysics::METERS_TO_FEET;
    const auto heightRelative = (state.position.y - state.groundHeight) * GamePhysics::METERS_TO_FEET;
    DrawText(TextFormat("%s", FormatNumberSuffix(heightAbsolute)), 750, 310, 15, colors[color]);
    DrawText(TextFormat("%s", FormatNumberSuffix(heightRelative)), 750, 330, 10, colors[color]);
    DrawRectangleLines(745, 328, 30, 14, colors[color]);

    // -- after burner warning --
    if (state.controls.throttle > 1.0f) {
        DrawText("A/B ON", 450, 390, 12, ORANGE);
    }

    // -- autopilot warning --
    if (state.autopilot) {
        DrawText("A/P ON", 450, 390, 12, colors[color]);
    }

    // --- low-altitude gear warning ---
    // If flying below 500 m AGL with gear retracted, flash a red GEAR alert
    // to remind the pilot to deploy gear before attempting to land.
    if (state.flying && !state.controls.gear) {
        const float agl = state.position.y - state.groundHeight;
        if (agl < 500.0f) {
            // blink at 4 Hz so it catches the pilot's eye
            if (static_cast<int>(GetTime() * 4) % 2 == 0) {
                const int textWidth = MeasureText("GEAR", 20);
                DrawText("GEAR", width / 2 - textWidth / 2, height / 2 + 20, 15, RED);
            }
        }
    }
}


void MinihudView::drawLadder(const AircraftState &state) const {
    BeginScissorMode(ladderX, ladderY, ladderWidth, ladderHeight);

    const auto fy = state.orientation.forward.y;
    const auto uy = state.orientation.up.y;
    const auto [x, y, z] = state.orientation.right;

    const auto pitch = asinf(fy) * RAD2DEG;
    const auto roll = (fabsf(fy) < 0.999f ? atan2f(-y, uy) : atan2f(z, x)) * RAD2DEG;

    const auto centerX = width / 2;
    const auto centerY = height / 2;

    const auto pixelsPerDegree = static_cast<float>(height) / fov;

    DrawCircleLines(centerX, centerY - static_cast<int>(ladderOffset), 5.0f, colors[color]);

    // freeze state
    rlPushMatrix();

    // take us to the center
    rlTranslatef(static_cast<float>(centerX), static_cast<float>(centerY) - ladderOffset, 0);

    // pitch & roll
    rlRotatef(-roll, 0, 0, 1);
    rlTranslatef(0, pitch * pixelsPerDegree, 0);

    // draw on the center

    // main line
    DrawLineEx({-100, 0}, {-20, 0}, 2, colors[color]);
    DrawLineEx({20, 0}, {100, 0}, 2, colors[color]);


    for (int i = -180; i <= 180; i += 15) {
        if (i == 0) continue;
        const auto lineY = -static_cast<float>(i) * pixelsPerDegree;
        // main line
        DrawLineEx({-100, lineY}, {-20, lineY}, 1, colors[color]);
        DrawLineEx({20, lineY}, {100, lineY}, 1, colors[color]);

        const auto to = i > 0 ? lineY + 10 : lineY - 10;
        DrawLineEx({100, lineY}, {110, to}, 1, colors[color]);
        DrawLineEx({-100, lineY}, {-110, to}, 1, colors[color]);
        DrawText(TextFormat("%d", i), -130, static_cast<int>(lineY) - 5, 10, colors[color]);
        DrawText(TextFormat("%d", i), 115, static_cast<int>(lineY) - 5, 10, colors[color]);
    }

    // resume from freeze
    rlPopMatrix();
    EndScissorMode();
}

void MinihudView::drawRateOfClimb(const AircraftState &state) const {
    const auto verticalSpeedFPM = state.forces.velocity.y * GamePhysics::MS_TO_FPM;
    constexpr float MAX_CLIMB_RATE_FPM = 50000.0f;
    const float MAX_BAR_PIXELS = (static_cast<float>(ladderHeight) / 2.0f) - 20.0f;

    float vsRatio = verticalSpeedFPM / MAX_CLIMB_RATE_FPM;
    if (vsRatio > 1.0f) vsRatio = 1.0f;
    if (vsRatio < -1.0f) vsRatio = -1.0f;

    const int currentBarHeight = static_cast<int>(vsRatio * MAX_BAR_PIXELS);
    const int maxBarPixels = static_cast<int>(MAX_BAR_PIXELS);

    constexpr int centerX = 740; // see speed location
    constexpr int centerY = 318;
    DrawLine(centerX, centerY - maxBarPixels, centerX, centerY + maxBarPixels, Fade(colors[color], 0.3f));
    DrawLine(centerX - 5, centerY, centerX + 5, centerY, colors[color]);

    if (currentBarHeight > 0) {
        DrawRectangle(centerX - 2, centerY - currentBarHeight, 4, currentBarHeight, colors[color]);
    } else {
        DrawRectangle(centerX - 2, centerY, 4, -currentBarHeight, colors[color]);
    }
}
