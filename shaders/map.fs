#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float time;

out vec4 finalColor;

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);

    // the edges darker (slope)
    float dist = distance(fragTexCoord, vec2(0.5, 0.5));
    float vignette = smoothstep(0.7, 0.4, dist);
    texelColor.rgb *= (vignette * 0.2 + 0.8);

    // lens effects
    vec2 uv = fragTexCoord;
    uv -= 0.5;
    uv *= 1.0 + dot(uv, uv) * 0.1;
    uv += 0.5;
    vec4 texelColor = texture(texture0, uv);

    // shine
    // float glare = smoothstep(0.4, 0.5, sin(fragTexCoord.x + fragTexCoord.y + time * 0.5) * 0.5 + 0.5);
    // vec3 glareColor = vec3(1.0, 1.0, 1.0) * glare * 0.15;

    // a little bit army green
    vec3 glassTint = vec3(0.9, 1.0, 0.9);

    finalColor = vec4((texelColor.rgb * glassTint) + glareColor, texelColor.a);
}