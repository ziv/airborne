#pragma once
#include "raylib.h"
#include "../core/AircraftTransformation.h"

inline Texture2D GenerateHorizonTexture() {
    // 1. הגדרת רזולוציה לטקסטורה. יחס 2:1 מתאים לספירה.
    const int texWidth = 1024;
    const int texHeight = 1024;

    // 2. יצירת ה-Render Texture (ה"דף הוירטואלי")
    RenderTexture2D target = LoadRenderTexture(texWidth, texHeight);

    // 3. תחילת ציור דו-ממדי בתוך הטקסטורה
    BeginTextureMode(target);
    ClearBackground(BLANK);
    // DrawRectangle(0, 0, texWidth / 2, texWidth, DARKBROWN);
    // DrawRectangle(texWidth / 2, 0, texWidth, texWidth, YELLOW);

    // --- ציור קו המשווה (חוצה את הכדור לרוחב) ---
    // הוא יושב בדיוק באמצע גובה הטקסטורה
    float equatorY = texHeight / 2.0f;
    // DrawLineEx({0, 0}, {(float) 1024, (float) 1024}, 10.0f, WHITE);

    // DrawLineEx({0, texHeight/2}, {texWidth, texHeight/2}, 10.0f, RED);
    // DrawLineEx({texWidth/2, 0}, {texWidth/2, texHeight}, 10.0f, GREEN);

    for (int i = 0; i < 1024; i += 64) {
        DrawLineEx({512 + 100, (float) i}, {512 - 100, (float) i}, 5.0f, GREEN);
    }

    for (int i = 0; i < 1024; i += 64) {
        DrawLineEx({(float) i, 0}, {(float) i, 1024, }, 2.0f, WHITE);
    }

    // for (int i = 0; i < 512; i += 100) {
    //     // DrawLineEx((Vector2){equatorY, (float) i - 100.f},
    //     //            (Vector2){equatorY, (float) i + 100.0f},
    //     //            2.0f,
    //     //            WHITE);
    //     // DrawLineEx({0, (float)i}, {1024.0, (float)i}, 2.0f, WHITE);
    //     DrawLineEx({(float)i, 0}, {(float)i, (float)texHeight}, 2.0f, WHITE);
    // }
    // int numTicks = 36; // שנת כל 10 מעלות
    // float tickSpacing = (float) texWidth / numTicks;
    // int tickHeight = 20;
    //
    // for (int i = 0; i < numTicks; i++) {
    //     float x = i * tickSpacing;
    //     DrawLineEx({x, (float) (equatorY - tickHeight / 2)},
    //                {x, (float) (equatorY + tickHeight / 2)}, 2.0f, WHITE);
    //
    //     // בונוס: טקסט של מעלות (רק להדגמה)
    //     // if (i % 3 == 0) DrawText(TextFormat("%d", i*10), x + 5, equatorY + 10, 10, WHITE);
    // }

    // --- ציור קו גריניץ' (חוצה את הכדור מלמעלה למטה) ---
    // נשים אותו באמצע הרוחב כדי שיפנה אלינו
    int greenwichX = texWidth / 2;
    DrawLineEx({(float) greenwichX, 0}, {(float) greenwichX, (float) texHeight}, 3.0f, RED);

    // 4. סיום הציור לתוך הטקסטורה
    EndTextureMode();

    // 5. חשוב! Render Texture מחזיר אובייקט עם טקסטורה הפוכה (בגלל איך ש-OpenGL עובד).
    // אנחנו מחזירים רק את הטקסטורה הפנימית, ונצטרך לזכור להפוך אותה ברינדור המודל.
    return target.texture;
}

class MinihudView {
    Mesh sphereMesh = GenMeshSphere(1.0f, 32, 32);
    Model horizonModel;
    // Texture2D dynamicTex = GenerateHorizonTexture();
public:
    MinihudView() {
        Texture2D example = LoadTexture("res/navballs/g.png");
        // for (int i = 0; i < sphereMesh.vertexCount; i++) {
            // sphereMesh.texcoords[i * 2 + 1] = 1.0f - sphereMesh.texcoords[i * 2 + 1];
        // }
        horizonModel = LoadModelFromMesh(sphereMesh);
        // SetMaterialTexture(&horizonModel.materials[0], MATERIAL_MAP_DIFFUSE, GenerateHorizonTexture());
        SetMaterialTexture(&horizonModel.materials[0], MATERIAL_MAP_DIFFUSE, example);
    }

    // void update();
    void draw(const Camera &camera, const Directions &directions, const Quaternion &quaternion);
};
