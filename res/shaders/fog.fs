#version 330

// נתונים שמגיעים מה-Vertex Shader
in vec2 fragTexCoord;
in vec4 fragColor;
in float fragDistance;

// טקסטורת הבסיס של השטח
uniform sampler2D texture0;

// משתנים (Uniforms) שאנחנו נעדכן מהקוד ב-C++
uniform vec3 skyColor;      // צבע השמיים (כחול-אפור)
uniform float fogNear;      // מאיזה מרחק הערפל מתחיל (למשל 5000)
uniform float fogFar;       // באיזה מרחק השטח נעלם לגמרי (למשל 30000)

// הצבע הסופי של הפיקסל שיצויר על המסך
out vec4 finalColor;

void main()
{
    // קבלת הצבע המקורי מהטקסטורה
    vec4 texelColor = texture(texture0, fragTexCoord);

    // שילוב צבע הטקסטורה עם צבע הקודקוד (למשל תאורה)
    vec4 baseColor = texelColor * fragColor;

    // חישוב פקטור הערפל (מספר בין 0.0 ל-1.0)
    // 0.0 = קרוב (אין ערפל), 1.0 = רחוק (ערפל מלא)
    // הפונקציה smoothstep יוצרת מעבר חלק
    float fogFactor = smoothstep(fogNear, fogFar, fragDistance);

    // "ערבוב" (Linear Interpolation) בין הצבע המקורי לצבע השמיים
    finalColor = mix(baseColor, vec4(skyColor, 1.0), fogFactor);

    // שמירה על השקיפות המקורית (בדרך כלל 1.0 בשטח)
    finalColor.a = baseColor.a;
}