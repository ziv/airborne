#include "GameplayScreen.h"

#include "Constants.h"

GameplayScreen::GameplayScreen() : cockpit(LoadTexture("res/cockpit-05.png")) {
    playerCamera.place({0.0f, 10.0f, -20.0f}, {0.0f, 10.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    playerInput.Speed = 5.0f;
}

GameplayScreen::~GameplayScreen() {
    UnloadTexture(cockpit);
}

ScreenState GameplayScreen::Update() {
    playerInput.Pitch = 0.0f;
    playerInput.Yaw = 0.0f;
    playerInput.Roll = 0.0f;

    const float deltaTime = GetFrameTime();

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) playerInput.Pitch = -1.0f * deltaTime;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) playerInput.Pitch = 1.0f * deltaTime;;

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) playerInput.Roll = -1.0f * deltaTime;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) playerInput.Roll = 1.0f * deltaTime;

    // todo for debug only, not suppose to be part of the game
    if (IsKeyDown(KEY_Q)) playerInput.Yaw = 0.5f * deltaTime;
    if (IsKeyDown(KEY_E)) playerInput.Yaw = -0.5f * deltaTime;

    if (IsKeyDown(KEY_LEFT_SHIFT)) playerInput.Speed += 20.0f * deltaTime;
    if (IsKeyDown(KEY_LEFT_CONTROL)) playerInput.Speed -= 20.0f * deltaTime;
    if (playerInput.Speed <= 0.0f) playerInput.Speed = 0.0f;
    if (playerInput.Pitch > 2000.0f) playerInput.Pitch = 2000.0f;

    if (IsKeyDown(KEY_R)) {
        playerCamera.levelOut(1.0f, playerInput.Speed);
    } else {
        playerCamera.move(playerInput);
    }

    return ScreenState::GAMEPLAY;
}

void GameplayScreen::Draw() {
    ClearBackground(BLUE);
    BeginMode3D(playerCamera.GetRaylibCamera());
    DrawGrid(100, 20.0f);
    DrawCube({100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, RED);
    DrawCube({300.0f, 10.0f, 300.0f}, 10.0f, 10.0f, 10.0f, MAROON);
    DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, PURPLE);
    DrawCube({-100.0f, 10.0f, -100.0f}, 10.0f, 10.0f, 10.0f, GREEN);
    DrawCube({0.0f, 10.0f, 0.0f}, 10.0f, 10.0f, 10.0f, ORANGE);

    // DrawCubeWires({0.0f, 10.0f, 100.0f}, 10.0f, 10.0f, 10.0f, MAROON);
    EndMode3D();

    DrawText(TextFormat("SPEED: %0.f", playerInput.Speed), 20, 20, 20, BLACK);
    DrawText(TextFormat("X: %0.f", playerCamera.GetRaylibCamera().position.x), 20, 40, 20, BLACK);
    DrawText(TextFormat("Y: %0.f", playerCamera.GetRaylibCamera().position.z), 20, 60, 20, BLACK);
    DrawText(TextFormat("H: %0.f", playerCamera.GetRaylibCamera().position.y), 20, 80, 20, BLACK);
    // DrawText("Use WASD/Arrows to fly. Q/E to Yaw. Shift/Ctrl for Speed.", 20, 50, 20, LIGHTGRAY);

    // כוונת צלב פשוטה באמצע המסך הוירטואלי שלנו
    constexpr int centerX = GameConfig::SCREEN_WIDTH / 2;
    constexpr int centerY = GameConfig::SCREEN_HEIGHT / 2;
    DrawLine(centerX - 15, centerY, centerX + 15, centerY, BLACK);
    DrawLine(centerX, centerY - 15, centerX, centerY + 15, BLACK);

    DrawHud();
}

void GameplayScreen::DrawHud() const {
    DrawTexture(cockpit, -8, 0, WHITE);
    DrawText(TextFormat("%0.f", playerInput.Speed), 510, 260, 15, DARKGREEN);
    DrawText(TextFormat("%0.f", playerCamera.GetRaylibCamera().position.y), 760, 260, 15, DARKGREEN);

    constexpr int hudSize = 280;
    // int hudWidth = 300;
    // int hudHeight = 300;
    constexpr int hudX = (GameConfig::SCREEN_WIDTH - hudSize) / 2;
    constexpr int hudY = (GameConfig::SCREEN_HEIGHT - hudSize) / 2 - 100;
    BeginScissorMode(hudX, hudY, hudSize, hudSize);
    // ==========================================
    // ציור ממשק משתמש (HUD)
    // ==========================================

    // קבלת צירי המטוס הנוכחיים מהמצלמה
    Vector3 forward = playerCamera.GetForward();
    Vector3 up = playerCamera.GetUp();
    Vector3 right = playerCamera.GetRight();

    // --- א. חישוב זווית הגלגול (שיפוע הקו) ---
    // נמצא וקטור תלת-ממדי שמקביל תמיד לאופק של כדור הארץ (ניצב לכיוון המבט שלנו ול"שמיים")
    Vector3 horizonDir3D = Vector3CrossProduct(forward, {0.0f, 1.0f, 0.0f});

    // הגנת קצה: אם המטוס טס בול 90 מעלות למעלה/למטה, המכפלה תתאפס
    if (Vector3Length(horizonDir3D) < 0.001f) {
        horizonDir3D = right;
    } else {
        horizonDir3D = Vector3Normalize(horizonDir3D);
    }

    // נתרגם את הווקטור התלת-ממדי לשיפוע (dx, dy) על המסך שלנו
    // על ידי בדיקה כמה ממנו פונה ימינה (right) וכמה למעלה (up) ביחס לעיניים שלנו
    float dx = Vector3DotProduct(horizonDir3D, right);
    float dy = -Vector3DotProduct(horizonDir3D, up); // מינוס כי ציר ה-Y במסך הולך הפוך

    // --- ב. מציאת ציר ה"שמיים" על המסך ---
    // אם dx, dy זה ימינה ושמאלה לאורך האופק, אז 90 מעלות לזה יהיה למעלה ולמטה
    float skyDirX = dy;
    float skyDirY = -dx;

    // --- ב. חישוב העלרוד (הסטה אנכית) ---
    // כמה המטוס מסתכל למעלה/למטה? (מחזיר זווית ברדיאנים)
    float pitch = asinf(forward.y);

    // חישוב יחס: כמה פיקסלים על המסך שווים לזווית של רדיאן אחד?
    // משתמשים ב-FOV של המצלמה (85 מעלות) שעליו "נמתח" הגובה של המסך
    float fovRadians = 85.0f * PI / 180.0f;
    float pixelsPerRadian = GameConfig::SCREEN_HEIGHT / fovRadians;

    // הסטת הקו מהמרכז: אף עולה (pitch חיובי) -> הקו יורד למטה במסך
    float pitchOffset = pitch * pixelsPerRadian;

    // --- ג. ציור הקו ---
    // float centerX = GameConfig::SCREEN_WIDTH / 2.0f;
    // float cameraTiltOffset = playerCamera.TiltDown * pixelsPerRadian;
    // // float centerY = (GameConfig::SCREEN_HEIGHT / 2.0f) + pitchOffset;
    // float centerY = (GameConfig::VIRTUAL_HEIGHT / 2.0f) - cameraTiltOffset;
    // 1. ניקח נקודה דמיונית שנמצאת בדיוק מול האף של המטוס (מרחק עצום קדימה)
    Vector3 nose3D = Vector3Add(playerCamera.GetRaylibCamera().position, Vector3Scale(forward, 10000.0f));

    // 2. נמיר את הנקודה התלת-ממדית לקואורדינטות פיקסלים על המסך הוירטואלי שלנו!
    Vector2 nose2D = GetWorldToScreenEx(nose3D, playerCamera.GetRaylibCamera(), GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);

    // 3. זה מרכז ה-HUD האמיתי והמדויק מתמטית
    float centerX = nose2D.x;
    float centerY = nose2D.y;

    // --- ד. ציור סולם העלרוד (Pitch Ladder) ---
    // נרוץ ממינוס 80 מעלות (צלילה) עד פלוס 80 מעלות (נסיקה) בקפיצות של 20
    for (int currentAngle = -80; currentAngle <= 80; currentAngle += 20) {
        // המרת המעלה של הקו הנוכחי לרדיאנים
        float targetPitch = currentAngle * PI / 180.0f;

        // כמה רדיאנים הקו הזה רחוק ממרכז הכוונת שלנו?
        float deltaPitch = targetPitch - pitch;

        // המרה למרחק בפיקסלים על המסך
        float pixelDist = deltaPitch * pixelsPerRadian;

        // מציאת נקודת האמצע של השלב הנוכחי בסולם
        float rungCenterX = centerX + skyDirX * pixelDist;
        float rungCenterY = centerY + skyDirY * pixelDist;

        // קו האופק (0) יהיה ארוך יותר משאר הקווים
        float rungLength = (currentAngle == 0) ? 110.0f : 90.0f;

        // נקודות הקצה של הקו (נמתחות לאורך dx, dy)
        Vector2 startPos = {rungCenterX - dx * rungLength, rungCenterY - dy * rungLength};
        Vector2 endPos = {rungCenterX + dx * rungLength, rungCenterY + dy * rungLength};

        // משאירים רווח באמצע לכוונת
        Vector2 midLeft = {rungCenterX - dx * 30.0f, rungCenterY - dy * 30.0f};
        Vector2 midRight = {rungCenterX + dx * 30.0f, rungCenterY + dy * 30.0f};

        // בחירת עובי הקו (האופק מודגש יותר)
        float thickness = (currentAngle == 0) ? 2.0f : 1.0f;

        DrawLineEx(startPos, midLeft, thickness, GREEN);
        DrawLineEx(midRight, endPos, thickness, GREEN);

        // ציור ה"פסיקים" בקצוות של כל שלב
        if (currentAngle == 0) {
            // לאופק עצמו הפסיקים יורדים קצת למטה
            DrawLineEx(startPos, {startPos.x + dy * 15.0f, startPos.y - dx * 15.0f}, thickness, GREEN);
            DrawLineEx(endPos, {endPos.x + dy * 15.0f, endPos.y - dx * 15.0f}, thickness, GREEN);
        } else {
            // במטוסים אמיתיים, הפסיקים של הסולם תמיד מצביעים לכיוון קו האופק!
            // אם אנחנו בזווית חיובית - הפסיק יצביע למטה (נגד skyDir).
            // אם בזווית שלילית - הפסיק יצביע למעלה (עם skyDir).
            float tickDirX = (currentAngle > 0) ? -skyDirX : skyDirX;
            float tickDirY = (currentAngle > 0) ? -skyDirY : skyDirY;

            DrawLineEx(startPos, {startPos.x + tickDirX * 15.0f, startPos.y + tickDirY * 15.0f}, thickness, GREEN);
            DrawLineEx(endPos, {endPos.x + tickDirX * 15.0f, endPos.y + tickDirY * 15.0f}, thickness, GREEN);

            // כיתוב המעלה ליד הקו
            DrawText(TextFormat("%d", currentAngle), startPos.x - 35, startPos.y - 10, 20, GREEN);
        }
    }
    //
    //
    //
    // float lineLength = 100.0f; // חצי מאורך הקו שיוצג
    //
    // // נקודת התחלה וסיום של קו האופק
    // Vector2 startPos = {centerX - dx * lineLength, centerY - dy * lineLength};
    // Vector2 endPos = {centerX + dx * lineLength, centerY + dy * lineLength};
    //
    // // ב-HUD אמיתי יש רווח באמצע עבור כוונת הטיסה, אז נצייר שני קווים מופרדים
    // Vector2 midLeft = {centerX - dx * 30.0f, centerY - dy * 30.0f};
    // Vector2 midRight = {centerX + dx * 30.0f, centerY + dy * 30.0f};
    //
    // DrawLineEx(startPos, midLeft, 2.0f, GREEN);
    // DrawLineEx(midRight, endPos, 2.0f, GREEN);
    //
    // // נוסיף פסיקים קטנים בקצוות כדי שזה ייראה כמו אופק מקצועי
    // DrawLineEx(startPos, {startPos.x + dy * 15.0f, startPos.y - dx * 15.0f}, 2.0f, GREEN);
    // DrawLineEx(endPos, {endPos.x + dy * 15.0f, endPos.y - dx * 15.0f}, 2.0f, GREEN);
    EndScissorMode();
    DrawRectangleLines(hudX, hudY, hudSize, hudSize, DARKGREEN);
    DrawText(TextFormat("SPD: %0.f", playerInput.Speed), hudX - 80, hudY + hudSize / 2, 20, GREEN);
}
