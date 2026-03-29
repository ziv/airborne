#pragma once
#include "raylib.h"
#include "../Constants.h"
#include "../primitives/GameData.h"

inline void DrawPowerGauge(const GameData &game) {
    float gaugeX = GetScreenWidth() * 0.72f;
    float gaugeY = GetScreenHeight() * 0.65f;
    float gaugeWidth = 40.0f;
    float maxHeight = 120.0f; // הגובה המקסימלי ב-100% עוצמה

    // 2. חישוב הגובה הנוכחי לפי ה-Throttle
    // ננרמל את ה-throttle (0.0 עד 1.2) לגובה הפיזי של המד
    float currentGaugeHeight = (game.throttle / 1.0f) * maxHeight;

    // 3. בחירת צבע לפי המצב
    Color gaugeColor = GREEN;
    if (game.throttle > 1.0f) {
        gaugeColor = RED; // מצב Afterburner!
    }

    // if (game.throttle > 1.0f) {
    //     // הבהוב קל לפי הזמן
    //     float alpha = abs(sin(GetTime() * 10.0f));
    //     gaugeColor = Fade(RED, alpha);
    //
    //     // רעד קל (Vibration)
    //     gaugeX += GetRandomValue(-1, 1);
    // }

    // ציור הרקע של המד (כהה ושקוף קצת)
    DrawRectangle(gaugeX, gaugeY - maxHeight, gaugeWidth, maxHeight, Fade(DARKGRAY, 0.5f));

    // ציור המד החי
    DrawRectangle(gaugeX, gaugeY - currentGaugeHeight, gaugeWidth, currentGaugeHeight, gaugeColor);

    // הוספת מסגרת דקה בשביל ה-Look
    DrawRectangleLines(gaugeX, gaugeY - maxHeight, gaugeWidth, maxHeight, WHITE);

    // הוספת טקסט אחוזים ליד
    DrawText(TextFormat("%i%%", (int)(game.throttle * 100)), gaugeX + 5, gaugeY + 10, 15, gaugeColor);
}
