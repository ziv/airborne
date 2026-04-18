#version 330

// inputs from raylib
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

// the chroma color we want to remove
uniform vec3 keyColor = vec3(0.0, 1.0, 0.0);
uniform float threshold = 0.7;

out vec4 finalColor;

void main() {
    // the current pixel color
    vec4 texelColor = texture(texture0, fragTexCoord);

    // the distance to our chroma
    float diff = distance(texelColor.rgb, keyColor);

    // if chroma, transparent
    if (diff < threshold) {
        finalColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        finalColor = texelColor * fragColor;
    }
}