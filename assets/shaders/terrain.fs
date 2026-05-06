#version 330

in vec2 fragTexCoord;
in float fragCamDist;

uniform sampler2D texture0;
uniform vec4 ambientLight;
uniform vec4 fogColor;

out vec4 finalColor;

const float fogStart = 40000.0;
const float fogEnd   = 70000.0;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);
    vec4 lit = texColor * ambientLight;
    float fogFactor = clamp((fragCamDist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    finalColor = mix(lit, fogColor, fogFactor);
}
