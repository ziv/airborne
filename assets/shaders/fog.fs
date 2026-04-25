#version 330

// from the vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;
in float fragDistance;
in vec3 fragWorldPos; // NEW: Received from VS

// base texture of the terrain
uniform sampler2D texture0;

// NEW: Vars replacing the single 'skyColor'
uniform vec3 cameraPos;    // Needed to calculate viewing angle
uniform vec3 zenithColor;  // Top of the sky (must match sky.fs)
uniform vec3 horizonColor; // Horizon color (must match sky.fs)

// vars from your code
uniform float fogNear;
uniform float fogFar;

// final color will be output to the framebuffer
out vec4 finalColor;

void main()
{
    // get the original color from the texture
    vec4 texelColor = texture(texture0, fragTexCoord);

    // mixing the texture color with the vertex color
    vec4 baseColor = texelColor * fragColor;

    // 1. Calculate direction vector from camera to this terrain pixel
    vec3 viewDir = normalize(fragWorldPos - cameraPos);

    // 2. Use the Y direction to find how high we are looking (0.0 to 1.0)
    // float upFactor = abs(viewDir.y);
    float upFactor = max(viewDir.y, 0.0);

    // 3. Apply the exact same curve used in your sky shader (e.g., 0.6)
    float skyBlend = clamp(pow(upFactor, 0.6), 0.0, 1.0);

    // 4. This is the exact color of the sky directly behind this pixel
    vec3 dynamicSkyColor = mix(horizonColor, zenithColor, skyBlend);

    // ------------------------------------

    // fog factor will be 0.0 when fragDistance <= fogNear, and 1.0 when >= fogFar
    float fogFactor = smoothstep(fogNear, fogFar, fragDistance);

    // linear interpolation using our dynamic sky color instead of a static one
    finalColor = mix(baseColor, vec4(dynamicSkyColor, 1.0), fogFactor);

    // keeping original alpha
    finalColor.a = baseColor.a;
}