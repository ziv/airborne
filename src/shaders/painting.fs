#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

uniform vec2 renderSize;
uniform float radius = 2.0;
uniform float sigma = 2.0;

out vec4 finalColor;

void main() {
    vec3 n[9];

    vec2 src_size = vec2(1.0 / renderSize.x, 1.0 / renderSize.y);

    for (int i=-1; i<=1; i++) {
        for (int j=-1; j<=1; j++) {
            n[(i+1)*3+(j+1)] = texture(texture0, fragTexCoord + vec2(i,j)*src_size*radius).rgb;
        }
    }

    vec3 m0 = (n[0] + n[1] + n[2] + n[3] + n[4] + n[5] + n[6] + n[7] + n[8]) / 9.0;

    float colorsCount = 16.0;
    vec3 posterized = floor(m0 * colorsCount) / colorsCount;

    finalColor = vec4(posterized, 1.0) * fragColor;
}