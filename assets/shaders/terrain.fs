#version 330

in vec2 fragTexCoord;
in float fragCamDist;

uniform sampler2D texture0;
uniform vec4 ambientLight;

out vec4 finalColor;

void main()
{
    vec4 texColor = texture(texture0, fragTexCoord);

    // float fogStart = 30000.0;
    // float fogEnd   = 50000.0;
    // float fogFactor = clamp((fragCamDist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    // finalColor = texColor * vec4(texColor.rgb, texColor.a * (1.0 - fogFactor));
    finalColor = texColor * ambientLight; //  vec4(texColor.rgb, texColor.a * (1.0 - fogFactor));
}
