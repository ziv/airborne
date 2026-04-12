#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;

void main() {
    vec4 texelColor = texture2D(texture0, fragTexCoord);
    float luminance = dot(texelColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    vec3 greenTint = vec3(0.0, 1.0, 0.0);
    gl_FragColor = vec4(vec3(luminance) * greenTint, texelColor.a);
}