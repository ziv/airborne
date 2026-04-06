#version 330

// משתני כניסה (מגיעים מ-Raylib)
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// מטריצות טרנספורמציה (מגיעות מ-Raylib)
uniform mat4 mvp;
uniform mat4 matModel;

// משתני יציאה (עוברים ל-Fragment Shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
    // מחשבים את מיקום הפיקסל בעולם האמיתי (קריטי לחישוב מרחק)
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    // המיקום הסופי על המסך
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}