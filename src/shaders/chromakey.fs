#version 330

// קלטים מ-Raylib
in vec2 fragTexCoord;
in vec4 fragColor;

// התמונה של ה-Cockpit
uniform sampler2D texture0;

// הצבע שאנחנו רוצים להסיר (הירוק)
uniform vec3 keyColor = vec3(0.0, 1.0, 0.0); // ירוק טהור
uniform float threshold = 0.5; // רמת הדיוק של החיתוך

out vec4 finalColor;

void main() {
    // קריאת הצבע של הפיקסל הנוכחי בתמונה
    vec4 texelColor = texture(texture0, fragTexCoord);

    // חישוב המרחק בין צבע הפיקסל לצבע הירוק (Key Color)
    float diff = distance(texelColor.rgb, keyColor);

    // אם הצבע קרוב מספיק לירוק, נהפוך אותו לשקוף (Alpha = 0)
    if (diff < threshold) {
        finalColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        finalColor = texelColor * fragColor;
    }
}