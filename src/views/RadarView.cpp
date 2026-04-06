#include "RadarView.h"
#include "raymath.h"

void RadarView::update() {
    if (IsKeyPressed(KEY_R)) {
        TraceLog(LOG_ERROR, "RRRR");
        if (1000 == currentRadarRange) {
            currentRadarRange = 10000;
            return;
        }
        if (10000 == currentRadarRange) {
            currentRadarRange = 100000;
            return;
        }
        if (100000 == currentRadarRange) {
            currentRadarRange = 1000;
            return;
        }
    }
}

void RadarView::draw(const AircraftState &state, Vector3 enemy) {
    // DrawRectangle(303, 600, 148, 148, BLACK);

    // DrawCircle(377, 684, 20, GREEN);
    DrawCircleLines(377, 684, 30, DARKGREEN);
    DrawCircleLines(377, 684, 60, DARKGREEN);
    DrawText(TextFormat("RADAR %f.", currentRadarRange), 305, 602, 10, DARKGREEN);

    const Vector3 relPos = Vector3Subtract(enemy, state.position);
    Vector3 playerRight = {-state.orientation.forward.z, 0.0f, state.orientation.forward.x};

    const float radarY = (relPos.x * state.orientation.forward.x) + (relPos.z * state.orientation.forward.z);
    const float radarX = (relPos.x * playerRight.x) + (relPos.z * playerRight.z);

    float distanceToEnemy = sqrtf(radarX * radarX + radarY * radarY);
    if (distanceToEnemy < currentRadarRange) {
        float radarRadiusPixels = 74.0f; //  (148x148)
        float pixelsPerMeter = radarRadiusPixels / currentRadarRange;

        float finalScreenX = radarCenter.x - (radarX * pixelsPerMeter);
        float finalScreenY = radarCenter.y + (radarY * pixelsPerMeter);

        DrawRectangle((int) finalScreenX - 3, (int) finalScreenY - 3, 6, 6, RED);
        int enemyAltFeet = (int) (enemy.y * 3.28084f);
        DrawText(TextFormat("%d", enemyAltFeet), (int) finalScreenX + 5, (int) finalScreenY - 5, 10, WHITE);
    }
}
