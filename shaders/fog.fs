#version 330

// משתנים שמגיעים מה-Vertex Shader
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;

// טקסטורת הבסיס של המודל
uniform sampler2D texture0;

// משתני ערפל (אנחנו נשלח אותם מ-C++)
uniform vec3 cameraPos;    // מיקום המצלמה הנוכחי
uniform vec4 fogColor;     // צבע הערפל (למשל אפור-כחול)
uniform float fogDensity;  // כמה הערפל סמיך (0.0 עד 1.0)

// משתנה יציאה (הצבע הסופי של הפיקסל על המסך)
out vec4 finalColor;

void main()
{
    // 1. מקבלים את הצבע המקורי של הטקסטורה בנקודה זו
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 baseColor = texelColor * fragColor;

    // 2. חישוב המרחק של הפיקסל מהמצלמה (רק על מישור הקרקע XZ, בלי גובה)
    vec2 distVec = fragPosition.xz - cameraPos.xz;
    float distance = length(distVec);

    // 3. נוסחת ערפל אקספוננציאלית (Exp2 נותן מראה טבעי יותר מלינארי)
    // ככל שהמרחק או הסמיכות גדלים, fogFactor קטן לכיוון 0.0
    float fogFactor = 1.0 / exp(pow(distance * fogDensity, 2.0));

    // הגבלה של ה-Factor בין 0 ל-1 ליתר ביטחון
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // 4. ערבוב (Mix): הצבע הסופי הוא שילוב של צבע הקרקע וצבע הערפל
    // אם fogFactor=1 (קרוב מאוד), נראה רק קרקע.
    // אם fogFactor=0 (רחוק מאוד), נראה רק ערפל.
    finalColor = mix(fogColor, baseColor, fogFactor);

    // מוודאים שהאלפא נשארת מלאה
    finalColor.a = baseColor.a;
}