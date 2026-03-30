#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float time; // בשביל תנועה קלה של הברק

out vec4 finalColor;

void main() {
    // 1. קריאת הצבע המקורי מהמפה
    vec4 texelColor = texture(texture0, fragTexCoord);

    // 2. יצירת אפקט "קימור" (כהות בקצוות)
    float dist = distance(fragTexCoord, vec2(0.5, 0.5));
    float vignette = smoothstep(0.7, 0.4, dist);
    texelColor.rgb *= (vignette * 0.2 + 0.8);

    // 3. יצירת ברק (Glare) אלכסוני
    // הנוסחה הזו יוצרת פס לבן שזז לפי הזמן
    float glare = smoothstep(0.4, 0.5, sin(fragTexCoord.x + fragTexCoord.y + time * 0.5) * 0.5 + 0.5);
    vec3 glareColor = vec3(1.0, 1.0, 1.0) * glare * 0.15; // עוצמת הברק

    // 4. הוספת גוון ירקרק עדין של זכוכית צבאית
    vec3 glassTint = vec3(0.9, 1.0, 0.9);

    finalColor = vec4((texelColor.rgb * glassTint) + glareColor, texelColor.a);
}