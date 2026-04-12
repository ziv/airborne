// night_vision.fs
#version 330

// Variables provided by Raylib
in vec2 fragTexCoord;    // Texture coordinates of the current pixel
in vec4 fragColor;       // General color set during drawing (usually white)
uniform sampler2D texture0; // The original map texture
uniform vec4 colDiffuse;   // Diffuse color (usually white)

// Variable we send from C++ to generate noise that changes over time
uniform float time;

// Helper function to generate pseudo-random noise
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    // 1. Sample the original color from the texture
    vec4 texelColor = texture2D(texture0, fragTexCoord);

    // 2. Convert to grayscale (standard luminance calculation)
    // We give high weight to green since that's what we want to emphasize
    float luminance = dot(texelColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    // 3. Create the green color base (black-green-white)
    vec3 greenTint = vec3(0.0, 1.0, 0.0); // Pure green
    // Can be adjusted, e.g. phosphor green: vec3(0.1, 0.9, 0.1)

    vec3 finalRGB = vec3(luminance) * greenTint;

    // 4. Add night vision effects (optional but adds a lot)

    // a. Grain/Noise effect that changes over time
    float noise = (rand(fragTexCoord + time) - 0.5) * 0.08; // Noise intensity is 8%
    finalRGB += noise;

    // b. Scanlines effect - dark horizontal lines
    float scanline = sin(fragTexCoord.y * 800.0) * 0.04; // Subtle lines
    finalRGB -= scanline;

    // 5. Return the final color with the original alpha
    gl_FragColor = vec4(finalRGB, texelColor.a);
}