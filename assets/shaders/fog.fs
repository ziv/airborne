#version 330


// from the vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;
in float fragDistance;

// base texture of the terrain
uniform sampler2D texture0;


// vars from our code
uniform vec3 skyColor;
uniform float fogNear;
uniform float fogFar;

// final color will be output to the framebuffer
out vec4 finalColor;

void main()
{
    // get the original color from the texture
    vec4 texelColor = texture(texture0, fragTexCoord);

    // mixing the texture color with the vertex color (which may include lighting effects)
    vec4 baseColor = texelColor * fragColor;

    // fog factor will be 0.0 when fragDistance <= fogNear, and 1.0 when fragDistance >= fogFar
    float fogFactor = smoothstep(fogNear, fogFar, fragDistance);

    // linear interpolation: finalColor = baseColor * (1.0 - fogFactor) + vec4(skyColor, 1.0) * fogFactor;
    finalColor = mix(baseColor, vec4(skyColor, 1.0), fogFactor);

    // keeping original alpha (usually 1.0 for terrain)
    finalColor.a = baseColor.a;
}