#version 330

// נתונים שמגיעים מ-Raylib
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// מטריצות טרנספורמציה ש-Raylib מעדכן אוטומטית
uniform mat4 mvp;
uniform mat4 matModel;

// נתונים שאנחנו מעבירים ל-Fragment Shader
out vec2 fragTexCoord;
out vec4 fragColor;
out float fragDistance; // המרחק מהמצלמה

void main()
{
    // חישוב המיקום הסופי של הקודקוד על המסך
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    // העברת טקסטורה וצבע
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // חישוב המיקום המוחלט של הקודקוד בעולם
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);

    // חישוב המרחק (במטרים) מנקודת האפס (כי בשיטה שלנו המצלמה תמיד ב-0,0,0)
    // אנחנו משתמשים רק ב-X וב-Z כי הגובה (Y) פחות רלוונטי לערפל אופקי
    fragDistance = length(worldPosition.xz);
}